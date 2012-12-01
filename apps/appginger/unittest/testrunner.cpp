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
#include <string>

#include <stddef.h>


#include <cppunit/CompilerOutputter.h>
#include <cppunit/XmlOutputter.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

using namespace std;

/*
	testrunner [OPTIONS] [FILENAME]
	Option
		--xml		Output to XML
		--text		Output to text
		--stdout	Output to standard output, if no filename
		--stderr	Output to standard error, if no filename
*/
int main( int argc, char ** argv ) {
	CppUnit::TextUi::TestRunner runner;
	CppUnit::TestFactoryRegistry &registry = CppUnit::TestFactoryRegistry::getRegistry();
	runner.addTest( registry.makeTest() );
	
	
	bool use_xml = false;
	bool use_text = false;
	bool use_stdout = false;
	bool use_stderr = false;
	const char * fname = NULL;
	for ( int i = 1; i < argc; i++ ) {
		string arg( argv[ i ] );
		if ( arg == "--xml" ) {
			use_xml = true;
		} else if ( arg == "--text" ) {
			use_text = true;
		} else if ( arg == "--stdout" ) {
			use_stdout = true;
		} else if ( arg == "--stderr" ) {
			use_stderr = true;
		} else {
			fname = argv[ i ];
		}
	}
	use_xml = use_xml || !use_text;

	ofstream out;
	if ( fname != NULL ) {
		out.open( fname );
	}

	if ( !use_text ) {
		runner.setOutputter( 
			new CppUnit::XmlOutputter( 
				&runner.result(), 
				( fname != NULL ? out : use_stdout ? std::cout : std::cerr )
			) 
		);
	} 
		
	// Run the tests.
  	bool wasSucessful = runner.run( "", false, true, false );

	if ( out.is_open() ) {
		out.close();
	}
	
  	// Return error code 1 if the one of test failed.
  	return wasSucessful ? 0 : 1;
}
