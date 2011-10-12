SysMap::value_type( "stringIndex", SysInfo( fnc_syscall, Arity( 2 ), Arity( 1 ), sysStringIndex, "Indexes a string" ) ),
SysMap::value_type( "stringExplode", SysInfo( fnc_syscall, Arity( 1 ), Arity( 0, true ), sysStringExplode, "Explodes a string into its members" ) ),
SysMap::value_type( "stringLength", SysInfo( fnc_syscall, Arity( 1 ), Arity( 1 ), sysStringLength, "Returns the length of a string" ) ),	
SysMap::value_type( "stringAppend", SysInfo( fnc_syscall, Arity( 2 ), Arity( 1 ), sysStringAppend, "Appends two strings" ) ),
