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

#ifndef CAGE_CRAWL_HPP
#define CAGE_CRAWL_HPP

#include "cage.hpp"

namespace Ginger {

class CageCrawl {
private:
	CageClass * 			cage;
	Ref *					current;

private:
	static unsigned long lengthAfterKey( Ref * key );
	static Ref * findKey( Ref * );

public:
	//	Returns sys_cage_crawl_termin when exhausted.
	Ref * next();
	Ref * currentObjA() { return this->current; }
	
public:
	CageCrawl( CageClass * c ) : 
		cage( c ) 
	{
		this->current = c->start;
	}

	CageCrawl( CageClass * c, Ref * obj_A ) : 
		cage( c )
	{	
		this->current = obj_A;
	}

};

} // namespace Ginger

#endif
