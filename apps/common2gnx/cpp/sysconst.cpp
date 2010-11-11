#include "sysconst.hpp"

#include <string>
#include <map>

#define SYSFN( x )    this->table[ x ] = new SysConst( "sysfn", x )
#define SYSFN2( x, y )    this->table[ x ] = new SysConst( "sysfn", y )

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
		SYSFN( "garbageCollect" );
		SYSFN2( "print", "refPrint" );
		SYSFN2( "println", "refPrintln" );

		SYSFN( "explode" );
		SYSFN( "length" );
		SYSFN( "append" );
		
		SYSFN( "listExplode" );
		SYSFN( "listLength" );
		SYSFN( "listAppend" );
		
		SYSFN( "vectorExplode" );
		SYSFN( "vectorLength" );
		SYSFN( "vectorAppend" );
		
		SYSFN( "stringExplode" );
		SYSFN( "stringLength" );
		SYSFN( "stringAppend" );
		
		//	Recordclasses & Vectorclasses
		#include "sysconsts.inc.auto"
	}
} theSysConstTable;

SysConst * lookupSysConst( std::string key ) {
	return theSysConstTable.lookup( key );
}

