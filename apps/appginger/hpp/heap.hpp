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

#include <map>

#include "cage.hpp"
#include "bigint.hpp"
#include "rational.hpp"


namespace Ginger {

class HeapClass {
friend class HeapCrawl;
friend class CageFinder;
private:
	CageClass *						current;
	std::vector< CageClass * >		zoo;
	MachineClass *					machine_ptr;
	
public:
	bool wouldGC( int size );
	CageClass * preflight( Ref * & pc, int size );
	CageClass * preflight( int size );
	Ref copyString( const char *s );				//	Copy string, vetoing GC
	Ref copyString( Ref * & pc, const char *s );	//	Copy string, possibly causing GC
	Ref copyDouble( gngdouble_t d );				//	Copy double, vetoing GC
	Ref copyDouble( Ref * & pc, gngdouble_t d );	//	Copy double, possibly causing GC
	Ref copyBigInt( const char * s );				//	Copy big int, vetoing GC
	Ref copyBigInt( Ref * & pc, const char * s );	//	Copy big int, possibly causing GC
	Ref copyBigIntExternal( Ref * & pc, const BigIntExternal & e );			//	Copy big int, possibly causing GC
	Ref copyRationalExternal( Ref * & pc, const RationalExternal & e );		//	Copy big int, possibly causing GC

public:
	void selectCurrent();
	CageClass * newCageClass();

public:
	typedef std::vector< CageClass * >::iterator cage_iterator;
	cage_iterator begin() { return this->zoo.begin(); }
	cage_iterator end() { return this->zoo.end(); }

public:
	HeapClass( MachineClass * machine );
	~HeapClass();

};

} // namespace Ginger

#endif
