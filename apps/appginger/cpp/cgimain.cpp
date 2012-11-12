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
#include <cstdlib>

#include <ext/stdio_filebuf.h> // __gnu_cxx::stdio_filebuf

#include <rude/cgi.h>

#include "mnx.hpp"

#include "rcep.hpp"
#include "sysprint.hpp"


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

class CgiMain : public ToolMain {
public:
	int run() {
		this->context.initCgi();
		
		MachineClass * vm = this->context.newMachine();
		Package * interactive_pkg = this->context.initInteractivePackage( vm );
		RCEP rcep( interactive_pkg );
		rcep.isPrinting() = false;		//	Turn off printing.
		
		vector< string > & args = this->context.arguments();
		for ( vector< string >::iterator it = args.begin(); it != args.end(); ++it ) {
			const string filename( *it );

			stringstream commstream;
			//	tail is 1-indexed!
			commstream << this->context.syntax( filename ) << " < " ;
			commstream << shellSafeName( filename ) << " | ";
			commstream << SIMPLIFYGNX << " -suA";

			{
				list< string > & folders = vm->getAppContext().getProjectFolderList();
				for ( 
					list< string >::iterator it = folders.begin();
					it != folders.end();
					++it
				) {
					commstream << " -j" << *it;
				}
			}
			
			commstream << " -p " << shellSafeName( interactive_pkg->getTitle() );
			string command( commstream.str() );
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
	} catch ( Ginger::Problem & e ) {
		e.report();
		return EXIT_FAILURE;
	}
}
