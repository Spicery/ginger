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
	CageClass * preflight( int size );
	CageClass * preflight( Ref * & pc, int size );
	Ref copyString( const char *s );		//	Copy string, possibly causing GC

public:
	void selectCurrent();
	CageClass * newCageClass();

public:
	HeapClass( MachineClass * machine );
	~HeapClass();

};

#endif