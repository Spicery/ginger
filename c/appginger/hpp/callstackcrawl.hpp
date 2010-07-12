#ifndef CALL_STACK_CRAWL_HPP
#define CALL_STACK_CRAWL_HPP

#include "common.hpp"
#include "callstacklayout.hpp"
#include "machine.hpp"

class CallStackCrawl {
private:
	Ref *			sp;
	Ref *			sp_base;
	
public:
	Ref * next();

public:
	CallStackCrawl( MachineClass * m ) : 
		sp( m->sp ),
		sp_base( m->sp_base )
	{
	}
};

#endif