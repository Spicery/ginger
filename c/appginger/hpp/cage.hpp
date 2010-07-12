#ifndef CAGE_HPP
#define CAGE_HPP

#include <vector>

#include "common.hpp"

class MachineClass;
class HeapClass;
class CageClass;

class XfrClass {
private:
    Ref *			tmptop;
	Ref	*			origin;
	CageClass * 	cage;

public:
	bool checkRoom( int );
	void xfrRef( Ref x );
	void xfrVector( std::vector< Ref > & chain );
	void xfrSubstring( const char *s, int a, int b );
	void xfrCopy( Ref *p, int n );
	void xfrCopy( Ref * obj_A, Ref * obj_Z1 );

	void setOrigin();      						//  set the object origin
	Ref make();              					//  return valid ptr
	

public:
	XfrClass( CageClass * c );
	XfrClass( Ref * & pc, MachineClass & machine, int size );
	XfrClass( Ref * & pc, HeapClass & machine, int size );
};

class CageClass {
friend class CageCrawl;
friend class ToSpaceCopier;
private:
	Ref				*	start;
	Ref				*	queue_base;
    Ref         	*	top;
    Ref         	*	end;
    
public:
	bool checkRoom( int );
	//bool checkRoomCopy( int );
	
public:	
	long nboxesInUse() { return this->top - this->start; }
	void reset();
	void resetQueue();
	bool isEmpty();
	bool hasEmptyQueue();
	bool hasntEmptyQueue();
	
public:
	friend class XfrClass;
	CageClass( int capacity );
	CageClass();
	~CageClass();
};

typedef class MachineClass * Machine;
typedef class CageClass *Cage;

#endif
