Notes Towards the Design of the C/Java-Style
============================================

The '=' Operator
----------------

This is overloaded. There are two uses:

	1.	var PATTERN = EXPR
	2.	UPDATE_EXPR = EXPR

Note that (1) is a kind of Query but (2) is a type of expression.

The ';' Converts Expressions into Statements
--------------------------------------------

The only role of the semi-colon is to convert expressions into statements. 
As a consequence, the only infix or postfix operators are within 
expressions, never statements.

