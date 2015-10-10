Issues with Queries
===================

BIND 
----

Problem
~~~~~~~
With the introduction of queries (logic programming), BIND takes onthe semantics of one-way unification. As a consequence a mismatch in the number of arguments is no longer an exception but merely a failiure.

As an unhappy consequence the following loop does not throw an exception but merely silently fails::

	for i in L & ( x, y ) := [ i, i ] do 
		<never get here>
	endfor

Solution
~~~~~~~~
There should be two types of BIND, one in which the binding is mandatory and the
other which is optional. The ":=" should introduce mandatory binding.

TRY
---

Problem
~~~~~~~
Similarly, when handling an escape we have this situation::

	try 
		<expr>
	catch Event( x, y ) then
		<reached if the args match x,y>
	else	
		<catchall reached if (x,y) does not bind>
	endtry

Solution
~~~~~~~~
If we match the EVENT then we want to commit to that branch of the try-block.
