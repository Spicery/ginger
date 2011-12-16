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

/** \mainpage AppContext Internal Documentation
*//*
 *
 * \section intro_sec Introduction
 *
 * This is the introduction.
 *
 * \section install_sec Installation
 *
 * \subsection step1 Step 1: Opening the box
*/

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
#include "term.hpp"
#include "mishap.hpp"
#include "sys.hpp"
#include "machine1.hpp"
#include "machine2.hpp"
#include "machine3.hpp"
#include "machine4.hpp"
//#include "database.hpp"

using namespace std;

Package * AppContext::initInteractivePackage( MachineClass * vm ) {
    Package * interactive_pkg = vm->getPackage( INTERACTIVE_PACKAGE );
    //Package * std_pkg = vm->getPackage( STANDARD_LIBRARY_PACKAGE );
    /*interactive_pkg->import( 
        Import(
            fetchFacetSet( "public" ),  		//  Import the public facet from ...
            std_pkg,                    		//  ... the standard library.
            std::string( STANDARD_LIBRARY_PACKAGE ),    //  Alias
            true,                       		//  Protected = nonmaskable.
            NULL                        		//  Not into - or should it be the empty FacetSet???
        )
    );*/
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
	//cout << "CGI " << fieldname << " = " << answer << endl;
	return answer;
#else
	return "";
#endif
}

string AppContext::syntax() { 
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
