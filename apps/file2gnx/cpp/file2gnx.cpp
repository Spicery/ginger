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
#include <cstdlib>
#include <fstream>
#include <list>
#include <vector>

#include <syslog.h>
#include <unistd.h>
#include <stddef.h>
#include <getopt.h>

#include "config.h"

#include "xdgconfigfiles.hpp"
#include "mnx.hpp"
#include "sax.hpp"
#include "mishap.hpp"

#define LICENSE_FILE	INSTALL_LIB "/COPYING"


using namespace std;
using namespace Ginger;

#define APP_TITLE		"file2gnx"
#define LOG_FACILITY	LOG_LOCAL2

#define COMMON2GNX		( INSTALL_TOOL "/common2gnx" )
#define CSTYLE2GNX		( INSTALL_TOOL "/cstyle2gnx" )
#define LISP2GNX 		( INSTALL_TOOL "/lisp2gnx" )
#define GNX2GNX     	( "/bin/cat" )

#define FILE2GNX_CONFIG_BASE	"parser-mapping.mnx"
#define FILE2GNX_CONFIG 		( "/etc/xdg/ginger/" FILE2GNX_CONFIG_BASE )

//
//	Insecure. We need to do this more neatly. It would be best if common2gnx
//	and lisp2gnx could handle being passed a filename as an argument. This
//	would be both more secure and efficient.
//
#define PARSER "parser"
#define PARSER_EXT "ext"
#define PARSER_EXE "exe"


class ExtnFinder {
private:
	list< string > extns;

public:
	void addGrammar( const string & grammar ) {
		this->extns.push_back( grammar );
	}

	void tryAddExtnOfPathName( const string & pathname ) {
		const size_t n = pathname.rfind( '.' );
		if ( n != string::npos ) {
			//cerr << "Pushing back: " << n << endl;
			this->extns.push_back( pathname.substr( n + 1, pathname.size() ) );
		}
	}

	bool hasNext() {
		//cerr << "Size of extensions list = " << this->extns.size() << endl;
		return not( this->extns.empty() );
	}

	string next() {
		string s = this->extns.back();
		this->extns.pop_back();
		return s;
	}

};

class ExtnLookup : public Ginger::SaxHandler {
private:
	const string mnx_file_name;
	string extn;
	bool found;
	string parser;

public:
	ExtnLookup( const string & _extn, const string & _config_file ) :
		mnx_file_name( _config_file ),
		extn( _extn ),
		found( false )
	{
	}
	
public:
	typedef map< string, string > Dict;
	
	void startTag( string & name, Dict & attrs ) {
		if ( name != PARSER ) return;
		Dict::iterator it = attrs.find( PARSER_EXT );
		Dict::iterator jt = attrs.find( PARSER_EXE );
		if ( it != attrs.end() && extn == it->second && jt != attrs.end() ) {
			this->found = true;
			this->parser = jt->second;
		} 
	}

	void endTag( std::string & name ) {
	}

private:

	bool lookup() {
		ifstream stream( this->mnx_file_name.c_str() );
		if ( stream.good() ) {
			Ginger::SaxParser saxp( stream, *this );
			saxp.readElement();
			return this->found;
		} else {
			return false;
		}
	}

	
public:
	bool hasNext() {
		if ( this->found ) return false;
		this->lookup();
		return this->found;
	}

	string next() {
		return this->parser;
	}

};

class StdExtnLookup {
private:
	const string & extn;
	bool done;
	const char * found;

public:
	StdExtnLookup( const string & _extn ) : extn( _extn ), done( false ) {}

private:
	static const char * defaultCommand( const string & ex ) {
		if ( ex == "cmn" || ex == "common" ) {
			return COMMON2GNX;
		} else if ( ex == "cst" || ex == "cstyle" ) {
			return CSTYLE2GNX;
		} else if ( ex == "lsp" || ex == "lisp" ) {
			return LISP2GNX;
		} else if ( ex == "gnx" ) {
			return GNX2GNX;
		} else {
			return NULL;
		}	
	}

public:
	bool hasNext() {
		if ( this->done ) return false;
		this->found = defaultCommand( this->extn );
		this->done = true;
		return this->found != NULL;
	}

	string next() {
		return this->found;
	}
};


void printGPL( const char * start, const char * end ) {
    bool printing = false;
    ifstream license( LICENSE_FILE );
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
}

static void printUsage() {
	cout << "Usage :  " << APP_TITLE << " [OPTIONS] [FILE]" << endl << endl;
	cout << "OPTION                SUMMARY" << endl;
	cout << "-g, --grammar=LANG    default front-end syntax" << endl;
	cout << "-H, --help            print out this help info (see --help=help)" << endl;
	cout << "-i, --stdin           compile from stdin" << endl;
	cout << "-L, --license         print out license information and exit" << endl;
    cout << "-V, --version         print out version information and exit" << endl;
	cout << endl;
}	

static void printHelpOptions() {
	cout << "--help=help           this short help" << endl;
	cout << "--help=licence        help on displaying license information" << endl;
}

static void printHelpLicense() {
	cout << "Displays key sections of the GNU Public License." << endl;
	cout << "--license=warranty    Shows warranty." << endl;
	cout << "--license=conditions  Shows terms and conditions." << endl;
}

