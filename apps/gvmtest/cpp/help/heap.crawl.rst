Command <heap.crawl [zeros=("true"|"false")] [dump=("true"|"false")]/>
======================================================================

Each virtual machine has a large area of store devoted to managing variable
sized objects. The <heap.crawl/> command performs a "forward crawl" of the 
heap of the current virtual machine, visiting each object in turn and 
collecting statistics.

The heap is itself a collection of sub-heaps called "cages". The phrase
cage comes from the metaphor of the heap being a collection of animals where
animals of incompatible types are separately 'caged'.

[Aside: At the time of writing, cages are private to a heap but the long term 
plan is to share cages of deeply-immutable objects, where deeply-immutable means 
that all other objects reachable from the chosen object are also immutable. The
garbage collector will be responsible for migrating objects into the shareable,
cages of deeply-immutable objects.]

A forward crawl iterates over each cage in turn and all the objects (in order) 
of that cage. The objects are classified according to the following 
taxonomy and counts assigned for each member of the hierarchy.

	Object
		String
		Vector
		Record
		Pair
		Map
		Mixed (vector-like but with record fields)
		Function
			Core-function (will always be zero as are built out-of-heap)
			Method
		Instance (of a user-defined class, as opposed to built-in class)
		Wide-record (double width fields)
		Atomic-wide-record

Option zeros=("true"|"false")
-----------------------------

If the optional attribute zeros is set to "true" then zero-counts will be 
reported. Otherwise they are suppressed (default).


Option dump=("true"|"false")
----------------------------

If the optional attribute dump is set to "true" then output is generated for
each object discovered during the heap-crawl.


Example Output
--------------

	steve% rlwrap -S '%>> ' -p Green ./gvmtest
	%>> <compile><constant type="string" value="hello"/></compile>
	define: 0 args, 0 locals, 0 results, 0 #words used
	enddefine
	Note: Virtual machine stopped normally
	%>> <stack/>
	"hello"
	%>> <heap.crawl/>
	Start Cage
	    Serial : 0
	    Used   : 16
	    Capacity: 1048576
	    #Function: 1
	    #String: 1
	End Cage
	%>> <heap.crawl zeros="true"/>
	Start Cage
	    Serial : 0
	    Used   : 16
	    Capacity: 1048576
	    #Function: 1
	    #Function/Core: 0
	    #Function/Method: 0
	    #Instance: 0
	    #Vector: 0
	    #Mixed: 0
	    #Record: 0
	    #Pair: 0
	    #Map: 0
	    #WRecord: 0
	    #AtomicWRecord: 0
	    #String: 1
	    #OTHER: 0
	End Cage
	%>> 
