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

		//	Lists (Pairs & Nils)
		this->table[ "newList" ] = new SysConst( "sysfn", "newList" );
		this->table[ "newPair" ] = new SysConst( "sysfn", "newPair" );
		this->table[ "isPair" ] = new SysConst( "sysfn", "isPair" );
		this->table[ "isNil" ] = new SysConst( "sysfn", "isNil" );
		this->table[ "head" ] = new SysConst( "sysfn", "head" );
		this->table[ "tail" ] = new SysConst( "sysfn", "tail" );
		
		//	Vectors
		this->table[ "newVector" ] = new SysConst( "sysfn", "newVector" );
		this->table[ "isVector" ] = new SysConst( "sysfn", "isVector" );
		
		
		this->table[ "garbageCollect" ] = new SysConst( "sysfn", "garbageCollect" );
		this->table[ "print" ] = new SysConst( "sysfn", "refPrint" );
		this->table[ "println" ] = new SysConst( "sysfn", "refPrintln" );
	}
} theSysConstTable;

SysConst * lookupSysConst( std::string key ) {
	return theSysConstTable.lookup( key );
}
