#ifndef MACHINE_HPP
#define MACHINE_HPP

#include <memory>

#include "dict.hpp"
#include "plant.hpp"
#include "instruction_set.hpp"
#include "heap.hpp"

class PlantClass;
typedef PlantClass * Plant;

class MachineClass {
private:
	std::auto_ptr<PlantClass>		plant_aptr;
	std::auto_ptr<DictClass>		dict_aptr;
	std::auto_ptr<HeapClass>		heap_aptr;
	
public:
	long		count;			//	Args count
	Ref			*link;			//	Return address
	Ref			*sp;
	Ref			*sp_base;
	Ref			*sp_end;
	Ref			*vp;
	Ref			*vp_base;
	Ref			*vp_end;
	
public:
	void			printfn( Ref x );
	Ref*			instructionShow( Ref * pc );
	void			resetMachine();
	PlantClass *	plant();		
	DictClass *		dict();
	HeapClass &		heap();
	
public:
	virtual Ref * setUpPC( Ref r );
	virtual void execute( Ref r ) = 0;
	virtual const InstructionSet & instructionSet() = 0;
	

public:
	MachineClass();
	virtual ~MachineClass() {}
};

typedef class MachineClass *Machine;

class PlantClass;

Machine machine_new( int n );
PlantClass *  machine_plant( Machine vm );
void machine_reset( Machine vm );

#endif
