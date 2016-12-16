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

//  Usage: gprojlibtool [OPTIONS] < QUERY > RESPONSE
//      -H, --help              print out help summary and exit
//      -V, --version           print out version information and exit
//      -L, --license[=PART]    print out license information and exit (see --help=license)
//

//  Local debugging switch for conditional code compilation.
//#define DBG_GPROJLIBTOOL 1
//#define DBG_SEARCH
 
#define GPROJLIBTOOL "gprojlibtool"
#define PATH_SEPARATOR "/"
#define FILE2GNX "file2gnx"

#include <iostream>
#include <string>
#include <utility>
#include <fstream>
#include <vector>
#include <cstdlib>
#include <regex>
#include <sstream>

#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <stddef.h>
#include <getopt.h>
#include <syslog.h>
#include <stddef.h>

#include "fileutils.hpp"
#include "printgpl.hpp"
#include "mnx.hpp"
#include "mishap.hpp"

#include "folderscan.hpp"

using namespace std;

#define AUTO_SUFFIX             ".auto"
#define AUTO_SUFFIX_SIZE        sizeof( AUTO_SUFFIX )

#define LOAD                    "load"
#define LOAD_SIZE               sizeof( LOAD )

static std::string getFile2GnxPath() {
    std::string cmd;
    if ( USESNAP ) {
        const char * snap = getenv( "SNAP" );
        if ( snap ) {
            cmd += snap;
        }
    }
    cmd += INSTALL_TOOL PATH_SEPARATOR FILE2GNX;
    return cmd;
}

class Task {
protected:
    vector< string > arguments;
public:
    virtual void run() = 0;
private:
    //  Remove folder path & extensions.
    static string nameOnly( string pathname ) {

        regex namepart( "^(?:[^/]*/)*([^/.]+)(?:\\.[^./]*)*/?$" );
        cmatch name;
        if ( regex_match( pathname.c_str(), name, namepart ) ) {
            return name[1];
        } else {
            return "";
        }
    }
public:
    virtual void addArgument( const string arg ) {
        this->arguments.push_back( arg );
    }
    virtual const string projectName() const {
        return nameOnly( this->projectFolder() );
    }
    virtual const string projectFolder() const {
        if ( arguments.empty() ) {
            throw Ginger::Mishap( "No project specified" );
        }
        return arguments[0];
    }
    virtual ~Task() {}
};

inline bool doesFileExist( const std::string& name ) {
    struct stat buffer;   
    return stat( name.c_str(), &buffer ) == 0; 
}

enum STATE { NORMAL, PERCENT1, PERCENT2 };

static int unhex( char ch ) {
    if ( '0' <= ch && ch <= '9' ) {
        return ch - '0';
    } else if ( 'A' <= ch && ch <= 'F' ) {
        return 10 + ch - 'A';
    } else if ( 'a' <= ch && ch <= 'f' ) {
        return 10 + ch - 'a';
    } else {
        stringstream character;
        character << ch;
        throw Ginger::Mishap( "Invalid hex character" ).culprit( "Character", character.str() );
    }
}


/*
    Reverse the HTML escape process that is used to escape autoloadable
    file names.
    
    WARNING!!!!!! We have to decode the root, otherwise the
    encoded names (such as for "+" and "++" will not work)                  
*/
static string URLdecode( string name ) {
    //cout << "URLdecoding: " << name << endl;
    stringstream sofar;
    enum STATE state = NORMAL;
    int n;
    for ( 
        string::iterator it = name.begin();
        it != name.end();
        ++it
    ) {
        char ch = *it;
        //cerr << "State = " << state << endl;
        switch ( state ) {
            case NORMAL: {
                if ( ch == '%' ) {
                    //cerr << "Switching state" << endl;
                    state = PERCENT1;
                } else {
                    sofar << ch;
                }
                break;
            }
            case PERCENT1: {
                n = unhex( ch );
                state = PERCENT2;
                break;
            }
            case PERCENT2: {
                sofar << (char)( ( n << 4 ) | unhex( ch ) );
                state = NORMAL;
                break;
            }
        }
    }
    //cout << "Decoding " << name << " into " << sofar.str() << endl;
    return sofar.str();
}


