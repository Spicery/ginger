How to Add New Keywords to common2gnx
=====================================

For every new keyword 	this means modifying
	
	a.	toktype.hpp - to add a new case to the enum tokType
	b.	toktype.hpp - to add the enum to new tok_type_name
	c.	item_map.cpp - where the attributes of the token are set
	
Only the last part (c) is tricky. You need to specify 

	* 	the external name of the token
	* 	the internal enum
	*	the 'role' of the token
	*	the precedence, if it can appear in a postfix context
		
Figuring out the role and precedence need detailed knowledge
of the parser. The role determines the generic treatment of
the token and just getting that right does most of the work
for you. The most useful roles are

	PatternRole 		PATTERN TOKEN <rest>, postfix to a pattern
	CloserRole			e.g. endif, endfor
	PrefixRole			TOKEN <rest>, starts an expression
	PostfixRole			EXPR TOKEN <rest>, postfix or infix
	BothfixRole			May be prefix, postfix or infix
	LiteralRole			TOKEN, e.g. true, absent, termin
	UnarySysRole		TOKEN EXPR, prefix sys call
	BinaryFormRole		EXPR TOKEN EXPR, Specifically infix
	BinarySysRole		EXPR TOKEN EXPR, infix sys call
	SuffixOpRole		EXPR TOKEN, suffix operator
