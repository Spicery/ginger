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
#include <sstream>
#include <cstdlib>

#include <syslog.h>

#include <ext/stdio_filebuf.h> // __gnu_cxx::stdio_filebuf

#include "mishap.hpp"

#include "appcontext.hpp"
#include "toolmain.hpp"
#include "rcep.hpp"

#define APP_TITLE "ginger"

#define SIMPLIFYGNX		( INSTALL_TOOL "/simplifygnx" )
#define COMMON2GNX		( INSTALL_TOOL "/common2gnx" )
#define LISP2GNX		( INSTALL_TOOL "/lisp2gnx" )
#define GSON2GNX		( INSTALL_TOOL "/gson2gnx" )

#define RLCAT			( INSTALL_TOOL "/rlcat" )

using namespace std;

static void printWelcomeMessage() {
	cout << PACKAGE_NAME << ": " << PACKAGE_VERSION << ", Copyright (c) 2010  Stephen Leach" << endl;
	cout << "  +----------------------------------------------------------------------+" << endl;
	cout << "  | This program comes with ABSOLUTELY NO WARRANTY. It is free software, |" << endl;
	cout << "  | and you are welcome to redistribute it under certain conditions.     |" << endl;
	cout << "  | Use option --help=license for details.                               |" << endl;
	cout << "  +----------------------------------------------------------------------+" << endl;
}

class GingerMain : public ToolMain {
private:
	void mainLoop() {
		MachineClass * vm = this->context.newMachine();
		Package * interactive_pkg = this->context.initInteractivePackage( vm );
	 
		#ifdef DBG_APPCONTEXT
			clog << "RCEP ..." << endl;
		#endif
	
		RCEP rcep( interactive_pkg );
		
		/*string lang(
			m->hasAttribute( "language", "common" ) ? COMMON2GNX :
			m->hasAttribute( "language", "lisp" ) ? LISP2GNX :
			m->hasAttribute( "language", "gson" ) ? GSON2GNX :
			( throw Ginger::Mishap( "Unrecognised language" ).culprit( "Language", m->attribute( "language" ) ) )
		);*/
		string lang( COMMON2GNX );
	
		stringstream commstream;
		//	tail is 1-indexed!
		commstream << lang << " | " << SIMPLIFYGNX << " -s";
		string command( commstream.str() );
		
		//cerr << "Command so far: " << command << endl;
		FILE * gnxfp = popen( command.c_str(), "r" );
		if ( gnxfp == NULL ) {
			throw Ginger::Mishap( "Failed to translate input" );
		}
		// ... open the file, with whatever, pipes or who-knows ...
		// let's build a buffer from the FILE* descriptor ...
		__gnu_cxx::stdio_filebuf<char> pipe_buf( gnxfp, ios_base::in );
		// there we are, a regular istream is build upon the buffer :
		istream stream_pipe_in( &pipe_buf );
		
		while ( rcep.read_comp_exec_print( stream_pipe_in, std::cout ) ) {};
		
		pclose( gnxfp );
	}

public:
	virtual int run() {
		printWelcomeMessage();
		this->mainLoop();
		return EXIT_SUCCESS;
	}

public:
	GingerMain( const char * name ) : ToolMain( name ) {
		this->context.printLevel() = 2;
	}
	
	virtual ~GingerMain() {}
};

int main( int argc, char **argv, char **envp ) {
	openlog( APPGINGER_NAME, 0, LOG_LOCAL2 );
	setlogmask( LOG_UPTO( LOG_INFO ) );
	try {
		GingerMain main( APP_TITLE );
		return main.parseArgs( argc, argv, envp ) ? main.run() : EXIT_SUCCESS;
	} catch ( Ginger::SystemError & e ) {
		e.report();
		return EXIT_FAILURE;
	}
}
