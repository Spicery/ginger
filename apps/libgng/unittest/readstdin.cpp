#include <iostream>
#include <memory>

#include "mnx.hpp"

using namespace std;

int main( int argc, char **argv ) {     
	Ginger::MnxReader r( cin );
	for (;;) {
		// char ch;
		// for ( int i = 0; i < 10; i++ ) {
		// 	cin.get( ch );
		// 	cin.unget();
		// 	cout << cin.good() << endl;
		// }
		shared_ptr< Ginger::Mnx > mnx = r.readMnx();
		if ( not mnx ) break;
		mnx->render( cout );
		cout << endl;
	}
	
}
