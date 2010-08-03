/******************************************************************************\
	Copyright (c) 2010 Stephen Leach. AppGinger is distributed under the terms 
	of the GNU General Public License. This file is part of AppGinger.

    AppGinger is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    AppGinger is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with AppGinger.  If not, see <http://www.gnu.org/licenses/>.
\******************************************************************************/

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
	//	Volatile! Only cached when a garbage collection
	//	might be triggered.
	Ref *		program_counter;
	Ref *		func_of_program_counter;

public:
	void check_call_stack_integrity();	//	debug

		
public:
	long		count;			//	Args count
	Ref	*		link;			//	Return address
	Ref	*		func_of_link;
	Ref	*		sp;
	Ref *		sp_base;
	Ref	*		sp_end;
	Ref	*		vp;
	Ref	*		vp_base;
	Ref	*		vp_end;
	
public:
	Ref	&		fastPeek() { return *vp; }
	Ref &		fastPeek( int n ) { return *( vp - n ); } 
	Ref			fastPop() { return *vp--; }
	Ref			fastSet( Ref r ) { return *vp = r; }
	void		fastPush( Ref r ) { *++vp = r; }
	Ref			fastSubscr( int n ) { return *( vp - n ); }
	void 		fastDrop( int n ) { vp -= n; }
	ptrdiff_t	stackLength() { return this->vp - this->vp_base; }
	void		checkStackRoom( long n );
	
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
	virtual Ref sysFastListIterator();
	virtual Ref * setUpPC( Ref r );
	virtual void execute( Ref r ) = 0;
	virtual const InstructionSet & instructionSet() = 0;
	bool getShowCode();
	

public:
	MachineClass( AppGinger & appg );
	virtual ~MachineClass();
};

typedef class MachineClass *Machine;

Machine machine_new( int n );
void machine_reset( Machine vm );

#endif
