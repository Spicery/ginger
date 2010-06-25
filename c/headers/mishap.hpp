#ifndef MISHAP_HPP
#define MISHAP_HPP

#include <string>
#include <vector>
#include <utility>
#include <stdexcept>


//#include <setjmp.h>
#include "common.hpp"

void this_never_happens( void );
void to_be_done( charseq msg );
void mishap( charseq msg, ... );
//void reset( charseq msg, ... );
void warning( charseq msg, ... );

//extern jmp_buf mishap_jump_buffer;

class Mishap {
private:
	std::string message;
	std::vector< std::pair< std::string, std::string > > culprits;

public:
	Mishap & culprit( const std::string reason, const std::string arg );
	Mishap & culprit( const std::string arg );
	void report();
	
public:
	Mishap( const std::string & msg );
};

class ToBeDone : public Mishap {
public:
	ToBeDone() : 
		Mishap( "To be done" ) 
	{
	}
};

class Unreachable : public Mishap {
public:
	Unreachable() : 
		Mishap( "Unreachable" ) 
	{
	}
};

class NormalExit {
};


#endif
