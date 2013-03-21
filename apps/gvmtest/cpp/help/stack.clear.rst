Command <stack.clear/>
======================

This command clears the value stack. Be careful when doing this because the
COUNT register is not adjusted.

For a description of the virtual machine structure see
	<help topic="gvm"/>

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
	%% <stack.clear/>
	%% <stack.length/>
	0
	%% 
