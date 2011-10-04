//	To be included into sys.cpp.
// 	Each line should be terminated with a comma.
SysMap::value_type( "getEnv", SysInfo( fnc_syscall, Arity( 1 ), Arity( 1 ), sysGetEnv, "Returns the value of an environment variable" ) ),
SysMap::value_type( "putEnv", SysInfo( fnc_syscall, Arity( 2 ), Arity( 0 ), sysPutEnv, "Sets the value of an environment variable" ) ),
SysMap::value_type( "envVars", SysInfo( fnc_syscall, Arity( 0 ), Arity( 1 ), sysEnvVars, "Returns the environment list" ) ),

