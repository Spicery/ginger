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
	CageClass & preflight( int size );
	Ref copyString( const char *s );		//	Copy string, possibly causing GC
	void collectGarbage();

public:
	HeapClass( MachineClass * machine );
	~HeapClass();

};

#endif