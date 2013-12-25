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
#include <stddef.h>

#include <rude/cgi.h>

#include "mnx.hpp"

#include "rcep.hpp"
#include "sysprint.hpp"


#include <syslog.h>

#include "mishap.hpp"

#include "appcontext.hpp"
#include "toolmain.hpp"

#define APP_TITLE "ginger-cgi"
#include "wellknownpaths.hpp"

//#define DBG_CGI_MAIN

namespace Ginger {
using namespace std;

class CgiMain : public ToolMain {
public:
	int run() {
		this->context.initCgi();
		MachineClass * vm = this->context.newMachine();
		Package * interactive_pkg = this->context.initInteractivePackage( vm );
		RCEP rcep( interactive_pkg );

		rcep.isPrinting() = false;		//	Turn off printing.
		this->executeLoadFileList( rcep );
		this->executeFileArguments( rcep );
	    
	    return EXIT_SUCCESS;
	}

public:
	CgiMain( const char * name ) : ToolMain( name ) {
	}
	
	virtual ~CgiMain() {}
};

} // namespace Ginger

int main( int argc, char **argv, char **envp ) {
	openlog( APP_TITLE, 0, LOG_LOCAL2 );
	setlogmask( LOG_UPTO( LOG_INFO ) );
	try {
		Ginger::CgiMain main( APPGINGER_NAME );
		return main.parseArgs( argc, argv, envp ) ? main.run() : EXIT_SUCCESS;
	} catch ( Ginger::Mishap & e ) {
		e.report();
		return EXIT_FAILURE;
	}
}
