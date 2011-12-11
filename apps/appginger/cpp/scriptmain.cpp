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

#define APP_TITLE "ginger-script"

#define SIMPLIFYGNX		( INSTALL_TOOL "/simplifygnx" )
#define COMMON2GNX		( INSTALL_TOOL "/common2gnx" )
#define LISP2GNX		( INSTALL_TOOL "/lisp2gnx" )
#define GSON2GNX		( INSTALL_TOOL "/gson2gnx" )
#define TAIL			"/usr/bin/tail"


using namespace std;


class ScriptMain : public ToolMain {
public:
	int run() {		
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
						
							const int posn = filestr.tellg();
							stringstream commstream;
							//	tail is 1-indexed!
							commstream << TAIL << " -c+" << ( posn + 1 );
							commstream << " "<< safeFileName( *it );
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
						rcep.execGnx( m, cout );
					}
				}
			}
			filestr.close();
		}	
	    return EXIT_SUCCESS;
	}

public:
	ScriptMain( const char * name ) : ToolMain( name ) {
		this->context.setCgiMode();
	}
	
	virtual ~ScriptMain() {}
};

int main( int argc, char **argv, char **envp ) {
	openlog( APP_TITLE, 0, LOG_LOCAL2 );
	setlogmask( LOG_UPTO( LOG_INFO ) );
	try {
		ScriptMain main( APP_TITLE );
		return main.parseArgs( argc, argv, envp ) ? main.run() : EXIT_SUCCESS;
	} catch ( Ginger::SystemError & e ) {
		e.report();
		return EXIT_FAILURE;
	}
}
