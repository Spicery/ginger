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
#include <sstream>
#include <string>


#include <stddef.h>
#include <syslog.h>

#include <ext/stdio_filebuf.h> // __gnu_cxx::stdio_filebuf


#include "mishap.hpp"

#include "appcontext.hpp"
#include "toolmain.hpp"
#include "rcep.hpp"
#include "wellknownpaths.hpp"

namespace Ginger {
using namespace std;

static void printWelcomeMessage() {
	cout << PACKAGE_NAME << ": " << PACKAGE_VERSION << ", Copyright (c) 2010  Stephen Leach" << endl;
	cout << "  +----------------------------------------------------------------------+" << endl;
	cout << "  | This program comes with ABSOLUTELY NO WARRANTY. It is free software, |" << endl;
	cout << "  | and you are welcome to redistribute it under certain conditions.     |" << endl;
	cout << "  | Use option --help=license for details.                               |" << endl;
	cout << "  +----------------------------------------------------------------------+" << endl;
}

class Main : public ToolMain {
private:
	void mainLoop() {
		MachineClass * vm = this->context.newMachine();
		Package * interactive_pkg = this->context.initInteractivePackage( vm );
	 
		#ifdef DBG_APPCONTEXT
			clog << "RCEP ..." << endl;
		#endif
	
		RCEP rcep( interactive_pkg );
		this->executeStdin( false, rcep );
	}

public:
	virtual int run() {
		printWelcomeMessage();
		this->mainLoop();
		return EXIT_SUCCESS;
	}

public:
	Main( const char * name ) : ToolMain( name ) {
		this->context.printDetailLevel().setChatty();
	}
	
	virtual ~Main() {}
};

} // namespace Ginger

int main( int argc, char **argv, char **envp ) {
	openlog( APPGINGER_NAME, 0, LOG_LOCAL2 );
	setlogmask( LOG_UPTO( LOG_INFO ) );
	try {
		Ginger::Main main( APPGINGER_NAME );
		return main.parseArgs( argc, argv, envp ) ? main.run() : EXIT_SUCCESS;
	} catch ( Ginger::Mishap & e ) {
		e.report();
		return EXIT_FAILURE;
	}
}
