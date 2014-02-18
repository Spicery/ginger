#include <iostream>
#include <string>
#include <uuid/uuid.h>

using namespace std;

int main() {
	uuid_t x;
	uuid_string_t s;
	for ( int i = 0; i < 10000000; i++ ) {
		uuid_generate( x );
	}
	uuid_unparse( x, s );
	string str( s );
	cout << s << endl;
}
