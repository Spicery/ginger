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

//#include <iostream>
#include <fstream>
//#include <vector>
#include <string>

//#include <cstdio>
//#include <cstdlib>
#include <sstream>

#include <ext/stdio_filebuf.h> // __gnu_cxx::stdio_filebuf

//#include <unistd.h>
//#include <getopt.h>
//#include <syslog.h>

#include <rude/cgi.h>

//#include "gngversion.hpp"
#include "mnx.hpp"

//#include "appcontext.hpp"
#include "rcep.hpp"
//#include "term.hpp"
//#include "sys.hpp"
//#include "machine1.hpp"
//#include "machine2.hpp"
//#include "machine3.hpp"


#include <syslog.h>

#include "mishap.hpp"

#include "appcontext.hpp"
#include "toolmain.hpp"

#define APP_TITLE "ginger-cgi"

#define SIMPLIFYGNX		( INSTALL_TOOL "/simplifygnx" )
#define COMMON2GNX		( INSTALL_TOOL "/common2gnx" )
#define LISP2GNX		( INSTALL_TOOL "/lisp2gnx" )
#define GSON2GNX		( INSTALL_TOOL "/gson2gnx" )
#define TAIL			"/usr/bin/tail"


using namespace std;

class CgiResponse {
private:
	int count;
	string ctype;

public:
	CgiResponse() :
		count( 0 ), 
		ctype( "text/plain" ) 
	{
	}
	
public:
	void setContentType( const string & type ) {
		this->ctype = type;
	}

	void open() {
		if ( this->count++ != 0 ) return;
		cout << "Content-type: " << ctype << "\r\n\r\n";
	}
	
	void close() {
		if ( count == 0 ) {
			cout << "Content-type: text/html\r\n\r\n";
			cout << "<html><head><title>AppGinger</title></head><body>\n";
			cout << "Empty script\n";
			cout << "</body></html>\n";
		}
	}
};


class CgiMain : public ToolMain {
public:
	int run() {
		this->context.initCgi();
		
		MachineClass * vm = this->context.newMachine();
		Package * interactive_pkg = this->context.initInteractivePackage( vm );
		RCEP rcep( interactive_pkg );
	
		vector< string > & args = this->context.arguments();
		for ( vector< string >::iterator it = args.begin(); it != args.end(); ++it ) {
			//cout << "<H2>" << *it << "</H2>" << endl;
			fstream filestr( it->c_str(), fstream::in );
			string line;
			if ( filestr.good() ) {
				//	Chew off first line.
				getline( filestr, line );
				//cout << line << "<BR/>" << endl;
			}
			if ( filestr.good() ) {
				Ginger::MnxReader content( filestr );
				CgiResponse response;
				for (;;) {
					shared< Ginger::Mnx > m = content.readMnx();
					if ( not m ) break;
					if ( m->name() == "option" ) {
						if ( m->hasAttribute( "projectFolder" ) ) {
							this->context.addProjectFolder( m->attribute( "projectFolder" ) );
						}
						if ( m->hasAttribute( "machine" ) ) {
							this->context.setMachineImplNum( atoi( m->attribute( "machine" ).c_str() ) );	
						}
						if ( m->hasAttribute( "content-type" ) ) {
							response.setContentType( m->attribute( "content-type" ) );
						}
						if ( m->hasAttribute( "language" ) ) {
							//	We are guaranteed to be just after the '>' of
							//	the option.
							//	We need to skip forward until we find an end-of-line.
							getline( filestr, line );
						
							string lang(
								m->hasAttribute( "language", "common" ) ? COMMON2GNX :
								m->hasAttribute( "language", "lisp" ) ? LISP2GNX :
								m->hasAttribute( "language", "gson" ) ? GSON2GNX :
								( throw Ginger::Mishap( "Unrecognised language" ).culprit( "Language", m->attribute( "language" ) ) )
							);
						
							response.open();
							const int posn = filestr.tellg();
							stringstream commstream;
							//	tail is 1-indexed!
							commstream << TAIL << " -c+" << ( posn + 1 );
							commstream << " "<< shellSafeName( *it );
							commstream << " | " << lang << " | " << SIMPLIFYGNX << " -s";
							string command( commstream.str() );
							//cout << "Command so far: " << command << endl;
							FILE * gnxfp = popen( command.c_str(), "r" );
							if ( gnxfp == NULL ) {
								throw Ginger::Mishap( "Failed to translate input" );
							}
							// ... open the file, with whatever, pipes or who-knows ...
							// let's build a buffer from the FILE* descriptor ...
							__gnu_cxx::stdio_filebuf<char> pipe_buf( gnxfp, ios_base::in );
							// there we are, a regular istream is build upon the buffer :
							istream stream_pipe_in( &pipe_buf );
							
							Ginger::MnxReader gnx_read( stream_pipe_in );
							for (;;) {
								shared< Ginger::Mnx > m = gnx_read.readMnx();
								if ( not m ) break;
								rcep.execGnx( m, cout );
							}
							
							pclose( gnxfp );
							break;
						}
					} else {
						response.open();
						rcep.execGnx( m, cout );
					}
				}
				response.close();
			}
			filestr.close();
		}	
	    return EXIT_SUCCESS;
	}

public:
	CgiMain( const char * name ) : ToolMain( name ) {
	}
	
	virtual ~CgiMain() {}
};

int main( int argc, char **argv, char **envp ) {
	openlog( APP_TITLE, 0, LOG_LOCAL2 );
	setlogmask( LOG_UPTO( LOG_INFO ) );
	try {
		CgiMain main( APPGINGER_NAME );
		return main.parseArgs( argc, argv, envp ) ? main.run() : EXIT_SUCCESS;
	} catch ( Ginger::SystemError & e ) {
		e.report();
		return EXIT_FAILURE;
	}
}