/*
   * <fetch.index/>
   * <fetch.var.gnx def.pkg=${DEFPKG} var.name=${VAR} />
   * <fetch.init.gnx def.pkg=${DEFPKG} />
*/
class ExecuteTask : public Task {
private:
    Ginger::SharedMnx query;

private:
    /*
    INDEX ::= <project name=${PROJECT_NAME}> PACKAGE* </project>
    PACKAGE ::= <package name=${PACKAGE_NAME}> IMPORTS INCLUDES DEFINITIONS </package>
    IMPORTS ::= <imports> IMPORT* </imports>
    IMPORT ::= <import from.pkg=${PKG_NAME} [ alias=${ALIAS} ] from.tags=${TAGS} to.tags=${TAGS1} />
    INCLUDE ::= <include from.pkg=${PKG_NAME} [ alias=${ALIAS} ] from.tags=${TAGS} to.tags=${TAGS1}/>
    DEFINITIONS ::= <definitions> DEFINITON* </definitions>
    DEFINITION ::= <define var.name=${VAR} tags=${TAGS} />

    For example::

        <project name="Foo">
            <package name="maths=">
                <imports>
                    <import from.pkg="ginger.constants" from.tags="public" to.tags="public" />
                </imports>
                <definitions>
                    <define var.name="pi" tags="public" />
                    <define var.name="e" tags="public" />
                </definitions>
            </package>
        </project>
    */
    void addAllPackages( Ginger::MnxBuilder & b ) {
        FolderScan fscan( this->projectFolder() );
        while ( fscan.nextFolder() ) {
            const string entry = fscan.entryName();
            if ( entry.size() >= 1 && entry[0] == '.' ) continue;

            const string pkg_folder = fscan.folderName() + '/' + entry;

            b.start( "package" );
            b.put( "name", entry );

            b.start( "imports" );
            ifstream imp( pkg_folder + "/imports.gnx" );
            if ( imp ) {
                Ginger::MnxReader reader( imp );
                Ginger::SharedMnx imports = reader.readMnx();
                for ( Ginger::MnxChildIterator kids( imports ); !!kids; ++kids ) {
                    auto k = *kids;
                    if ( ! k->hasName( "import" ) ) continue;
                    if ( ! k->hasAttribute( "from" ) ) continue;
                    if ( ! k->hasAttribute( "match0" ) ) continue;
                    b.start( "import" );
                    b.put( "from.pkg", k->attribute( "from" ) );
                    b.put( "from.tags", k->attribute( "match0" ) );
                    b.end();
                }
            }
            b.end();
            
            this->addPackage( b, pkg_folder );

            b.end();
        }
    }

    void addPackage( Ginger::MnxBuilder & b, const string pkg_folder ) {
        FolderScan fscan( pkg_folder );

        b.start( "definitions" );
        
        while ( fscan.nextFolder() ) {
            const string entry = fscan.entryName();

            #ifdef DBG_SEARCH
                cerr << "subfolder: " << entry << endl;
            #endif

            //  Check that -entry- matches *.auto
            if ( entry.find( AUTO_SUFFIX, entry.size() - AUTO_SUFFIX_SIZE ) != string::npos ) {
            
                const string default_tag = entry.substr( 0, entry.size() + 1 - AUTO_SUFFIX_SIZE );
                //cout << "TAG = " << default_tag << endl;
            
                #ifdef DBG_SEARCH
                    cerr << "*.auto: " << entry << endl;
                #endif
                
                FolderScan files( fscan.folderName() + "/" + entry );
                while ( files.nextFile() ) {
                    string fname = files.entryName();
                    #ifdef DBG_SEARCH
                        cerr << "Entry : " << fname << endl;
                    #endif
                    
                    size_t n = fname.rfind( '.' );
                    if ( n == string::npos ) continue;
                    
                    const string root = URLdecode( fname.substr( 0, n ) );
                    const string extn = fname.substr( n + 1 );
                    
                    #ifdef DBG_SEARCH
                        cerr << "Adding " << root << " -> " << ( files.folderName() + "/" + fname ) <<  endl;
                    #endif
                    
                    b.start( "define" );
                    b.put( "name", root );
                    b.put( "tags",default_tag );
                    b.end();
                }
            } else if ( entry == "load" ) {
                //  It doesn't match *.auto but it is a load folder though.
                //const string p( fscan.folderName() + "/" + entry );
                //newc->setLoadFolder( p );
            }
        }   

        b.end();
    }

