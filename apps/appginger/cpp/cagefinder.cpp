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

#include "cagefinder.hpp"

namespace Ginger {

/**
 * 	In reality we want to keep the population of the cages-set as 
 *	low as possible. So we don't have a full population of cages.
 *	so that's why we check the resulting cage contains the pointer.
 * 
 * 	In addition, if we have a small population then it is quite
 *	likely to be 0 or 1. So that's why we specially check those.
 */
CageClass * CageFinder::find( Ref * obj_K ) {
	
	//	Small population means that we'll often have no cages in
	//	here, so exit quickly.
	if ( this->num_cages == 0 ) return NULL;

	//	Small population means it is worth having a cache.
	if ( this->last_cage ) {
		if ( this->last_cage->contains( obj_K ) ) {
			//	Cache got lucky.
			return this->last_cage;
		} else if ( this->num_cages == 1 ) {
			//	Cache missed - but maybe we only have one cage. In which 
			//	case we're done.
			return NULL;
		}
	} 

	//	We can only reach here if we have at least 2 cages and the
	//	previously successful cache cage failed. So we look for the 
	//	cage whose end is just past the object-pointer.

	iterator it = this->cages.upper_bound( obj_K );
	if ( it == this->cages.end() ) return NULL;

	//	So we have the only cage that it might be in. We check
	//	whether it really does contain it. We have to do that 
	//	because we don't necessarily have all the possible cages
	//	in our list - and the check is cheap.

	CageClass * c = it->second;
	if ( c->contains( obj_K ) ) {
		//	It was successful, so cache the result.
		this->last_cage = c;
		return c;
	}

	//	Unsuccessful.
	return NULL;
}

} // namespace Ginger
