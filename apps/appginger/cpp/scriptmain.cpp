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

#include <fstream>
#include <string>
#include <sstream>

#include <syslog.h>

#include <ext/stdio_filebuf.h> // __gnu_cxx::stdio_filebuf

#include "mnx.hpp"
#include "mishap.hpp"

#include "rcep.hpp"
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
private:
	void runFrom( RCEP & rcep, Ginger::MnxReader & gnx_read ) {
		for (;;) {
			shared< Ginger::Mnx > m = gnx_read.readMnx();
			if ( not m ) break;
			rcep.execGnx( m, cout );
			rcep.printResults( cout, 0 );
		}			
	}
	
public:
	int run() {		
		MachineClass * vm = this->context.newMachine();
		Package * interactive_pkg = this->context.initInteractivePackage( vm );
		RCEP rcep( interactive_pkg );
	
		vector< string > & args = this->context.arguments();
		for ( vector< string >::iterator it = args.begin(); it != args.end(); ++it ) {
			stringstream commstream;
			//	tail is 1-indexed!
			commstream << this->context.syntax( *it ) << " < " << shellSafeName( *it ) << " | ";
			commstream << SIMPLIFYGNX << " -suA";
			commstream << " -p " << shellSafeName( interactive_pkg->getTitle() );
			string command( commstream.str() );
			//	cerr << "Command so far: " << command << endl;
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
		if ( this->context.useStdin() ) {
			stringstream commstream;
			//	tail is 1-indexed!
			commstream << this->context.syntax() << " | ";
			commstream << SIMPLIFYGNX << " -suA";
			commstream << " -p " << shellSafeName( interactive_pkg->getTitle() );
			string command( commstream.str() );

			FILE * gnxfp = popen( command.c_str(), "r" );
			if ( gnxfp == NULL ) {
				throw Ginger::Mishap( "Failed to translate input" );
			}
		
			__gnu_cxx::stdio_filebuf<char> pipe_buf( gnxfp, ios_base::in );
			istream stream_pipe_in( &pipe_buf );
			while ( rcep.unsafe_read_comp_exec_print( stream_pipe_in, std::cout ) ) {}
			
			pclose( gnxfp );
		}
	    return EXIT_SUCCESS;
	}

public:
	ScriptMain( const char * name ) : ToolMain( name ) {
		this->context.printDetailLevel() = 1;
	}
	
	virtual ~ScriptMain() {}
};

int main( int argc, char **argv, char **envp ) {
	openlog( APP_TITLE, 0, LOG_LOCAL2 );
	setlogmask( LOG_UPTO( LOG_INFO ) );
	try {
		ScriptMain main( APP_TITLE );
		return main.parseArgs( argc, argv, envp ) ? main.run() : EXIT_SUCCESS;
	} catch ( Ginger::Problem & e ) {
		e.report();
		return EXIT_FAILURE;
	}
}
