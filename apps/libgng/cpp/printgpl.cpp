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

//	STL headers.
#include <iostream>
#include <fstream>

//	STL-ised Old Headers
#include <cstdlib>

#include <stddef.h>

//	Headers specific to the libgng.
#include "printgpl.hpp"

namespace Ginger {
using namespace std;

void printGPL( const char * start, const char * end ) {
	bool printing = false;
	ifstream license( INSTALL_LIB "/LICENSE.TXT" );
	string line;
	while ( getline( license, line ) )  {
		if ( !printing && ( start == NULL || line.find( start ) != string::npos ) ) {
			printing = true;
		} else if ( printing && end != NULL && line.find( end ) != string::npos ) {
			printing = false;
		}
		if ( printing ) {
			std::cout << line << std::endl;
		}
	}
}

void printGPLWarranty() {
	printGPL( "Disclaimer of Warranty.", "Limitation of Liability." );                 
}

void printGPLConditions() {
	printGPL( "TERMS AND CONDITIONS", "END OF TERMS AND CONDITIONS" );
}

void printGPL() {
	printGPL( NULL, NULL );
}

int optionPrintGPL( const char * optarg ) {
	if ( optarg == NULL || string( optarg ) == string( "all" ) ) {
		printGPL();
	} else if ( string( optarg ) == string( "warranty" ) ) {
		printGPLWarranty();                 
	} else if ( string( optarg ) == string( "conditions" ) ) {
		printGPLConditions();
	} else {
		std::cerr << "Unknown license option: " << optarg << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

} // namespace
