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
#include "command.hpp"

#include "appcontext.hpp"
#include "toolmain.hpp"
#include "rcep.hpp"

#define APP_TITLE "ginger"

#include "wellknownpaths.hpp"


using namespace std;

//#define DBG_CLIMAIN

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
	cerr << "Interrupted (signal " << s << ")" << endl;
	*SIGINT_FLAG = true;
}

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
	 	RCEP rcep( interactive_pkg );

	 	try {
	 		this->executeLoadFileList( rcep );
	 		this->executeFileArguments( rcep );
	 	} catch ( Ginger::Mishap & e ) {
	 		if ( e.isSystemError() ) throw;
			e.report();
			cerr << endl << SYS_MSG_PREFIX << "Reset after runtime error ..." << endl;
			vm->resetMachine();
	 	}

		for (;;) {
			#ifdef DBG_CLIMAIN
				cerr << "Entering executeStdin" << endl;
			#endif
			try {
				this->executeStdin( true, rcep );
				#ifdef DBG_CLIMAIN
					cerr << "Loop finished naturally, end of input" << endl;
				#endif
				break;
			} catch ( Ginger::Mishap & e ) {
				#ifdef DBG_CLIMAIN
					cerr << "Mishap detected" << endl;
				#endif
				if ( e.isSystemError() ) throw;
				e.report();
				cerr << endl << SYS_MSG_PREFIX << "Reset after runtime error ..." << endl;
				vm->resetMachine();
		 	}
		 	#ifdef DBG_CLIMAIN
				cerr << "Back to loop start" << endl;
			#endif

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
		this->context.initShell();
		this->context.printDetailLevel().setChatty();
	}
	
	virtual ~GingerMain() {}
};

int main( int argc, char **argv, char **envp ) {
	openlog( APPGINGER_NAME, 0, LOG_LOCAL2 );
	setlogmask( LOG_UPTO( LOG_INFO ) );
	try {
		GingerMain main( APP_TITLE );
		return main.parseArgs( argc, argv, envp ) ? main.run() : EXIT_SUCCESS;
	} catch ( Ginger::Mishap & e ) {
		e.report();
		return EXIT_FAILURE;
	}
}
