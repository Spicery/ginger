#ifndef SYSCONST_HPP
#define SYSCONST_HPP

#include <string>

class SysConst {
public:
	const char * 	tag;
	const char *	value;
	
public:
	SysConst( const char * t, const  char * v ) :
		tag( t ),
		value( v )
	{
	}
};

SysConst * lookupSysConst( std::string key );

#endif
