Command <registers/>
====================

Shows the values of the registers of the Ginger VM. For details about the 
GVM structure see <help topic="gvm"/>

	* PC 	virtual program counter, pointer to currently executing instruction
	* FUNC 	pointer to currently executing function
	* COUNT the number of items passed on the stack to the current call
	* LINK  the previous PC i.e. return address
	* FUNCLINK the previous value of FUNC

Example
-------

	steve% rlwrap -S '%% ' -p Green ./gvmtest
	%% <registers/>
	COUNT: 0
	PC (offset from FUNC): 0
	FUNC: <NULL>
	FUNCLINK: <NULL>
	%%
