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

//	Local libs
#include "lnxitem.hpp"
#include "lnxreader.hpp"

namespace Ginger {
using namespace std;

/*LnxItem::LnxItem( LnxItem * item ) :
	reader( item->reader ),
	values( item->values )
{
}*/


void LnxItem::put( const std::string & key, const std::string & value ) {
	//cout << "Putting " << key << " = " << value << endl;
	this->propertyValue( key ) = value;
}

std::string & LnxItem::propertyValue( const std::string & key ) {
	int n = this->reader->propertyIndex( key );
	//cerr << "checking N = " << n << " of " << this->values.size() << endl;
	return this->values[ n ];
}

bool LnxItem::hasProperty( const std::string & key ) const {
	return this->reader->hasProperty( key );
}


std::string & LnxItem::propertyValue( int n ) {
	return this->values[ n ];
}

void LnxItem::resetToDefaults() {
	this->values = this->reader->propertyDefaults();
}

const std::string & LnxItem::propertyKey( int n ) {
	return this->reader->propertyKey( n );
}

void LnxItem::show() {
	cout << "SHOW size = " << this->values.size() << endl;
	for ( unsigned int i = 0; i < this->values.size(); i++ ) {
		cout << this->propertyKey( i ) << " -> " << this->values[ i ] << endl;
	}
}

} // namespace
