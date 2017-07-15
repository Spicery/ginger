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

#include "scanpkg.hpp"

namespace Ginger {


Ref * ScanPkg::next() {
	if ( it == pkg->table.end() ) return static_cast< Ref * >( 0 );
	this->var = (*it).first;	//	debug
	Valof * id = (*it).second;
	++it;
	return &id->valof;
}

ScanPkg::ScanPkg( Package * d ) : 
	pkg( d ),
	it( d->table.begin() )
{
}

} // namespace Ginger
