#ifndef CAGE_CRAWL_HPP
#define CAGE_CRAWL_HPP

#include "cage.hpp"

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

#endif