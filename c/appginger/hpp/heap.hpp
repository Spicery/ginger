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

#ifndef HEAP_HPP
#define HEAP_HPP

#include "cage.hpp"

class HeapClass {
friend class HeapCrawl;
private:
	CageClass *						current;
	std::vector< CageClass * >		zoo;
	MachineClass *					machine_ptr;
	
public:
	bool wouldGC( int size );
	CageClass * preflight( Ref * & pc, int size );
	CageClass * preflight( int size );
	Ref copyString( const char *s );		//	Copy string, possibly causing GC

public:
	void selectCurrent();
	CageClass * newCageClass();

public:
	HeapClass( MachineClass * machine );
	~HeapClass();

};

#endif
