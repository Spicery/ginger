Command <stack [order=("top-first"|"top-last")]/>
=========================================

Prints out the contents of the stack, one per line. 

Optional Attribute: order
-------------------------

The attribute "order" determines whether items are printed out top-first or 
top-last. The default value is top-first.

Example
-------

	steve% rlwrap -S '%% ' -p Green ./gvmtest
	%% <compile><constant type="int" value="101"/><constant type="int" value="102"/></compile>
	define: 0 args, 0 locals, 0 results, 0 #words used
	enddefine
	Note: Virtual machine stopped normally
	define: 0 args, 0 locals, 0 results, 0 #words used
	enddefine
	Note: Virtual machine stopped normally
	%% <stack order="top-first"/>
	102
	101
	%% <stack order="top-last"/>
	101
	102
	%%