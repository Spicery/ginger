SysMap::value_type( "isMutableVector", SysInfo( Arity( 1 ), Arity( 1 ), sysIsMutableVector, "Return true if the vector is mutable, else false" ) ),
SysMap::value_type( "vectorExplode", SysInfo( Arity( 1 ), Arity( 0, true ), sysVectorExplode, "Explodes a vector into its members" ) ),
SysMap::value_type( "vectorLength", SysInfo( Arity( 1 ), Arity( 1 ), sysVectorLength, "Returns the length of a vector" ) ),	
SysMap::value_type( "vectorAppend", SysInfo( Arity( 2 ), Arity( 1 ), sysVectorAppend, "Appends two vectors" ) ),
