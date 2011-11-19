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

#include <string>
#include <map>
#include "metainfo.hpp"
#include "arity.hpp"

namespace Ginger {
using namespace std;

struct MetaInfo {
	Arity in_arity;
	Arity out_arity;
	
	MetaInfo() {
	}
	
	MetaInfo( Arity in, Arity out ) : 
		in_arity( in ),
		out_arity( out )
	{	
	}
};

typedef std::map< std::string, MetaInfo >::value_type MAPLET;

const std::map< std::string, MetaInfo >::value_type rawData[] = {
#include "../../automatic/metainfo/metainfo.cpp.inc"
//MAPLET( "*", MetaInfo( Arity( 2, false ), Arity( 1, false ) ) ),
};
const int numElems = sizeof rawData / sizeof rawData[0];
std::map< std::string, MetaInfo > metaInfo( rawData, rawData + numElems );

Arity inArity( const string & name ) {
	return metaInfo[ name ].in_arity;
}

Arity outArity( const string & name ) {
	return metaInfo[ name ].out_arity;
}

} // namespace
