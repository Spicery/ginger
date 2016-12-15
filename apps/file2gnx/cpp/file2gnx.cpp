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

#include <sys/wait.h>

#include "config.h"

#include "maybe.hpp"
#include "printgpl.hpp"
#include "xdgconfigfiles.hpp"
#include "mnx.hpp"
#include "sax.hpp"
#include "mishap.hpp"

#include "pathparser.hpp"

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
#define PARSER_TYPE "type"
#define PARSER_TYPE_SINGLE "value"
#define PARSER_TYPE_BINDING "binding"

struct Command {
	string command;
	bool single_valued_expr;
public:
	Command() : single_valued_expr( false ) {}
};

class ExtnLookup : public Ginger::SaxHandler {
private:
	const string mnx_file_name;
	string extn;
	bool found;
	bool done;
	Command cmd;

public:
	ExtnLookup( const string & _extn, const string & _config_file ) :
		mnx_file_name( _config_file ),
		extn( _extn ),
		found( false ),
		done( false )
	{
	}
	
public:
	typedef map< string, string > Dict;
	
	void startTag( string & name, Dict & attrs ) {
		if ( name != PARSER ) return;
		Dict::iterator it = attrs.find( PARSER_EXT );
		Dict::iterator jt = attrs.find( PARSER_EXE );
		Dict::iterator kt = attrs.find( PARSER_TYPE );
		if ( it != attrs.end() && extn == it->second && jt != attrs.end() ) {
			this->found = true;
			cmd.command = jt->second;
			cmd.single_valued_expr = ( kt->second == PARSER_TYPE_SINGLE );
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
		if ( this->done ) return false;
		if ( this->found ) return true;
		this->lookup();
		return this->found;
	}

	Command next() {
		this->done = true;
		return this->cmd;
	}

};

class StdExtnLookup {
private:
	const string & extn;
	bool done;
	Command cmd;

public:
	StdExtnLookup( const string & _extn ) : extn( _extn ), done( false ) {}

private:
	static string defaultCommand( const string & ex ) {
		if ( ex == "cmn" || ex == "common" ) {
			return COMMON2GNX;
		} else if ( ex == "cst" || ex == "cstyle" ) {
			return CSTYLE2GNX;
		} else if ( ex == "lsp" || ex == "lisp" ) {
			return LISP2GNX;
		} else if ( ex == "gnx" ) {
			return GNX2GNX;			
		} else {
			return "";
		}	
	}

public:
	bool hasNext() {
		if ( this->done ) return false;
		this->cmd.single_valued_expr = not this->extn.empty() and this->extn[0] == '1';
		if ( not this->cmd.single_valued_expr ) {
			this->cmd.command = defaultCommand( this->extn );
		} else {
			//cerr << "Process " << this->extn.substr( 1 ) << endl;
			this->cmd.command = defaultCommand( this->extn.substr( 1 ) );
		}
		return not this->cmd.command.empty();
	}

	Command next() {
		this->done = true;
		return this->cmd;
	}

};


/*	USAGE:
	file2gnx [--using-name FILENAME] FILE
	file2gnx --using-name FILENAME < FILE
*/

static void printUsage() {
	cout << "Usage :  " << APP_TITLE << " [OPTIONS] [FILE]" << endl << endl;
	cout << "OPTION                    SUMMARY" << endl;
	cout << "-u, --using-name FILE     use FILE to determine the grammar and variable name" << endl;
	cout << "-g, --grammar=NAME        specify the grammar by name" << endl;
	cout << "-H, --help                print out this help info (see --help=help)" << endl;
	cout << "-L, --license             print out license information and exit" << endl;
    cout << "-V, --version             print out version information and exit" << endl;
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
        { "using-name",		optional_argument,		0, 'u' },
        { "grammar",        optional_argument,      0, 'g' },
        { "help",           optional_argument,      0, 'H' },
        { "license",        optional_argument,      0, 'L' },
        { "version",        no_argument,            0, 'V' },
        { 0, 0, 0, 0 }
    };

class Main {
private:
	Maybe< string > using_name;
	Maybe< string > file;
	string var_name;
	string grammar;
	    
public:

	bool parse( int argc, char ** argv ) {
		openlog( APP_TITLE, 0, LOG_FACILITY );
		setlogmask( LOG_UPTO( LOG_INFO ) );

		for(;;) {
	        int option_index = 0;
	        int c = getopt_long( argc, argv, "g:u:H::L::V", long_options, &option_index );
	        //cerr << "Got c = " << c << endl;
	        if ( c == -1 ) break;
	        switch ( c ) {
	            case 'u': {
	            	this->using_name.setValue( string( optarg ) );
	            	break;
	            }
	            case 'g': {
	            	this->using_name.setValue( string( "." ) + optarg );
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
	            case 'L': {
	            	exit( Ginger::optionPrintGPL( optarg ) );
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
	   	vector< string > files;
		if ( optind < argc ) {
			while ( optind < argc ) {
			   	 files.push_back( argv[ optind++ ] );
			}
		}

		//	Now rationalise the options.
		{
			int nfiles = files.size();
			if ( nfiles == 1 ) {
				this->file.setValue( files[ 0 ] );
			} else if ( nfiles > 1 ) {
				printUsage();
				exit( EXIT_FAILURE );
			}
		}

		if ( this->using_name.isValid() ) {
			PathParser pp( this->using_name.fastValue() );
			this->var_name = pp.stem();
			this->grammar = pp.extension();
		} else if ( this->file.isValid() ) {
			PathParser pp( this->file.fastValue() );
			this->var_name = pp.stem();
			this->grammar = pp.extension();
		} else {
			throw Ginger::Mishap( "Cannot determine effective file-name" ).culprit( "Hint", "Missing --using-name option?" );
		}

		return true;
	}

private:

	void runExec( const string & command ) {
		const char * cmd = command.c_str();
		if ( this->file.isValid() ) {
			execl( cmd, cmd, this->file.fastValue().c_str(), (char)0 );
		} else {
			execl( cmd, cmd, (char)0 );
		}
	}

	void runWrap( const string & command ) {
		cout << "<bind><var name=\"";
		Ginger::mnxRenderText( cout, this->var_name );
		cout << "\" protected=\"true\"/>" << endl;
		vector< string > strargs;
		strargs.push_back( command );
		if ( this->file.isValid() ) {
			strargs.push_back( this->file.fastValue() );
		}
		const pid_t pid = fork();
		if ( pid != 0 ) {
			//	Parent.
			int retval;
			waitpid( pid, &retval, 0 );
		} else {
			runExec( command );
		}
		cout << "</bind>" << endl;
		exit( EXIT_SUCCESS );
	}

	void run( const Command & cmd ) {
		if ( cmd.single_valued_expr ) {
			runWrap( cmd.command );
		} else {
			runExec( cmd.command );
		}
	}

public:
	void main() {
		StdExtnLookup stdlookup( this->grammar );
		while ( stdlookup.hasNext() ) {
			run( stdlookup.next() );
		}
		XDGConfigFiles cfiles( FILE2GNX_CONFIG_BASE );
		while ( cfiles.hasNext() ) {
			const string cfile( cfiles.next() );
			ExtnLookup elookup( this->grammar, cfile );
			//cerr << "Doing config lookup for " << extn << " in " << cfile << endl;
			while ( elookup.hasNext() ) {
				run( elookup.next() );
			}
		}
		throw (
			SystemError( "Cannot detect syntax, giving up" ).
			culprit( "Grammar", this->grammar ).
			culprit( "Source", this->file.isValid() ? this->file.fastValue() : string( "<stdin>" ) )
		);
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
