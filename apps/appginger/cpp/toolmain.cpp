/******************************************************************************\
    Copyright (c) 2010 Stephen Leach. AppGinger is distributed under the terms 
    of the GNU General Public License. This file is part of AppGinger.

    AppGinger is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    AppGinger is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with AppGinger.  If not, see <http://www.gnu.org/licenses/>.
\******************************************************************************/


#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#include <cstdio>
#include <cstdlib>
#include <sstream>

#include <stddef.h>
#include <unistd.h>
#include <getopt.h>
#include <syslog.h>
#include <sys/errno.h>

#ifdef GNU_FD_TO_IFSTREAM
    #include <ext/stdio_filebuf.h> // __gnu_cxx::stdio_filebuf
#else
    #include "fdifstream.hpp"
#endif

#include "printgpl.hpp"
#include "fileutils.hpp"
#include "gngversion.hpp"
#include "mnx.hpp"
#include "command.hpp"

#include "appcontext.hpp"
#include "rcep.hpp"
#include "mishap.hpp"
#include "sys.hpp"
#include "misclayout.hpp"
#include "functionlayout.hpp"

#include "toolmain.hpp"
#include "wellknownpaths.hpp"
#include "userinput.hpp"

/*
    This is the structure of struct option, which does not seem to be
    especially well documented. Included for handy reference.
    struct option {
        const char *name;   // option name
        int has_arg;        // 0 = no arg, 1 = mandatory arg, 2 = optional arg
        int *flag;          // variable to return result or NULL
        int val;            // code to return (when flag is null)
                            //  typically short option code
*/

#define USAGE_FILE INSTALL_LIB "/ginger/usage/usage.txt"

extern char * optarg;
static struct option long_options[] =
    {
        { "engine",         required_argument,      0, 'E' },
        { "help",           optional_argument,      0, 'H' },
        { "machine",        required_argument,      0, 'm' },
        { "version",        no_argument,            0, 'V' },
        { "debug",          required_argument,      0, 'd' },
        { "grammar",        required_argument,      0, 'g' },
        { "license",        optional_argument,      0, 'L' },
        { "load",           required_argument,      0, 'l' },
        { "package",        required_argument,      0, 'p' },
        { "project",        required_argument,      0, 'j' },
        { "stdin",          no_argument,            0, 'i' },
        { "quiet",          no_argument,            0, 'q' },
        { "level",          required_argument,      0, 'v' },
        { 0, 0, 0, 0 }
    };

