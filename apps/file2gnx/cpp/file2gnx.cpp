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
#include <fstream>

#include <syslog.h>
#include <unistd.h>
#include <stddef.h>

#include "mnx.hpp"
#include "sax.hpp"
#include "mishap.hpp"

using namespace std;
using namespace Ginger;

#define APP_TITLE		"file2gnx"
#define LOG_FACILITY	LOG_LOCAL2

#define COMMON2GNX		( INSTALL_TOOL "/common2gnx" )
#define CSTYLE2GNX		( INSTALL_TOOL "/cstyle2gnx" )
#define LISP2GNX 		( INSTALL_TOOL "/lisp2gnx" )
#define GNX2GNX     	( "/bin/cat" )

//	This will need to be significantly expanded when we adopt XDG
//	standards properly.
#define FILE2GNX_CONFIG 	"/etc/xdg/ginger/parser-mapping.mnx" 

//
//	Insecure. We need to do this more neatly. It would be best if common2gnx
//	and lisp2gnx could handle being passed a filename as an argument. This
//	would be both more secure and efficient.
//
void run( string command, string pathname ) {
	const char * cmd = command.c_str();
	execl( cmd, cmd, pathname.c_str(), NULL );
}

#define PARSER "parser"
#define PARSER_EXT "ext"
#define PARSER_EXE "exe"

class ExtnLookup : public Ginger::SaxHandler {
private:
	string pathname;
	string extn;
	bool found;
	string parser;

public:
	ExtnLookup( const string & pathname, const string & extn ) :
		pathname( pathname ),
		extn( extn ),
		found( false )
	{}

public:
	typedef map< string, string > Dict;
	
	void startTag( string & name, Dict & attrs ) {
		if ( name != PARSER ) return;
		Dict::iterator it = attrs.find( PARSER_EXT );
		Dict::iterator jt = attrs.find( PARSER_EXE );
		if ( it != attrs.end() && extn == it->second && jt != attrs.end() ) {
			this->found = true;
			this->parser = jt->second;
		} 
	}

	void endTag( std::string & name ) {
	}

public:

	string getParser() {
		return this->parser;
	}

	bool lookup( const char * mnx_file_name ) {
		ifstream stream( mnx_file_name );
		if ( stream.good() ) {
			Ginger::SaxParser saxp( stream, *this );
			saxp.readElement();
			return this->found;
		} else {
			return false;
		}
	}

	const char * defaultCommand() {
		if ( extn == "cmn" ) {
			return COMMON2GNX;
		} else if ( extn == "cst" ) {
			return CSTYLE2GNX;
		} else if ( extn == "lsp" ) {
			return LISP2GNX;
		} else if ( extn == "gnx" ) {
			return GNX2GNX;
		} else {
			throw Mishap( "Filename has unrecognised extension, giving up" ).culprit( "Filename", pathname ).culprit( "Extension", extn );
		}	
	}

};

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

		ExtnLookup elookup( pathname, extn );
		const char * cmdpath = (
			elookup.lookup( FILE2GNX_CONFIG ) ?
			elookup.getParser().c_str() :
			elookup.defaultCommand()
		);
		
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
