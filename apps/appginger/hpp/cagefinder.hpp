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

#ifndef CAGE_FINDER_HPP
#define CAGE_FINDER_HPP

#include <map>
#include <cassert>

#include "common.hpp"

#include "cage.hpp"

namespace Ginger {

class CageFinder {
	typedef std::map< Ref *, CageClass * >::iterator iterator;
private:
	int num_cages;	///	Only used for optimisation.
	CageClass * last_cage;
	std::map< Ref *, CageClass * > cages;
public:
	CageFinder() : num_cages( 0 ), last_cage( NULL ) {}

public:
	void add( CageClass * cage ) {
		assert( cage != NULL );
		num_cages += 1;
		this->cages[ cage->endRefPtr() ] = cage;
	}

	CageClass * find( Ref * obj_K );
};

} // namespace Ginger

#endif
