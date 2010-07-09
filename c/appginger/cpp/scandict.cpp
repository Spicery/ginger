#include "scandict.hpp"

Ref * ScanDict::next() {
	if ( it == dict->table.end() ) return static_cast< Ref * >( 0 );
	Ident id = (*it).second;
	++it;
	return &id->valof;
}

ScanDict::ScanDict( DictClass * d ) : 
	dict( d ),
	it( d->table.begin() )
{
}
