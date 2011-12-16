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
#include <signal.h>

#include <ext/stdio_filebuf.h> // __gnu_cxx::stdio_filebuf

#include "mishap.hpp"
#include "gngversion.hpp"

#include "appcontext.hpp"
#include "toolmain.hpp"
#include "rcep.hpp"

#define APP_TITLE "ginger"

#define SIMPLIFYGNX		( INSTALL_TOOL "/simplifygnx" )
#define COMMON2GNX		( INSTALL_TOOL "/common2gnx" )
#define LISP2GNX		( INSTALL_TOOL "/lisp2gnx" )
#define GSON2GNX		( INSTALL_TOOL "/gson2gnx" )


using namespace std;

static void printWelcomeMessage() {
	cout << PACKAGE_NAME << ": " << PACKAGE_VERSION << ", Copyright (c) 2010  Stephen Leach" << endl;
	cout << "  +----------------------------------------------------------------------+" << endl;
	cout << "  | This program comes with ABSOLUTELY NO WARRANTY. It is free software, |" << endl;
	cout << "  | and you are welcome to redistribute it under certain conditions.     |" << endl;
	cout << "  | Use option --help=license for details.                               |" << endl;
	cout << "  +----------------------------------------------------------------------+" << endl;
}

static bool sigint_dummy;
static bool * SIGINT_FLAG = &sigint_dummy;

void sigint_handler( int s ){
	cout << "Caught signal " << s << endl;
	*SIGINT_FLAG = true;
}

class Popen {
private:
	FILE * gnufp;
	
public:
	FILE * file() { return this->gnufp; }
	
public:
	Popen( string command ) : gnufp( NULL ) {
		this->gnufp = popen( command.c_str(), "r" );
		if ( this->gnufp == NULL ) {
			throw Ginger::SystemError( "Failed to open subprocesses" );	
		}
	}
	~Popen() { if ( this->gnufp != NULL ) pclose( this->gnufp ); }
};

class GingerMain : public ToolMain {
private:
	void mainLoop() {
	
		MachineClass * vm = this->context.newMachine();
		SIGINT_FLAG = &vm->sigint_flag;

		struct sigaction sigIntHandler;
		sigIntHandler.sa_handler = sigint_handler;
		sigemptyset( &sigIntHandler.sa_mask );
		sigIntHandler.sa_flags = SA_RESTART;
		sigaction( SIGINT, &sigIntHandler, NULL );

		Package * interactive_pkg = this->context.initInteractivePackage( vm );
	 
		#ifdef DBG_APPCONTEXT
			clog << "RCEP ..." << endl;
		#endif
	
		RCEP rcep( interactive_pkg );
		
		//cout << "Using syntax: " << this->context.syntax() << endl;
		
		stringstream commstream;
		//	tail is 1-indexed!
		commstream << this->context.syntax() << " | " << SIMPLIFYGNX << " -s";
		string command( commstream.str() );
		
		for (;;) {
			try {
				{
					//	This block is required to neatly force the closure of the
					//	piped.
					Popen p( commstream.str() );		
					// ... open the file, with whatever, pipes or who-knows ...
					// let's build a buffer from the FILE* descriptor ...
					__gnu_cxx::stdio_filebuf<char> pipe_buf( p.file(), ios_base::in );
					// there we are, a regular istream is build upon the buffer :
					istream stream_pipe_in( &pipe_buf );
					
					while ( rcep.read_comp_exec_print( stream_pipe_in, std::cout ) ) {}
				}
	
				//	Check whether or not we were interrupted by a signal.			
				if ( not vm->sigint_flag ) break;
				cout << "SIGINT got trapped" << endl;
				vm->sigint_flag = false;
			} catch ( Ginger::CompileTimeError & e ) {
				e.report();
				cout << endl << SYS_MSG_PREFIX << "Reset..." << endl;
				vm->resetMachine();
			}
		}

	}

public:
	virtual int run() {
		if ( this->context.welcomeBanner() ) printWelcomeMessage();
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
	} catch ( Ginger::Problem & e ) {
		e.report();
		return EXIT_FAILURE;
	}
}