    void fetchIndex() {
        #ifdef DBG_GPROJLIBTOOL
            cerr << "COMMAND: " << "fetchIndex" << endl;
            cerr << "FOLDER:  " << this->projectFolder() << endl;
            cerr << "Name:    " << this->projectName() << endl;
        #endif
        Ginger::MnxBuilder b;
        b.start( "project" );
        b.put( "name", this->projectName() );
        this->addAllPackages( b );
        b.end();
        b.build()->render();
        cout << endl;
    }

private:
    /*
       * <fetch.index/>
       * <fetch.var.gnx def.pkg=${DEFPKG} var.name=${VAR} />
       * <fetch.init.gnx def.pkg=${DEFPKG} />
    */
    void fetchVarGnx() {
        if ( ! this->query->hasAttribute(  "def.pkg" ) ) {
            throw Ginger::Mishap( "Malformed query - missing def.pkg" );
        }
        if ( ! this->query->hasAttribute( "var.name" ) ) {
            throw Ginger::Mishap( "Malformed query - missing var.name" );
        }
        string def_pkg = this->query->attribute( "def.pkg" );
        string var_name = this->query->attribute( "var.name" );

        //  Now we need to look in ${PROJECT_FOLDER}/${PACKAGE_NAME}/*.auto/${VAR.NAME}.*
        string pkg_folder = this->projectFolder() + PATH_SEPARATOR + def_pkg;
        FolderScan pkg_scan( pkg_folder );
        while ( pkg_scan.nextFolder() ) {
            const string entry = pkg_scan.entryName();
            if ( entry.find( AUTO_SUFFIX, entry.size() - AUTO_SUFFIX_SIZE ) != string::npos ) {
                FolderScan autoloadable_scan( pkg_folder + PATH_SEPARATOR + entry );
                while ( autoloadable_scan.nextFile() ) {
                    const string autoloadable_file = autoloadable_scan.entryName();
                    if ( autoloadable_file.size() <= var_name.size() ) continue;
                    if ( autoloadable_file[ var_name.size() ] != '.' ) continue;
                    if ( std::equal( var_name.begin(), var_name.end(), autoloadable_file.begin() ) ) {
                        const string pathname = autoloadable_scan.fullPath();
                        std::string cmd = getFile2GnxPath();
                        execl( cmd.c_str(), cmd.c_str(), pathname.c_str(), NULL );
                    }
                }
            }
        }
    }

private:

    /*
       * <fetch.init.gnx def.pkg=${DEFPKG} />
    */
    void fetchInitGnx() {
        if ( ! this->query->hasAttribute( "def.pkg" ) ) {
            throw Ginger::Mishap( "Malformed query - missing def.pkg" );
        }
        string def_pkg = this->query->attribute( "def.pkg" );
        
        //  Now we need to look in ${PROJECT_FOLDER}/${PACKAGE_NAME}/imports.gnx
        const string load_folder = this->projectFolder() + PATH_SEPARATOR + def_pkg + PATH_SEPARATOR + "load";
        cout << "<seq>" << endl;
        if ( doesFileExist( load_folder ) ) {
            //  Start up a copy of file2gnx for every file in the folder.
            FolderScan lscan( load_folder );
            while ( lscan.nextFile() ) {
                const string file = lscan.fullPath();
                const int pid = fork();
                if ( pid == 0 ) {
                    //  In the child.
                    std::string cmd = getFile2GnxPath();
                    execl( cmd.c_str(), cmd.c_str(), file.c_str(), NULL );
                } else if ( pid > 0 ) {
                    int return_status;
                    waitpid( pid, &return_status, 0 );
                } else {
                    throw Ginger::Mishap( "Subprocess failed" ).culprit( "Subprocess", FILE2GNX );
                }
            }
        }
        cout << "</seq>" << endl;
    }

public:
    virtual void run() {
        Ginger::MnxReader reader;  
        this->query = reader.readMnx();
        if ( ! this->query ) {
            throw Ginger::Mishap( "No MinXML query on input" );
        }
        if ( query->hasName( "fetch.index" ) ) {
            this->fetchIndex();
        } else if ( query->hasName( "fetch.var.gnx" ) ) {
            this->fetchVarGnx();
        } else if ( query->hasName( "fetch.init.gnx" ) ) {
            this->fetchInitGnx();
        } else {
            throw Ginger::Mishap( "Unrecognised query" ).culprit( "Query", this->query->toString() );
        }
    }
};

