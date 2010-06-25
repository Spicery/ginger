#ifndef HEAP_HPP
#define HEAP_HPP

#include "cage.hpp"

class HeapClass {
private:
	CageClass *						current;
	std::vector< CageClass * >		zoo;
	MachineClass *					machine_ptr;
	
public:
	void garbageCollect();
	CageClass & preflight( int size );
	Ref copyString( const char *s );		//	Copy string, possibly causing GC


public:
	HeapClass( MachineClass * machine );
	~HeapClass();

};

#endif