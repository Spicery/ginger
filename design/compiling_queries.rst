How to Compile Queries
======================

Setup for Loops, Conditionals and Naked Queries
-----------------------------------------------
Each query has these parts: INIT, TEST, NEXT, BODY, ADVN, FIN. A loop for 
query Q with body B is therefore compiled like this:

		INIT[ Q ]
		goto test

	label body:
		NEXT[ Q ]	# advance before body is executed. Populates unknowns.
		BODY[ Q ]
		ADVN[ Q ]	# advance after body is executed. Sets up state.
	label test:
		# TEST[ Q ] takes a pair of (pass,fail) labels. On success it 
		# continues at 'pass', on failure it continues at 'fail'.
		TEST[ Q ]( body, done )
	label done:
		FINI[ Q ]

Conditionals have three parts IFQ, THEN, ELSE and are compiled like this:

	pass := newLabel()
	fail := newLabel()
	done := newLabel()

	INIT[ IFQ ]
	TEST[ IFQ ]( pass, fail )
	label pass: THEN; goto done
	label fail: ELSE
	label done:

Naked queries are even simpler

	pass := newLabel()
	fail := newLabel()
	INIT[ IFQ ]
	TEST[ IFQ ]( pass, fail )
	label fail:
	throw 
	label pass:


Parallel Loops P // Q
---------------------

	INIT[ P // Q ] --> INIT[ P ]; INIT[ Q ]

	TEST[ P // Q ]( pass, fail ) -->
		ok := newLabel()
		done := newLabel()
		TEST[ P ]( ok, fail )
		label ok:
		TEST[ Q ]( pass, done )
		label done:

	NEXT[ P // Q ] --> NEXT[ P ]; NEXT[ Q ]
	ADVN[ P // Q ] --> ADVN[ P ]; ADVN[ Q ]
	BODY[ P // Q ] --> BODY[ P ]; BODY[ Q ]
	FINI[ P // Q ] --> FINI[ P ]; FINI[ Q ]

Where Clause Q where C
----------------------

	INIT[ Q where C ] --> INIT[ Q ]

	TEST[ Q where C ]( pass, fail ) --> 
		start := newLabel()
		ok := newLabel()
		label start:
		TEST[ Q ]( ok, fail )
		label ok:
		if C then goto pass
		NEXT[ Q ]
		ADV[ Q ]
		goto start
		
	NEXT[ Q where C ] --> NEXT[ Q ]
	ADVN[ Q where C ] --> ADVN[ Q ]
	BODY[ Q where C ] --> BODY[ Q ]
	FINI[ Q where C ] --> FINI[ Q ]

Q do S
------

	INIT[ Q do S ] --> INIT[ Q ]
	TEST[ Q do S ] --> TEST[ Q ]
	NEXT[ Q do S ] --> NEXT[ Q ]
	ADVN[ Q do S ] --> ADV[ Q ]
	BODY[ Q do S ] --> BODY[ Q ]; S 
	FINI[ Q do S ] --> FINI[ Q ]

Q finally S
-----------

	INIT[ Q finally S ] --> INIT[ Q ]
	TEST[ Q finally S ] --> TEST[ Q ]
	NEXT[ Q finally S ] --> NEXT[ Q ]
	ADVN[ Q finally S ] --> ADV[ Q ]
	BODY[ Q finally S ] --> BODY[ Q ]
	FINI[ Q finally S ] --> FINI[ Q ]; S

Q while C then S else T
-----------------------

	INIT[ Q while C then S ] --> INIT[ Q ]

	TEST[ Q while C then S ]( pass, fail ) --> 
		done := newLabel()
		y := newLabel()
		n := newLabel
		TEST[ Q ]( y, n )
		label y:
		if C then goto pass
		S
		goto done
		label n:
		FINI[ Q ]
		label done:

	
	NEXT[ Q while C then S else T ] --> NEXT[ Q ]
	ADVN[ Q while C then S else T ] --> ADV[ Q ]
	BODY[ Q while C then S else T ] --> BODY[ Q ]
	
	FINI[ Q while C then S else T ] --> ()

Q until C then S else T
-----------------------

	INIT[ Q until C then S ] --> INIT[ Q ]

	TEST[ Q until C then S ]( pass, fail ) --> 
		done := newLabel()
		y := newLabel()
		n := newLabel
		TEST[ Q ]( y, n )
		label y:
		if not C then goto pass
		S
		goto done
		label n:
		FINI[ Q ]
		label done:

	
	NEXT[ Q until C then S else T ] --> NEXT[ Q ]
	ADVN[ Q until C then S else T ] --> ADV[ Q ]
	BODY[ Q until C then S else T ] --> BODY[ Q ]
	
	FINI[ Q until C then S else T ] --> ()


Nested Loops P & Q
------------------

	INIT( P & Q ) -->
		need_testp := true
		INIT[ P ]

	TEST[ P & Q ]( pass, fail ) -->
		repeat
			if need_testp then
				ok := newLabel()
				TEST[ P ]( ok, fail )
				label ok:
				need_testp ::= false
				NEXT[ P ]
				BODY[ P ]
				INIT[ Q ]
			else
				y := newLabel()
				n := newLabel()
				TEST[ Q ]( pass, n )
				label n:
				need_testp ::= true
				FINI[ Q ]
				ADVN[ P ]
			endif
		endrepeat

	NEXT[ P & Q ] -->
		ADV[ Q ]

	ADV[ P & Q ] -->
		ADV[ Q ]

	BODY[ P & Q ] -->
		BODY[ Q ]

	FINI[ P & Q ] -->
		FINI[ P ]