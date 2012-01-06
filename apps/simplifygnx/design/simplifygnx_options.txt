Command Line options for SimplifyGNX
====================================

Apart from the standard command line options, simplifygnx offers some fine 
grained control over what it does.

*-s, --standard*:: 
	A standard set of transformations, equal to options 0-9. 

*-0, --self*::
	Replace named lambda self-references with special forms. This is
	an important analysis that makes named lambdas possible.
	
*-1, --absolute*::
	Add def.pkg attribute to all globals.

*-2, --arity*::
	Add arity marking to all subexpressions. This is marked with the
	annotation 'arity' and the syntax is an option leading '+' followed
	by a small integer (radix 10).

*-3, --flatten*::
	Eliminates nested sequences, which makes other pattern recognition
	easier. This also includes restructuring lists to eliminate appends.
	This is an important transformation.

*-5, --lift*::
	Transform nested lambdas so that they only reference immediate locals.
	This option entails scope analysis (--scope) and self-reference analysis
	(--self).

*-6, --scope*::
	Mark all variable definitions and references as local or global.

*-7, --sysapp*::
	Replace calls to standard variable with sys-calls.

*-8, --tailcall*::
	Marks function applications as to whether or not they are in a 
	tail-call position.

*-A, --slotalloc*::
	Assigns a frame slot to every inner local variable. This means that a
	back-end can actually completely ignore inner variables. This entails
	lifting analysis (--lift).

*-B, --toplevel*::
	Moves every lambda to top-level. This entails lift transformation (--lift).
