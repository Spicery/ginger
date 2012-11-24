#include "sysconst.hpp"

#include <string>
#include <map>

#define SYSFN( x )    		this->table[ x ] = new SysConst( "sysfn", x )
#define SYSFN2( x, y )    	this->table[ x ] = new SysConst( "sysfn", y )
#define CONST( t, x )		this->table[ x ] = new SysConst( t, x )

class SysConstTable {
	typedef std::map< std::string, SysConst * > Table;

private:
	Table table;

public:
	SysConst * lookup( std::string & key ) {
		Table::iterator it = this->table.find( key );
		return it == this->table.end() ? NULL : it->second;
	}

public:
	SysConstTable() {
	
		//	Absent & Present
		CONST( "absent", "absent" );
		#ifdef PRESENT_CONSTANT_REQUIRED
			CONST( "present", "present" );
		#endif
		//	Booleans
		CONST( "bool", "true" );
		CONST( "bool", "false" );
		
		//	Misc.
		CONST( "undefined", "undefined" );
		CONST( "indeterminate", "indeterminate" );
		
		//	Functions (I think this is debatable).
		SYSFN( "not" );
		
	}
} theSysConstTable;

SysConst * lookupSysConst( std::string key ) {
	return theSysConstTable.lookup( key );
}

