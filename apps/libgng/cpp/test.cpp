#include <iostream>
#include <stdexcept>
#include <fstream>

#include "gson.hpp"
#include "mishap.hpp"

using namespace std;
using namespace Ginger;

int main( int argc, char ** argv ) {
	try {

		//ifstream input( "test.gson" );
		GSON p = GSON::readSettingsFile( "test.gson" );
		if ( not p ) {
			cout << "Awesome" << endl;
		} else {
			p->render();
			cout << endl;
		}

		cout << "Prompt is " << p.index( "prompt" ).getString() << endl;

		//cout << ">>> ";
		//p->get( 0 )->render();
		//cout << endl;
	} catch ( Mishap & m ) {
		m.report();
	}
	return 0;
}
