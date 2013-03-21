Command <stack.length/>
=======================

Prints the length of the GVM value stack.

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
	%% <stack.length/>
	2
	%% 