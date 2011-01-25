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
        default: {
            cerr <<  "Invalid implementation (" << this->machine_impl_num << ") using implementation 1" << endl;
            return new Machine1( *this );
            break;
        }
    }
}

void AppContext::addProjectFolder( std::string & folder ) {
	this->project_folder_list.push_back( folder );
}


void AppContext::addProjectFolder( const char * folder ) { 
	this->project_folder_list.push_back( folder );
}
