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

class Popen : public Ginger::Command {
private:
	FILE * fp;
	
public:
	FILE * file() { return this->fp; }
	
	void cleanShutdown() {
		this->fp = NULL;
	}

public:
	Popen( const string command ) : 
		Command( "/bin/sh" )
	{
		this->addArg( "-c" );
		this->addArg( command );
		this->runWithOutput();
		fp = fdopen( this->getInputFD(), "r" );
		#ifdef DBG_POPEN
			cerr << "Opening popen " << this->child_pid << endl;
		#endif
	}
	
	~Popen() {
		#ifdef DBG_POPEN
			cerr << "Closing popen" << endl;
		#endif
		if ( fp != NULL ) {
			#ifdef DBG_POPEN
				cerr << "Interrupting" << endl;
			#endif
			this->interrupt();
			#ifdef DBG_POPEN
				cerr << "Closing (1)" << endl;
			#endif
			fclose( fp );
			#ifdef DBG_POPEN
				cerr << "Closing (2)" << endl;
			#endif
		}
	}
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
	 	RCEP rcep( interactive_pkg );

	 	try {
	 		this->executeLoadFileList( rcep );
	 		this->executeFileArguments( rcep );
	 	} catch ( Ginger::Mishap & e ) {
			e.report();
			cerr << endl << SYS_MSG_PREFIX << "Reset after runtime error ..." << endl;
			vm->resetMachine();
	 	} catch ( Ginger::CompileTimeError & e ) {
			e.report();
			cerr << endl << SYS_MSG_PREFIX << "Reset after compilation error ..." << endl;
			vm->resetMachine();
	 	}
		
		/**********************************************************************\
		*	BLOCK1, replacement for BLOCK2, see comment below. This is the
		* 	version I want to use.
		\**********************************************************************/
	
		#ifndef BLOCK2
		for (;;) {
			try {
				this->executeStdin( rcep );
				break;
			} catch ( Ginger::Mishap & e ) {
				e.report();
				cerr << endl << SYS_MSG_PREFIX << "Reset after runtime error ..." << endl;
				vm->resetMachine();
		 	} catch ( Ginger::CompileTimeError & e ) {
				e.report();
				cerr << endl << SYS_MSG_PREFIX << "Reset after compilation error ..." << endl;
				vm->resetMachine();
		 	}
		}
		#endif
		
		/**********************************************************************\
		*	The following code BLOCK2 is commented out until testing confirms
		* 	the BLOCK1 code correctly replaces it. I had some strange 
		*	compilation & runtime issues that I solved by trial and error
		* 	Hopefully BLOCK1, which is perfectly elegant, solves this fully.
		\**********************************************************************/

		#ifdef BLOCK2
		bool cont = true;
		while ( cont ) {
			try {
				Popen p( this->context.syntax() );		
				// ... open the file, with whatever, pipes or who-knows ...
				// let's build a buffer from the FILE* descriptor ...
				__gnu_cxx::stdio_filebuf<char> pipe_buf( p.file(), ios_base::in );
				// there we are, a regular istream is build upon the buffer :
				istream stream_pipe_in( &pipe_buf );
					
				while ( cont ) {
					//	This block is required to neatly force the closure of the
					//	piped.
					try {
						while ( rcep.unsafe_read_comp_exec_print( stream_pipe_in, std::cout ) ) {}
						cont = false;
					} catch ( Ginger::Mishap & e ) {
						e.report();
						cerr << endl << SYS_MSG_PREFIX << "Reset after runtime error ..." << endl;
						vm->resetMachine();
						break;
					} 			
				}
			} catch ( Ginger::CompileTimeError & e ) {
				e.report();
				cerr << endl << SYS_MSG_PREFIX << "Reset after compilation error ..." << endl;
				vm->resetMachine();
			}
		}
		#endif

	}

public:
	virtual int run() {
		if ( this->context.welcomeBanner() ) printWelcomeMessage();
		this->mainLoop();
		return EXIT_SUCCESS;
	}

public:
	GingerMain( const char * name ) : ToolMain( name ) {
		this->context.printDetailLevel() = 2;
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
