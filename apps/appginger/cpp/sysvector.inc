SysMap::value_type( "vectorIndex", SysInfo( fnc_syscall, Arity( 2 ), Arity( 1 ), sysVectorIndex, "Indexes a vector" ) ),
SysMap::value_type( "vectorExplode", SysInfo( fnc_syscall, Arity( 1 ), Arity( 0, true ), sysVectorExplode, "Explodes a vector into its members" ) ),
SysMap::value_type( "vectorLength", SysInfo( fnc_syscall, Arity( 1 ), Arity( 1 ), sysVectorLength, "Returns the length of a vector" ) ),	
SysMap::value_type( "vectorAppend", SysInfo( fnc_syscall, Arity( 2 ), Arity( 1 ), sysVectorAppend, "Appends two vectors" ) ),