class Main {
private:
    unique_ptr< Task >          task;
    string                      project_folder;
    
public:
    void parseArgs( int argc, char **argv, char **envp );
    void summary();
    void run();
    std::string version();
    
public:
    Main() : 
        task( nullptr )
    {
    }
};

std::string Main::version() {
    //  ToDo: not very happy with the way I have got versioning structured.
    return "0.1";
}

extern char * optarg;
static struct option long_options[] =
    {
        { "help",           optional_argument,      0, 'H' },
        { "license",        optional_argument,      0, 'L' },
        { "query",          no_argument,            0, 'Q' },
        { "version",        no_argument,            0, 'V' },
        { 0, 0, 0, 0 }
    };

static void printUsage() {
    cout << "Usage: " << GPROJLIBTOOL << " [MODE OPTION] <PATHNAME>" << endl;
    cout << "MODE OPTIONS" << endl;
    cout << "-Q, --query           execute a MinXML query read from stdin (default mode)" << endl;
    cout << "-H, --help[=TOPIC]    help info on optional topic (see --help=help)" << endl;
    cout << "-L, --license[=PART]  print out license information (see --help=license)" << endl;
    cout << "-V, --version         print out version information" << endl;
    cout << endl;
}

static void printHelpOptionUsage() {
    cout << "--help=help           this short help" << endl;
    cout << "--help=licence        help on displaying license information" << endl;
    cout << "--help=std            print out variables in std" << endl;
}

static void printLicenseOptionUsage() {
    cout << "Displays key sections of the GNU Public License." << endl;
    cout << "--license=warranty    Shows warranty." << endl;
    cout << "--license=conditions  Shows terms and conditions." << endl;
}

void Main::parseArgs( int argc, char **argv, char **envp ) {
    this->task.reset( new ExecuteTask() );
    for(;;) {
        int option_index = 0;
        int c = getopt_long( argc, argv, "H::L::QV", long_options, &option_index );
        if ( c == -1 ) break;
        switch ( c ) {
            case 'H': {
                //  Eventually we will have a "home" for our auxillary
                //  files and this will simply go there. Or run a web
                //  browser pointed there.
                if ( optarg == NULL ) {
                    printUsage();
                } else if ( std::string( optarg ) == "help" ) {
                    printHelpOptionUsage();
                } else if ( std::string( optarg ) == "license" ) {
                    printLicenseOptionUsage();
                } else {
                    cout << "Unknown help topic " << optarg << endl;
                }
                exit( EXIT_SUCCESS );   //  Is that right?
            }
            case 'L': {
                Ginger::optionPrintGPL( optarg );
                exit( EXIT_SUCCESS );   //  Is that right?              
            }
            case 'Q': {
                //  Default case. No action needed.
                break;
            }
            case 'V': {
                cout << GPROJLIBTOOL << ": version " << this->version() << " (" << __DATE__ << " " << __TIME__ << ")" << endl;
                exit( EXIT_SUCCESS );   //  Is that right?
            }
            case '?': {
                exit( EXIT_FAILURE );
            }
            default: {
                cout << "?? getopt returned character code " << hex << static_cast< int >( c ) << dec << endl;
            }
        }
    }
    //  PROCESS OTHER ARGS GOES HERE
    //      Aggregate the remaining arguments, which are effectively filenames (paths).
    if ( optind < argc ) {
        while ( optind < argc ) {
            this->task->addArgument( argv[ optind++ ] );
        }
    }
}

void Main::run() {
    this->task->run();
}

int main( int argc, char ** argv, char **envp ) {
    openlog( GPROJLIBTOOL, LOG_INFO, LOG_LOCAL2 );
    setlogmask( LOG_UPTO( LOG_INFO ) );
    try {
        Main main;
        main.parseArgs( argc, argv, envp );
        main.run();
        return EXIT_SUCCESS;
    } catch ( Ginger::Mishap & p ) {
        p.culprit( "Detected by", GPROJLIBTOOL );
        p.report();
        return EXIT_FAILURE;
    }
}
