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
#include <fstream>

#include <cppunit/CompilerOutputter.h>
#include <cppunit/XmlOutputter.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

using namespace std;

int main( int argc, char ** argv ) {
	CppUnit::TextUi::TestRunner runner;
	CppUnit::TestFactoryRegistry &registry = CppUnit::TestFactoryRegistry::getRegistry();
	runner.addTest( registry.makeTest() );

	ofstream out;
	if ( argc == 2 ) {
		out.open( argv[ 1 ] );
	}
	
	runner.setOutputter( 
		new CppUnit::XmlOutputter( 
			&runner.result(), 
			( out.is_open() ? out : std::cerr )
		) 
	);
		
	// Run the tests.
  	bool wasSucessful = runner.run();

	if ( out.is_open() ) {
		out.close();
	}
	
  	// Return error code 1 if the one of test failed.
  	return wasSucessful ? 0 : 1;
}
