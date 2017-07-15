/******************************************************************************\
	Copyright (c) 2010 Stephen Leach. AppGinger is distributed under the terms 
	of the GNU General Public License. This file is part of Ginger.

    Ginger is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Ginger is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Ginger.  If not, see <http://www.gnu.org/licenses/>.
\******************************************************************************/

//	Local libs
#include "state.hpp"
#include "mapping.hpp"

namespace SRC2LNX_NS {
//using namespace Ginger;
using namespace std;

void Mapping::put( const std::string & key, const std::string & value ) {
	this->mapping[ key ] = value;
}

void Mapping::update( State * state ) {
	map< string, string >::iterator it = this->mapping.find( state->propValue( this->src ) );
	if ( it != this->mapping.end() ) {
		state->propValue( this->dst ) = it->second;
	}
}

} // namespace
