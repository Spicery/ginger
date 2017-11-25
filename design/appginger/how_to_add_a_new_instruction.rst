How to Add a New Instruction
============================

1. 	Navigate to the instructions folder.
		cd $appginger/instructions

2. 	Create a new file for the instruction in it. The filename must be 
	of the form
		NAME.EXT
	where NAME is the name of the instruction and EXT describes the
	type of the instruction.
	
	The NAME must be suitable for use as a C++ variable name and must follow
	the rules for C++ variables.
	
	EXT must match the regexp /i(rv)*/ 
	Each letter indicates the type of an argument to the instruction.
		i = instruction
		c = constant, traced by the garbage collector
		r = reference, traced by garbage collector
		v = variable
		
3. 	The content of the file should make use of the following macros
	to access the virtual machine.
		
		pc 			= virtual program counter
		vm 			= pointer to virtual machine 
		VMVP 		= pointer to top of value stack, grows up
		VMSP		= pointer to the current stack frame, grows down
		VMLINK 		= return address
		VMLINKFUNC	= function containing return address
		VMPCFUNC	= function containing program counter
		LOCAL( expr ) = Nth local variable in current frame
		VMCOUNT     = count of #args supplied to a function call
	
	To exit, returning program counter, use the following macro. 
	
		RETURN( <expression> )
		
	The virtual machine state may be in fast-but-volatile locations.
	To force the state into/out-of the virtual machine object use
	
		FREEZE = copy volatile state into VM object
		MELT   = copy VM state into volatile store
		
4.	Add the new instruction to make_machine.py in 
	${GINGER_DEV_HOME}/apps/automatic/machine/. This will cause the
	appropriate C++ variables to be created. In that folder run:

		make clean
		make

5.  In order for the CodeGNX to work

	[a] Add constant definitions to vm_instructions.hpp in 
		${GINGER_DEV_HOME}/apps/appginger/hpp
	[b] Add to the method CodeGenClass::compileInstruction in
		${GINGER_DEV_HOME}/apps/appginger/cpp
 
	
5.	Change directory to the appginger source directory. Clean out the
	*.o files and rebuild.

		cd ${GINGER_DEV_HOME}
		make clean
		make
		
6.	Rerun functional tests.

		cd ${GINGER_DEV_HOME}
		make clean
		make check
