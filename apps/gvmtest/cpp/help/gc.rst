Command <gc/>
=============

Invokes a QUIESCENT garbage-collection. 

Example
-------

	steve% rlwrap -S '%% ' -p Green ./gvmtest
	%% <compile><constant type="string" value="hellow world"/></compile>
	define: 0 args, 0 locals, 0 results, 0 #words used
	enddefine
	Note: Virtual machine stopped normally
	%% <heap.crawl/>
	Start Cage
	    Serial : 0
	    Used   : 17
	    Capacity: 1048576
	    #Function: 1
	    #String: 1
	End Cage
	%% <gc/>
	Garbage collection time: 0.035ms
	Number of objects copied             : 1
	Number of function objects copied    : 0
	Number of vector objects copied      : 0
	Number of record objects copied      : 0
	Number of user objects copied        : 0
	Number of string objects copied      : 1
	Number of wide-record objects copied : 0
	%% <heap.crawl/>
	Start Cage
	    Serial : 0
	    Used   : 0
	    Capacity: 1048576
	End Cage
	Start Cage
	    Serial : 1
	    Used   : 4
	    Capacity: 1048576
	    #String: 1
	End Cage
	%% 