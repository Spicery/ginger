import subprocess
import re
import os.path
import argparse
import sys

CXX = "g++"
CXXFLAGS = [
    "-I../hpp",
    "-I../../libgng/hpp",
    "-Wall",
    "-g",
    "-std=c++11"
]

class BuildManager:

    def __init__( self ):
        # The table of build-rules.
        self.targetUpdateAction = {}
        # The package name is, of course, Ginger.
        self.PACKAGE_TARNAME = "ginger"
        # Main installation directory. Binary executables may go somewhere else.
        self.prefix = "/usr/local"
        # Where the binary executables will go. 
        self.exec_prefix = "/usr/local"
        # Where to find the data files supporting the runtime.
        self.datarootdir = "{}/share".format( self.prefix )
        # -D flags
        self.INSTALL = {}
        # Installation directory of the main executables.
        self.INSTALL['INSTALL_BIN'] = "{}/bin".format( self.exec_prefix )
        # Installation directory for the supporting executables.
        self.INSTALL['INSTALL_TOOL'] = "{}/libexec/ginger".format( self.exec_prefix )
        # Installation directory for the supporting data files.
        self.INSTALL['INSTALL_LIB'] = "{}/{}".format( self.datarootdir, self.PACKAGE_TARNAME )
        # Installation directory for the documentation.
        self.INSTALL['INSTALL_DOC'] = "{}/doc/{}".format( self.datarootdir, self.PACKAGE_TARNAME )


    def fetchOptions( self, default_target ):
        p = argparse.ArgumentParser( prog='python3 mkfile.py', description="Experimental build script" )
        p.add_argument( '-t', '--target', default=default_target, help='Try to build the target' )
        p.add_argument( '-v', '--verbose', action='store_true', help='Switches on verbose mode' )
        p.add_argument( '-x', '--extlibs', action='store_true', help='Builds libraries external to this project' )
        self.options = p.parse_args()

    def updateTarget( self, target ):
        if self.options.verbose:
            print( 'target', target )
        if target in self.targetUpdateAction:
            self.targetUpdateAction[ target ]( self, target )

    def updateDependencies( self, src_list ):
        for src in src_list:
            self.updateTarget( src )

    def needsUpdate( self, dst, src_list, update_dependencies=True ):
        if update_dependencies:
            self.updateDependencies( src_list )
        return self.isOutOfDate( dst, src_list )

    @staticmethod
    def parseDependenciesFile( dfile ):
        with open( dfile, 'r' ) as d:
            return re.sub( '\\\\\n', '', re.sub( '^[^:]*:\\s', '', d.read() )).split()

    def _installDirs( self ):
        return [ '-D{}="{}"'.format( k, v ) for ( k, v ) in self.INSTALL.items() ]
        
    @staticmethod
    def run( arg_list, verbose=False, cwd=None ):
        if verbose:
            print( arg_list )
        result = subprocess.run( arg_list, cwd=cwd )
        rc = result.returncode
        if rc != 0:
            print( '### ERROR', file=sys.stderr )
            sys.exit( 1 )

    @staticmethod
    def isOutOfDate( dst, src_list ):
        try:
            tdst = os.path.getmtime( dst )
            for src in src_list:
                tsrc = os.path.getmtime( src )
                # print( 'Compare src {} {} > dst {} {}'.format( src, tsrc, dst, tdst ) )
                if tsrc > tdst:
                    return True
            return False
        except FileNotFoundError:
            return True

    def compileSrcFile( self, src_file, dst_file, just_dependencies=False ):
        if self.options.verbose:
            if just_dependencies:
                print( 'Calculating dependencies of {}'.format( dst_file ) )
            else:
                print( 'Compiling {} from {}'.format( dst_file, src_file ) )
        self.run(
            [ CXX ] +
            ( [ "-MM", "-MF" ] if just_dependencies else [ "-MMD", "-c", "-o"  ] ) +
            [ dst_file ] +
            self._installDirs() + 
            CXXFLAGS + 
            [ src_file ],
            verbose=self.options.verbose
        )

    def compile( self, obj_file ):
        src_file = re.sub( '[.]o$', '.cpp', obj_file )
        dep_file = re.sub( '[.]o$', '.d', obj_file )
        if not os.path.exists( dep_file ) or self.needsUpdate( dep_file, [ src_file ] ):
            self.compileSrcFile( src_file, dep_file, just_dependencies=True )
        if self.isOutOfDate( obj_file, self.parseDependenciesFile( dep_file ) ):
            self.compileSrcFile( src_file, obj_file )
        elif self.options.verbose:
            print( '{} is up to date'.format( obj_file ) )
    
    def link( self, executable, objs, external_libs ):
        if (
            self.needsUpdate( executable, external_libs ) or
            self.needsUpdate( executable, objs )
        ):
            if self.options.verbose:
                print( 'Linking executable {}'.format( executable ) )
            try:
                self.run(
                    [ CXX, "-o", executable ] +
                    CXXFLAGS +
                    self._installDirs() + 
                    objs + 
                    external_libs,
                    verbose=self.options.verbose
                )
            except SystemExit:
                print( '### Failed while trying to link {}'.format( executable ), file=sys.stderr )
                raise                
        elif self.options.verbose:
            print( '{} is up to date'.format( executable ) )

    def externalLibrary( self, ext_lib ):
        if self.options.extlibs:
            target = os.path.basename( ext_lib )
            cwd = os.path.dirname( ext_lib )
            try:
                self.run( [ 'python3', 'mkfile.py', '--extlibs', '--target', target ], cwd=cwd )
            except SystemExit:
                print( '### Failed while building external library {}'.format( ext_lib ), file=sys.stderr )
                raise
        elif self.options.verbose:
            print( 'Skipping external library {}'.format( ext_lib ) )


    def addTargetRule( self, fname, action ):
        self.targetUpdateAction[ fname ] = action

class Rule:

    def __init__( self, target ):
        self._target = target

    def target( self ):
        return self._target

    def __rshift__( self, build_mgr ):
        '''Abuse of the >> notation to style adding a rule'''
        build_mgr.addTargetRule( self.target(), self )

class CompileRule( Rule ):

    def __init__( self, target ):
        super().__init__( target )

    def __call__( self, build_mgr, obj_file ):
        build_mgr.compile( obj_file )

class LinkRule( Rule ):
    
    def __init__( self, target, obj_file_list, ext_libs_list ):
        super().__init__( target )
        self.obj_file_list = obj_file_list
        self.ext_libs_list = ext_libs_list

    def __call__( self, build_mgr, exe_file ):
        build_mgr.link( exe_file, self.obj_file_list, self.ext_libs_list )

class ProjectRule( Rule ):

    def __init__( self, target ):
        super().__init__( target )
    
    def __call__( self, build_mgr, ext_lib_file ):
        build_mgr.externalLibrary( ext_lib_file )


    
# Others
EXTERNAL_LIBS = [ "../../libgng/cpp/libgng.a" ]
EXECUTABLE = "tidymnx"
OBJS = [ "tidymnx.o" ]

# Rules
BUILDER = BuildManager()

for i in OBJS:
    CompileRule( i ) >> BUILDER
LinkRule( EXECUTABLE, OBJS, EXTERNAL_LIBS ) >> BUILDER
for i in EXTERNAL_LIBS:
    ProjectRule( i ) >> BUILDER

if __name__ == "__main__":
    BUILDER.fetchOptions( EXECUTABLE )
    BUILDER.updateTarget( BUILDER.options.target )

