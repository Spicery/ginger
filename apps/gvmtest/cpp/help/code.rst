Command <code> INSTRUCTION* </code>
===================================

The code command allows precise control over the planting of virtual 
machine code in a function object. (Well, it is the first hesitant step
towards that lofty goal.)

	INSTRUCTION ::= <vmi name=NAME> ARG* </vmi>
	ARG ::= <constant type=TYPE value=VALUE/>

Instruction Names
-----------------
NAME must be one of the instructions named in the ${GINGER_DEV_SRC}/instruction_set
folder e.g. "add", "mul", "gt". To get a complete list of instructions run

	% gvmtest 
	<help topic="gvm.instructions"/>

or

	% ginger-info -v



TYPE="int"
----------

VALUE should be an optionally signed long int in denary.


TYPE="bool"
-----------

VALUE should be "true" or "false".

TYPE="absent" or "indeterminate" or "undefined"
-----------------------------------------------

VALUE should be the same as the TYPE.


TYPE="char"
-----------

VALUE should be a string of length 1.

TYPE="string" or "symbol"
-------------------------

VALUE may be any string.

TYPE="double"
-------------

VALUE may be a double in standard notation.


TYPE="sysfn"
------------

VALUE may be the name of any built-in function e.g. "println"


TYPE="sysclass"
---------------
	
VALUE may be any of
	* "Absent"
	* "Bool"
	* "Small"
	* "Double"
	* "String" 
	* "Char"
	* "Nil"
	* "Pair"
	* "Vector"
	* "Class"
