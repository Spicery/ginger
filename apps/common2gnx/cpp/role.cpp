#include "role.hpp"

Role AnonRole			= { 0, PrefixCode, NoSpecialCode };
Role BinarySysRole 		= { 2, PostfixCode, SysCode };
Role BinaryFormRole 	= { 2, PostfixCode, FormCode };
Role BothfixRole 		= { DONTKNOW, BothfixCode, NoSpecialCode };
Role CharRole			= { 0, PrefixCode, LiteralCode };
Role CharSeqRole		= { 0, PrefixCode, NoSpecialCode };
Role CloserRole			= { DONTKNOW, NofixCode, NoSpecialCode };
Role EofRole			= { DONTKNOW, NofixCode, EofCode };
Role LiteralRole		= { 0, PrefixCode, LiteralCode };
Role NoRole				= { DONTKNOW, NofixCode, NoSpecialCode };
Role PostfixRole    	= { DONTKNOW, PostfixCode, NoSpecialCode };
Role PrefixRole 		= { DONTKNOW, PrefixCode, NoSpecialCode };
Role SkipRole			= { 0, PrefixCode, NoSpecialCode };
Role StringRole			= { 0, PrefixCode, LiteralCode };
Role SuffixSysRole		= { 1, PostfixCode, SysCode };
Role SymbolRole			= { 0, PrefixCode, LiteralCode };
Role UnarySysRole		= { 1, PrefixCode, SysCode };
Role UnaryFormRole		= { 1, PrefixCode, FormCode };
