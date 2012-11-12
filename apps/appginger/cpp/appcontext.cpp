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


#define SIMPLIFYGNX		( INSTALL_TOOL "/simplifygnx" )
#define COMMON2GNX		( INSTALL_TOOL "/common2gnx" )
#define CSTYLE2GNX		( INSTALL_TOOL "/cstyle2gnx" )
#define LISP2GNX		( INSTALL_TOOL "/lisp2gnx" )
#define GSON2GNX		( INSTALL_TOOL "/gson2gnx" )
#define GNX2GNX			"/bin/cat" 


#include <iostream>
#include <fstream>

#include <cstdio>
#include <cstdlib>

#include <unistd.h>
#include <getopt.h>
#include <syslog.h>

#include "common.hpp"
#include "rcep.hpp"
#include "appcontext.hpp"
#include "mishap.hpp"
#include "sys.hpp"
#include "machine1.hpp"
#include "machine2.hpp"
#include "machine3.hpp"
#include "machine4.hpp"

using namespace std;

Package * AppContext::initInteractivePackage( MachineClass * vm ) {
    Package * interactive_pkg = vm->getPackage( this->getInteractivePackage() );
    return interactive_pkg;
}

MachineClass * AppContext::newMachine() {
   switch ( this->machine_impl_num ) {
        case 1: return new Machine1( *this );
        case 2: return new Machine2( *this );
        case 3: return new Machine3( *this );
        case 4: return new Machine4( *this );
        default: {
            cerr <<  "Invalid implementation (" << this->machine_impl_num << ") using implementation 1" << endl;
            return new Machine1( *this );
            break;
        }
    }
}

void AppContext::addProjectFolder( const std::string & folder ) {
	this->project_folder_list.push_back( folder );
}


void AppContext::addProjectFolder( const char * folder ) { 
	this->project_folder_list.push_back( folder );
}

void AppContext::initCgi() {
#ifdef RUDECGI
	this->cgi = new rude::CGI();
#endif
}

const char* AppContext::cgiValue( const char * fieldname ) {
    #ifdef RUDECGI
    	const char * answer = this->cgi == NULL ? "" : this->cgi->value( fieldname );
    	return answer;
    #else
    	return "";
    #endif
}

const char * AppContext::syntax() { 
	if ( this->initial_syntax == "gnx" ) {
		return GNX2GNX;
	} else if ( this->initial_syntax == "" || this->initial_syntax == "common" ) {
		return COMMON2GNX;
	} else if ( this->initial_syntax == "lisp" ) {
		return LISP2GNX;
	} else if ( ( this->initial_syntax == "cstyle" ) || ( this->initial_syntax == "javascript" ) ) {
		return CSTYLE2GNX;
	} else {
		throw Ginger::Mishap( "Unrecognised language" ).culprit( "Language", this->initial_syntax );
	}
}

const char * AppContext::syntax( const std::string & filename ) { 
	std::string::size_type idx = filename.rfind( '.' );
	if ( idx != std::string::npos ) {
		std::string extension = filename.substr( idx );
 		if ( extension == ".gnx" ) {
			return GNX2GNX;
		} else if ( extension == ".cmn" || extension == "common" ) {
			return COMMON2GNX;
		} else if ( extension == ".lisp" ) {
			return LISP2GNX;
		} else if ( extension == ".cstyle" ) {
			return CSTYLE2GNX;
		}
	}
	return this->syntax();
}
