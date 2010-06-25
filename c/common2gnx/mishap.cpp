#include <iostream>
#include <vector>

#include "mishap.hpp"

using namespace std;

Mishap & Mishap::culprit( const std::string reason, const std::string arg ) {
	this->culprits.push_back( pair< const string, const string >( reason, arg ) );
	return *this;
}

Mishap & Mishap::culprit( const std::string arg ) {
	this->culprits.push_back( std::pair< const string, const string >( "Argument", arg ) );
	return *this;
}

void Mishap::report() {
	cerr << "MISHAP: " << this->message << endl;
	for ( 	
		vector< pair< string, string > >::iterator it = this->culprits.begin();
		it != this->culprits.end();
		++it
	) {
		cerr << it->first << " : " << it->second << endl;
	}
}
	
Mishap::Mishap( const std::string & msg ) : 
	message( msg ) 
{
}
