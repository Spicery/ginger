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
#include <cstdlib>

#include <stddef.h>
#include <syslog.h>
#include <sys/errno.h>


#include <ext/stdio_filebuf.h> // __gnu_cxx::stdio_filebuf

#include "mnx.hpp"
#include "mishap.hpp"
#include "command.hpp"

#include "rcep.hpp"
#include "appcontext.hpp"
#include "toolmain.hpp"

#define APP_TITLE "ginger-script"

#include "wellknownpaths.hpp"

using namespace std;

class ScriptMain : public ToolMain {
public:
	int run() {		
		MachineClass * vm = this->context.newMachine();
		Package * interactive_pkg = this->context.initInteractivePackage( vm );
		RCEP rcep( interactive_pkg );

		this->executeLoadFileList( rcep );			
		this->executeFileArguments( rcep );
		if ( this->context.useStdin() ) {
			this->executeStdin( false, rcep );
		}
	    return EXIT_SUCCESS;
	}

public:
	ScriptMain( const char * name ) : ToolMain( name ) {
		this->context.initScript();
		this->context.printDetailLevel().setBasic();
	}
	
	virtual ~ScriptMain() {}
};

int main( int argc, char **argv, char **envp ) {
	openlog( APP_TITLE, 0, LOG_LOCAL2 );
	setlogmask( LOG_UPTO( LOG_INFO ) );
	try {
		ScriptMain main( APP_TITLE );
		return main.parseArgs( argc, argv, envp ) ? main.run() : EXIT_SUCCESS;
	} catch ( Ginger::Mishap & e ) {
		e.report();
		return EXIT_FAILURE;
	}
}
