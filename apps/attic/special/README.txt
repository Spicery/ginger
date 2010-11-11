--- Handwritten files

Makefile		
special.c		

--- Automatically generated files

special1.c.auto		
special2.c.auto
special3.c.auto

footer.c.auto		
special.h.auto		

--- Target Types & Classes

Instruction
	instruction.hpp
		enum Instruction
	instruction.hpp.auto
		<contents>

InstructionSet
	instruction_set.hpp
		class InstructionSet
	instruction_set.hpp.auto
		<contents>
	instruction_set.cpp.auto
		InstructionSet::name( Ref ) -> const char *
		InstructionSet::classify( Ref ) ->const char *
	
InstructionSet1
	instruction_set1.hpp.auto
		class InstructionSet1
	instruction_set1.cpp.auto
		InstructionSet1::InstructionSet1()

InstructionSet2
	instruction_set2.hpp.auto
		class InstructionSet2	
	instruction_set2.cpp.auto
		InstructionSet2::InstructionSet3()

InstructionSet3
	instruction_set3.hpp.auto
		class InstructionSet3
	instruction_set3.cpp.auto
		InstructionSet3::InstructionSet3()

Implementation1
	implementation1.hpp
		class Implementation1
	implementation1.cpp
		Implementation1::instructionSet()
		Implementation1::execute( VM, Ref * PC )

Implementation2
	implementation2.hpp
	implementation2.cpp
	
Implementation3
	implementation3.hpp
		class Implementation3
	implementation3.cpp
		Implementation3::instructionSet()
		Implementation3::execute( VM, Ref * PC )
	implememtation3.cpp.auto
		Implementation3::execute( bool set_up, VM, Ref * PC )
