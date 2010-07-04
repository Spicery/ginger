#include "sysconst.hpp"

#include <string>
#include <map>


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
		this->table[ "absent" ] = new SysConst( "absent", "absent" );
		this->table[ "present" ] = new SysConst( "present", "present" );

		//	Booleans
		this->table[ "true" ] = new SysConst( "bool", "true" );
		this->table[ "false" ] = new SysConst( "bool", "false" );		
		this->table[ "not" ] = new SysConst( "sysfn", "not" );
		
		//	Misc
		this->table[ "garbageCollect" ] = new SysConst( "sysfn", "garbageCollect" );
		this->table[ "print" ] = new SysConst( "sysfn", "refPrint" );
		this->table[ "println" ] = new SysConst( "sysfn", "refPrintln" );
		
		//	Recordclasses & Vectorclasses
		#include "sysconsts.inc.auto"
	}
} theSysConstTable;

SysConst * lookupSysConst( std::string key ) {
	return theSysConstTable.lookup( key );
}
