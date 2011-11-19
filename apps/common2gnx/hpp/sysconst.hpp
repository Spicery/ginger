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

extern SysConst * lookupSysConst( std::string key );
//extern bool lookupSysConst( std::string key, const char ** tag, const char ** value );

#endif
