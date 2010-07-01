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
	CageClass & 	cage;

public:
	bool checkRoom( int );
	void xfrRef( Ref x );
	void xfrVector( std::vector< Ref > & chain );
	void xfrSubstring( const char *s, int a, int b );

	void setOrigin();      						//  set the object origin
	Ref make();              					//  return valid ptr

public:
	XfrClass( CageClass & c );
	XfrClass( MachineClass & machine, int size );
	XfrClass( HeapClass & machine, int size );
};

class CageClass {
private:
	Ref				*	start;
    Ref         	*	top;
    Ref         	*	end;
    
public:
	bool checkRoom( int );
	bool checkRoomCopy( int );
	
public:
	void beginCopy( int k );    			//  ensure headroom of k (use gc)
	void abandonCopy();         			//  explicit abandonment
	void xfrRef( Ref x );      				//  copy x to tmp-end-of-cage
                                            //  copy & remove items
	void xfrVector( std::vector< Ref > & chain );
	void xfrSubstring( const char *s, int a, int b );

	void setCopyOrigin();      				//  set the object origin
	Ref endCopy();              			//  return valid ptr
	
public:
	XfrClass & newXfr();

public:
	friend class XfrClass;
	CageClass( int capacity );
	~CageClass();
};

typedef class MachineClass * Machine;
typedef class CageClass *Cage;

#endif
