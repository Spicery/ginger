#ifndef MACHINE_HPP
#define MACHINE_HPP

#include <memory>
#include <iostream> 

#include "dict.hpp"
#include "plant.hpp"
#include "instruction_set.hpp"
#include "heap.hpp"
#include "appginger.hpp"

class PlantClass;
typedef PlantClass * Plant;

class MachineClass {
private:
	AppGinger &						appg;
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

	void 			print_results( std::ostream & out, float time_taken );
	void 			print_results( float time_taken );

	void			printfn( Ref x );
	void			printfn( std::ostream & out, Ref x );
	Ref*			instructionShow( Ref * pc );
	Ref*			instructionShow( std::ostream & out, Ref * pc );
	void			resetMachine();
	PlantClass *	plant();		
	DictClass *		dict();
	HeapClass &		heap();
	
public:
	virtual Ref * setUpPC( Ref r );
	virtual void execute( Ref r ) = 0;
	virtual const InstructionSet & instructionSet() = 0;
	bool getShowCode();
	

public:
	MachineClass( AppGinger & appg );
	virtual ~MachineClass() {}
};

typedef class MachineClass *Machine;

class PlantClass;

Machine machine_new( int n );
PlantClass *  machine_plant( Machine vm );
void machine_reset( Machine vm );

#endif