int printLicense( const char * arg ) {
	if ( arg == NULL || std::string( arg ) == std::string( "all" ) ) {
		printGPL( NULL, NULL );
	} else if ( std::string( arg ) == std::string( "warranty" ) ) {
		printGPL( "Disclaimer of Warranty.", "Limitation of Liability." );                 
	} else if ( std::string( arg ) == std::string( "conditions" ) ) {
		printGPL( "TERMS AND CONDITIONS", "END OF TERMS AND CONDITIONS" );
	} else {
		std::cerr << "Unknown license option: " << arg << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}


/*	USAGE:
	file2gnx FILE
	file2gnx < FILE
	file2gnx -g grammar < FILE
	file2gnx -i < FILE 
	file2gnx -g GRAMMAR FILE
	file2gnx -i -g GRAMMAR < FILE
*/


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
        { "help",           optional_argument,      0, 'H' },
        { "grammar",		required_argument,		0, 'g' },
        { "license",        optional_argument,      0, 'L' },
        { "stdin",			no_argument,			0, 'i' },
        { "version",        no_argument,            0, 'V' },
        { 0, 0, 0, 0 }
    };

class Main {
private:
	bool use_grammar;
	bool use_stdin;
	string grammar;
   	vector< string > files;
	    
public:

	Main() :
		use_grammar( false ),
		use_stdin( false )
	{}

	bool parse( int argc, char ** argv ) {
		openlog( APP_TITLE, 0, LOG_FACILITY );
		setlogmask( LOG_UPTO( LOG_INFO ) );

		for(;;) {
	        int option_index = 0;
	        int c = getopt_long( argc, argv, "g:H::iL::V", long_options, &option_index );
	        //cerr << "Got c = " << c << endl;
	        if ( c == -1 ) break;
	        switch ( c ) {
	            case 'g': {
	            	this->use_grammar = true;
	            	this->grammar = optarg;
	            	break;
	            }
	        	case 'H': {
	                if ( optarg == NULL ) {
	                	printUsage();
	                } else if ( std::string( optarg ) == "help" ) {
	                	printHelpOptions();
	                } else if ( std::string( optarg ) == "license" ) {
	                	printHelpLicense();
	                } else {
	                    cout << "Unknown help topic " << optarg << endl;
	                }
	                return false;
	          	}
	            case 'i': {
	            	this->use_stdin = true;
	            	break;
	            }
	            case 'L': {
	            	printLicense( optarg );
	            	return false;
	            }
				case 'V': {
	                cout << APP_TITLE << ": version " << PACKAGE_VERSION << " (" << __DATE__ << " " << __TIME__ << ")" << endl;
	            	return false;
	            }
	            default: {
	            	cerr << "Unrecognised option: " << static_cast< long >( c ) << endl;
	            	exit( EXIT_FAILURE );
	            }

	       	}
	   	}


	    //	Aggregate the remaining arguments, which are effectively filenames (paths).
		if ( optind < argc ) {
			 while ( optind < argc ) {
			   	this->files.push_back( argv[ optind++ ] );
			 }
		}

		//	Now rationalise the options.
		{
			int nfiles = files.size();
			if ( nfiles == 0 ) {
				this->use_stdin = true;
				this->use_grammar = true;
			} else if ( nfiles > 1 ) {
				printUsage();
				exit( EXIT_FAILURE );
			}
		}

		return true;
	}

private:

	void run( const string & command, const vector< const char * > & args ) {
		if ( args.size() > 1 ) {
			throw Unreachable();
		}
		const char * cmd = command.c_str();
		if ( args.empty() ) {
			execl( cmd, cmd, (char)0 );
		} else {
			execl( cmd, cmd, args[ 0 ], (char)0 );
		}
	}

	void useExtnFinder( ExtnFinder & efinder, const vector< const char * > & args ) {
		while ( efinder.hasNext() ) {
			const string extn( efinder.next() );
			//cerr << "EXTENSION " << extn << endl;
			StdExtnLookup stdlookup( extn );
			while ( stdlookup.hasNext() ) {
				//cerr << "Doing std lookup for " << extn << endl;
				string cmdpath( stdlookup.next() );
				run( cmdpath, args );
			}
			XDGConfigFiles cfiles( FILE2GNX_CONFIG_BASE );
			while ( cfiles.hasNext() ) {
				const string cfile( cfiles.next() );
				ExtnLookup elookup( extn, cfile );
				//cerr << "Doing config lookup for " << extn << " in " << cfile << endl;
				while ( elookup.hasNext() ) {
					string cmdpath( elookup.next() );
					run( cmdpath, args );
				}
			}
		}
	}


public:
	
	void main() {
		vector< const char * > args;
		ExtnFinder efinder;
		if ( this->use_grammar ) {
			efinder.addGrammar( this->grammar );
		}
		if ( not this->use_stdin ) {
			const string & pathname( this->files[ 0 ] );
			args.push_back( pathname.c_str() );
			efinder.tryAddExtnOfPathName( pathname );
		}		
		this->useExtnFinder( efinder, args );
		throw SystemError( "Cannot detect syntax, giving up" ).culprit( "Source", this->use_stdin ? string( "<stdin>" ) : this->files[ 0 ] );
	}

};

int main( int argc, char ** argv ) {
	try {
		Main main;
		if ( main.parse( argc, argv ) ) {
			main.main();
		}
		return EXIT_SUCCESS;
	} catch ( Ginger::Mishap & m ) {
		m.culprit( "Detected by", APP_TITLE );
		m.gnxReport();
		return EXIT_FAILURE;
	}
}