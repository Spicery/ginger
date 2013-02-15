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

#include <cstdlib>
#include <sstream>

#include <ext/stdio_filebuf.h> // __gnu_cxx::stdio_filebuf

#include <stddef.h>
#include <unistd.h>
#include <getopt.h>
#include <syslog.h>
#include <sys/errno.h>

#include "fileutils.hpp"
#include "gngversion.hpp"
#include "mnx.hpp"
#include "command.hpp"

#include "appcontext.hpp"
#include "rcep.hpp"
#include "mishap.hpp"
#include "sys.hpp"
#include "machine1.hpp"
#include "machine2.hpp"
#include "machine3.hpp"
#include "misclayout.hpp"
#include "functionlayout.hpp"

#include "toolmain.hpp"
#include "wellknownpaths.hpp"

using namespace std;



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

extern char * optarg;
static struct option long_options[] =
    {
        { "engine",         required_argument,      0, 'E' },
        { "help",           optional_argument,      0, 'H' },
        { "machine",        required_argument,      0, 'm' },
        { "version",        no_argument,            0, 'V' },
        { "debug",          required_argument,      0, 'd' },
        { "grammar",		required_argument,		0, 'g' },
        { "license",        optional_argument,      0, 'L' },
        { "load",           required_argument,      0, 'l' },
        { "package",        required_argument,      0, 'p' },
        { "project",		required_argument,		0, 'j' },
        { "stdin",			no_argument,			0, 'i' },
        { "quiet",          no_argument,            0, 'q' },
        { "level",		    required_argument,		0, 'v' },
        { 0, 0, 0, 0 }
    };

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

static void printUsage() {
	cout << "Usage :  " << PACKAGE_NAME << " [options] [files]" << endl << endl;
	cout << "OPTION                SUMMARY" << endl;
	cout << "-d, --debug=OPTION    add debug option (see --help=debug)" << endl;
	cout << "-e, --engine=<N>      run using engine #n" << endl;
	cout << "-g, --grammar=LANG    select front-end syntax" << endl;
	cout << "-H, --help            print out this help info (see --help=help)" << endl;
	cout << "-i, --stdin           compile from stdin" << endl;
	cout << "-j, --project=PATH    add a project folder to the search path" << endl;
	cout << "-L, --license         print out license information and exit" << endl;
    cout << "-l, --load=FILE       load a file from the load-folder of the current package" << endl;
    cout << "-O, --options         compact multiple options as one option (for #! scripts)" << endl;
    cout << "-p, --package=PKG     initial interactive package" << endl;
	cout << "-q, --quiet           no welcome banner" << endl;
	cout << "-v, --level=LEVEL     set results level to 1 or 2" << endl;
	cout << "-V, --version         print out version information and exit" << endl;
	cout << endl;
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
	if ( arg == NULL || std::string( arg ) == std::string( "all" ) ) {
		this->printGPL( NULL, NULL );
	} else if ( std::string( arg ) == std::string( "warranty" ) ) {
		this->printGPL( "Disclaimer of Warranty.", "Limitation of Liability." );                 
	} else if ( std::string( arg ) == std::string( "conditions" ) ) {
		this->printGPL( "TERMS AND CONDITIONS", "END OF TERMS AND CONDITIONS" );
	} else {
		std::cerr << "Unknown license option: " << arg << std::endl;
	}
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

    //  TO-DO delete this section - a bad idea.
    //  If the name of the binary has a '-' sign in it, it is used as
    //  as the default grammar.
    {
        //cerr << "Infer grammar ..." << endl;
        const string appname( argc >= 1 ? argv[ 0 ] : "" );
        const size_t n = appname.rfind( '-' );
        if ( n != string::npos ) {
            this->context.setSyntax( appname.substr( n + 1, appname.size() ) );
            //cerr << "Extn = " << appname.substr( n + 1, appname.size() ) << endl;
        //} else {
            //cerr << "None set" << endl;
        }
	}

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
                this->context.setMachineImplNum( atoi( optarg ) );
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
            	this->context.printDetailLevel() = level;
            	break;
            }
            case 'V': {
                cout << this->appName() << ": version " << this->context.version() << " (" << __DATE__ << " " << __TIME__ << ")" << endl;
                return false;
            }
            case '?': {
            	//	Invalid option: exit.
                return false;
            }
            default: {
            	//	This should not happen. It indicates that the option string 
            	//	does not conform to the cases of this switch statement.
            	throw Ginger::SystemError( "Unrecognised option" ).culprit( "Option code", static_cast< long >( c ) );
            }
        }
    }

	//	Aggregate the remaining arguments, which are effectively filenames (paths).
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

void ToolMain::executeCommand( RCEP & rcep, const string command ) {
    //cerr << "Command so far: " << command << endl;
    FILE * gnxfp = popen( command.c_str(), "r" );
    if ( gnxfp == NULL ) {
        throw Ginger::Mishap( "Failed to translate input" );
    }
    // ... open the file, with whatever, pipes or who-knows ...
    // let's build a buffer from the FILE* descriptor ...
    __gnu_cxx::stdio_filebuf<char> pipe_buf( gnxfp, ios_base::in );
    
    // there we are, a regular istream is build upon the buffer.
    istream stream_pipe_in( &pipe_buf );
    
    while ( rcep.unsafe_read_comp_exec_print( stream_pipe_in, std::cout ) ) {}

    pclose( gnxfp );    
}

void ToolMain::executeFile( RCEP & rcep, const string filename ) {
    stringstream commstream;
    //  tail is 1-indexed!
    commstream << this->context.syntax( filename );
    this->executeCommand( rcep, commstream.str() );
}

void ToolMain::executeStdin( const bool interactively, RCEP & rcep ) {
    this->executeCommand( rcep, this->context.syntax( interactively ) );
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
    Ginger::Command cmd( FETCHGNX );
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
