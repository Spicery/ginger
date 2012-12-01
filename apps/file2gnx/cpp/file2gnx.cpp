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

#include <syslog.h>
#include <unistd.h>
#include <stddef.h>

#include "mishap.hpp"

using namespace std;
using namespace Ginger;

#define APP_TITLE		"file2gnx"
#define LOG_FACILITY	LOG_LOCAL2

#define COMMON2GNX		( INSTALL_TOOL "/common2gnx" )
#define LISP2GNX 		( INSTALL_TOOL "/lisp2gnx" )
#define GNX2GNX     	( "/bin/cat" )

//
//	Insecure. We need to do this more neatly. It would be best if common2gnx
//	and lisp2gnx could handle being passed a filename as an argument. This
//	would be both more secure and efficient.
//
void run( string command, string pathname ) {
	const char * cmd = command.c_str();
	execl( cmd, cmd, pathname.c_str(), NULL );
}

int main( int argc, char ** argv ) {
	openlog( APP_TITLE, 0, LOG_FACILITY );
	setlogmask( LOG_UPTO( LOG_INFO ) );

	if ( argc != 2 ) {
		cerr << "Usage: " << APP_TITLE << " <pathname>" << endl;
		return EXIT_FAILURE;
	}
	
	try {
		
		const string pathname( argv[ 1 ] );
		
		const size_t n = pathname.rfind( '.' );
		if ( n == string::npos ) {
			throw Mishap( "Filename lacks extension, giving up" ).culprit( "Filename", pathname );
		} 
			
		//cout << "dot yes" << endl;
		string extn( pathname.substr( n + 1, pathname.size() ) );
		//cout << "extension is " << extn << endl;
		
		const char * cmdpath = NULL;
		if ( extn == "cmn" ) {
			cmdpath = COMMON2GNX;
		} else if ( extn == "lsp" ) {
			cmdpath = LISP2GNX;
		} else if ( extn == "gnx" ) {
			cmdpath = GNX2GNX;
		} else {
			throw Mishap( "Filename has unrecognised extension, giving up" ).culprit( "Filename", pathname ).culprit( "Extension", extn );
		}
		syslog( LOG_INFO, "Converting %s with extension %s using %s", pathname.c_str(), extn.c_str(), cmdpath );
		run( cmdpath, pathname );
		
		return EXIT_SUCCESS;
	} catch ( Ginger::Mishap & m ) {
		m.culprit( "Detected by", APP_TITLE );
		m.gnxReport();
		return EXIT_FAILURE;
	} catch ( Ginger::Problem & m ) {
		m.culprit( "Detected by", APP_TITLE );
		m.gnxReport();
		return EXIT_FAILURE;
	}
}
