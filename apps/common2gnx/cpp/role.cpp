#include "role.hpp"

Role PatternRole		= { DONTKNOW, DONTKNOW, PostfixCode, PatternCode };
Role IncrByRole			= { 1, 1, PrefixCode, NoSpecialCode };
Role DecrByRole			= { 1, 1, PrefixCode, NoSpecialCode };
Role AnonRole			= { 0, 1, PrefixCode, NoSpecialCode };
Role BinarySysRole 		= { 2, 1, PostfixCode, SysCode };
Role BinaryFormRole 	= { 2, 1, PostfixCode, FormCode };
Role BothfixRole 		= { DONTKNOW, DONTKNOW, BothfixCode, NoSpecialCode };
Role CharRole			= { 0, 1, PrefixCode, LiteralCode };
Role CharSeqRole		= { 0, DONTKNOW, PrefixCode, NoSpecialCode };
Role CloserRole			= { DONTKNOW, DONTKNOW, NofixCode, NoSpecialCode };
Role EofRole			= { DONTKNOW, DONTKNOW, NofixCode, EofCode };
Role LiteralRole		= { 0, 1, PrefixCode, LiteralCode };
Role NoRole				= { DONTKNOW, DONTKNOW, NofixCode, NoSpecialCode };
Role PostfixRole    	= { DONTKNOW, DONTKNOW, PostfixCode, NoSpecialCode };
Role PrefixRole 		= { DONTKNOW, DONTKNOW, PrefixCode, NoSpecialCode };
Role SkipRole			= { 0, 0, PrefixCode, NoSpecialCode };
Role StringRole			= { 0, 1, PrefixCode, LiteralCode };
Role SuffixOpRole		= { 1, 1, PostfixCode, NoSpecialCode };
Role UnarySysRole		= { 1, 1, PrefixCode, SysCode };