namespace Ginger  {
using namespace std;


void ToolMain::printGPL( const char * start, const char * end ) const {
    bool printing = false;
    ifstream license( LICENSE_FILE );
    if ( license.good() ) {
        std::string line;
        while ( getline( license, line ) )  {
            if ( !printing && ( start == NULL || line.find( start ) != string::npos ) ) {
                printing = true;
            } else if ( printing && end != NULL && line.find( end ) != string::npos ) {
                printing = false;
            }
            if ( printing ) {
                std::cout << line << std::endl;
            }
        }
    } else {
        std::cout << "License file not found." << endl;
    }
}

static void printFile( const char * filename ) {
    ifstream print_file( filename );
    if ( print_file.good() ) {
        std::string line;
        while ( getline( print_file, line ) )  {
            std::cout << line << std::endl;
        }
    } else {
        std::cerr << "File not found: " << filename << endl;
    }
}

static void printUsage() {
    printFile( USAGE_FILE );
}   

static void printHelpOptions() {
    cout << "--help=debug          help on the debugging options available" << endl;
    cout << "--help=help           this short help" << endl;
    cout << "--help=licence        help on displaying license information" << endl;
}

static void printHelpDebug() {
    cout << "--debug=showcode      Causes the generated instructions to be displayed." << endl;
    cout << "--debug=gctrace       Causes the garbage collector to emit debugging statistics." << endl;
}

static void printHelpLicense() {
    cout << "Displays key sections of the GNU Public License." << endl;
    cout << "--license=warranty    Shows warranty." << endl;
    cout << "--license=conditions  Shows terms and conditions." << endl;
}


void ToolMain::printLicense( const char * arg ) const {
    Ginger::optionPrintGPL( arg );
}

template < class ContainerT >
void tokenize(
    const std::string& str, 
    ContainerT & tokens,
    char delimiter, 
    const bool trimEmpty = false
) {
    std::string::size_type pos, lastPos = 0;
    for (;;) {
        pos = str.find_first_of( delimiter, lastPos );
        if ( pos == std::string::npos) {
            pos = str.length();
            if ( pos != lastPos || not trimEmpty ) {
                tokens.push_back(
                    typename ContainerT::value_type(
                        str.data() + lastPos,
                        pos - lastPos 
                    )
                );
            }

            break;
        }

        if ( pos != lastPos || not trimEmpty ) {
            tokens.push_back(
                typename ContainerT::value_type(
                    str.data() + lastPos,
                    pos - lastPos 
                )
            );
        }

        lastPos = pos + 1;
    }
};


void ToolMain::integrityChecks() {
    int integrity_failure = 0;
    if ( MAX_OFFSET_FROM_START_TO_KEY < OFFSET_FROM_FN_LENGTH_TO_KEY ) {
        integrity_failure = 1;
    }
    if ( integrity_failure ) {
        cerr << "ERROR: Integrity check failed: code " << integrity_failure << endl;
        exit( EXIT_FAILURE );
    }
}

// Return true for an early exit, false to continue processing.
bool ToolMain::parseArgs( int argc, char **argv, char **envp ) {
    this->integrityChecks();
    if ( envp != NULL ) this->context.setEnvironmentVariables( envp );
    return this->parseArgcArgv( argc, argv );
}

bool ToolMain::parseArgcArgv( int argc, char **argv ) {
    //bool meta_info_needed = false;
    for(;;) {
        int option_index = 0;
        int c = getopt_long( argc, argv, "d:e:g:H::ij:L::l:m:O:p:qv:V", long_options, &option_index );
        if ( c == -1 ) break;
        switch ( c ) {
            case 'd': {
                //std::string option( optarg );
                if ( std::string( optarg ) == std::string( "showcode" ) ) {
                    this->context.setShowCode();
                } else if ( std::string( optarg ) == std::string( "gctrace" ) ) {
                    this->context.setGCTrace( true );
                } else {
                    std::cerr << "Invalid debug option: " << optarg << std::endl;
                    return false;
                }
                break;
            }
            case 'e':
            case 'm' : {
                this->context.setMachineImplName( optarg );
                break;
            }
            case 'g': {
                this->context.setSyntax( optarg );
                break;
            }
            case 'H': {
                //  Eventually we will have a "home" for our auxillary
                //  files and this will simply go there. Or run a web
                //  browser pointed there.
                if ( optarg == NULL ) {
                    printUsage();
                } else if ( std::string( optarg ) == "help" ) {
                    printHelpOptions();
                } else if ( std::string( optarg ) == "debug" ) {
                    printHelpDebug();
                } else if ( std::string( optarg ) == "license" ) {
                    printHelpLicense();
                } else {
                    cout << "Unknown help topic %s" << optarg << endl;
                }
                return false;
            }
            case 'i': {
                this->context.useStdin() = true;
                break;
            }
            case 'j': {
                this->context.addProjectFolder( optarg );
                break;
            }
            case 'L': {
                printLicense( optarg );
                return false;
            }
            case 'l': {
                this->context.addLoadFile( optarg );
                break;
            }
            case 'O': {
                //  Multiple options as one option.
                
                //  If optarg is less than 1 in length, forget it.
                if ( optarg[ 0 ] == '\0' ) break;

                const string packed_options( &optarg[ 1 ] );
                vector< string > unpacked_options;
                tokenize< vector< string > >( 
                    packed_options,
                    unpacked_options,
                    optarg[ 0 ],
                    true
                );
                vector< char * > new_argv;
                new_argv.push_back( NULL );
                for ( 
                    vector< string >::iterator it = unpacked_options.begin(); 
                    it != unpacked_options.end(); 
                    ++it 
                ) {
                    new_argv.push_back( const_cast< char * >( it->c_str() ) );  
                }
                int new_argc = new_argv.size();
                new_argv.push_back( NULL );

                int current_optind = optind;
                optind = 1;
                
                const bool rtn = this->parseArgcArgv( new_argc, &new_argv[0] );

                optind = current_optind;

                if ( rtn ) {
                    break;
                } else {
                    return false;
                }
            }
            case 'p': {
                this->context.setInteractivePackage( optarg );
                break;
            }
            case 'q': {
                this->context.welcomeBanner() = false;
                break;
            }
            case 'v': {
                int level = atoi( optarg );
                this->context.printDetailLevel().level() = level;
                break;
            }
            case 'V': {
                cout << this->appName() << ": version " << this->context.version() << " (" << __DATE__ << " " << __TIME__ << ")" << endl;
                return false;
            }
            case '?': {
                //  Invalid option: exit.
                return false;
            }
            default: {
                //  This should not happen. It indicates that the option string 
                //  does not conform to the cases of this switch statement.
                throw SystemError( "Unrecognised option" ).culprit( "Option code", static_cast< long >( c ) );
            }
        }
    }

    //  Aggregate the remaining arguments, which are effectively filenames (paths).
    if ( optind < argc ) {
         while ( optind < argc ) {
            this->context.addArgument( argv[ optind++ ] );
         }
    }
    
    return true;
}

void ToolMain::runFrom( RCEP & rcep, Ginger::MnxReader & gnx_read ) {
    for (;;) {
        shared< Ginger::Mnx > m = gnx_read.readMnx();
        if ( not m ) break;
        rcep.execGnx( m, cout );
        rcep.printResults( cout, 0 );
    }           
}

class FileMgr {
private:
    bool tidy_exit;
    Ginger::Command & command;
    FILE * file;

public:
    FILE * get() const { return this->file; }
    int getFD() const { return fileno( this->file ); }
    void noteTidyExit() { this->tidy_exit = true; }

public:
    FileMgr( Ginger::Command & _command ) :
        tidy_exit( false ),
        command( _command ),
        file( NULL )
    {
        this->file = fdopen( this->command.runWithOutput(), "r" );
        if ( this->file == NULL ) {
            throw Ginger::Mishap( "Failed to translate input" );
        }
    }

public:
    ~FileMgr() { 
        if ( not this->tidy_exit ) {
            this->command.interrupt();
        }
        if ( this->file != NULL ) {
            fclose( this->file ); 
        }
    }   
};

void ToolMain::executeCommand( RCEP & rcep, Ginger::Command command ) {
    FileMgr gnxfp( command );
    
    #ifdef GNU_FD_TO_IFSTREAM
        // ... open the file, with whatever, pipes or who-knows ...
        // let's build a buffer from the FILE* descriptor ...
        __gnu_cxx::stdio_filebuf<char> pipe_buf( gnxfp.get(), ios_base::in );
        // there we are, a regular istream is build upon the buffer.
        istream stream_pipe_in( &pipe_buf );
    #else
        FileDescriptorIFStream stream_pipe_in( gnxfp.getFD() );
    #endif
   
    while ( rcep.unsafe_read_comp_exec_print( stream_pipe_in, std::cout ) ) {}
    gnxfp.noteTidyExit();    
}

void ToolMain::executeFile( RCEP & rcep, const string filename ) {
    this->executeCommand( rcep, this->context.fileSyntaxCommand( filename ) );
}

/**
 * This is the command for acquiring and parsing user input to XML.
 * TODO: need to fix the way we invoke GNU readline. We may well prefer
 *       to invoke gngreplay.
 */
void ToolMain::executeStdin( const bool use_gnu_readline, RCEP & rcep ) {
    if ( use_gnu_readline ) {
        UserInput user_input( this->context.stdinSyntaxCommand() );
        while ( rcep.mainloop( user_input, std::cout ) ) {}
    } else {
        this->executeCommand( rcep, this->context.stdinSyntaxCommand() );
    }
}

void ToolMain::executeFileArguments( RCEP & rcep ) {
    vector< string > & args = this->context.arguments();
    for ( vector< string >::iterator it = args.begin(); it != args.end(); ++it ) {
        this->executeFile( rcep, *it );
    }
}

void ToolMain::executeLoadFileList( RCEP & rcep ) {
    list< string > & load_files = this->context.getLoadFileList();
    for ( list< string >::iterator it = load_files.begin(); it != load_files.end(); ++it ) {
        this->loadFileFromPackage( rcep, rcep.currentPackage(), *it );
    }
}


void ToolMain::loadFileFromPackage( RCEP & rcep, Package * pkg, const std::string filename ) {
    Ginger::Command cmd( ( USESNAP ? getenv( "SNAP" ) : nullptr ), FETCHGNX );
    cmd.addArg( "-X" );
    {
        list< string > & folders = rcep.getMachine()->getAppContext().getProjectFolderList();
        for ( 
            list< string >::iterator it = folders.begin();
            it != folders.end();
            ++it
        ) {
            cmd.addArg( "-j" );
            cmd.addArg( *it );
        }
    }

    Ginger::MnxBuilder qb;
    qb.start( "fetch.load.file" );
    qb.put( "pkg.name", pkg->getTitle() );
    qb.put( "load.file", filename );
    qb.end();
    shared< Ginger::Mnx > query( qb.build() );

    #ifdef DBG_SCRIPT_MAIN
        cerr << "scriptmain asking for loadfile, using fetchgnx -X" << endl;
        cerr << "  [[";
        query->render( cerr );
        cerr << "]]" << endl;
    #endif  

    cmd.runWithInputAndOutput();
    int fd = cmd.getInputFD();   
    FILE * foutd = fdopen( cmd.getOutputFD(), "w" );
    query->frender( foutd );
    fflush( foutd );

    stringstream prog;
    for (;;) {
        static char buffer[ 1024 ];
        //  TODO: protect close with finally.
        int n = read( fd, buffer, sizeof( buffer ) );
        if ( n == 0 ) break;
        if ( n == -1 ) {
            if ( errno != EINTR ) {
                perror( "Script main loading file" );
                throw Ginger::Mishap( "Failed to read" );
            }
        } else if ( n > 0 ) {
            prog.write( buffer, n );
        }
    }

    //  TODO: protect close with finally.
    fclose( foutd );

    #ifdef DBG_SCRIPT_MAIN
        cerr << "  [[" << prog.str() << "]]" << endl;
    #endif
    while ( rcep.unsafe_read_comp_exec_print( prog, std::cout ) ) {}
}

} // namespace Ginger
