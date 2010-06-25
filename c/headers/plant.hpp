#ifndef PLANT_HPP
#define PLANT_HPP

#include <vector>
#include <stack>
#include "shared.hpp"

#include "machine.hpp"
#include "common.hpp"
#include "instruction_set.hpp"
#include "destination.hpp"

class MachineClass;
typedef MachineClass * Machine;

class PlantState {
public:
	int			nlocals;
	int 		ninputs;
	int 		nresults;				//	Unsure about this.
	int 		slot;
	shared< std::vector< Ref > >	code_data;
};

class PlantClass {
private:
//	boost::shared_ptr< std::vector< Ref > >	code_data;
	shared< std::vector< Ref > >	code_data;
	std::stack< PlantState > 	dump;
	
public:
	void save( int N, int A ) {
		PlantState s;
		s.slot = this->slot;
		s.ninputs = this->ninputs;
		s.nlocals = this->nlocals;
		s.nresults = this->nresults;		//	 Not too sure about this
		s.code_data = this->code_data;
		this->dump.push( s );
		
		this->code_data = shared< std::vector< Ref > >( new std::vector< Ref >() );
		this->nlocals = N;
		this->ninputs = A;
		this->slot = N;
		//	To-do: what about nresults????
	}
	
	void restore() {
		const PlantState & s = dump.top();
		this->slot = s.slot;
		this->ninputs = s.ninputs;
		this->nlocals = s.nlocals;
		this->nresults = s.nresults;		//	Unsure about this.
		this->code_data = s.code_data;
		this->dump.pop();
	}
	
public:
	size_t codePosition() {
		return this->code_data->size();
	}
	
	void codeUpdate( int idx, Ref r ) {
		(*(this->code_data))[ idx ] = r;
	}
	
	void plantRef( Ref r ) {
		this->code_data->push_back( r );
	}
	
	const InstructionSet & instructionSet();
	
	Ref detach();
	
private:
	std::vector< DestinationClass > 	destination_vector;
	
public:

	DestinationClass & newDestination();
	
public:

	std::vector< Ref > dump_data;
	int 		nresults;
	int			nlocals;
	int 		ninputs;
	int 		slot;
	MachineClass 	*vm;

public:
	PlantClass( MachineClass * machine );
};


typedef class PlantClass *Plant;

#include "term.hpp"
#include "machine.hpp"
#include "plant.hpp"

void plant_term( Plant plant, Term term );
Ref plant_detach( Plant plant );
void plant_count( Plant plant, Term term );
void plant1( Plant plant, Term term );
void plant0( Plant plant, Term term );

#endif
