#include <iostream>

#include "rdlmnxsrc.hpp"

using namespace std;

int main( int argc, char **argv ) {     
	Ginger::ReadlineMnxSource rdl;
	rdl.setPrompt( "??? " );
	char ch;
	while ( rdl.get( ch ) ) {
		cout << "Got " << static_cast< int >( ch ) << endl;
	}
	
}
