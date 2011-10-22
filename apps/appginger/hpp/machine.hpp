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
#include <vector>

#include "dict.hpp"
#include "plant.hpp"
#include "instruction_set.hpp"
#include "heap.hpp"
#include "appcontext.hpp"
#include "package.hpp"
#include "registers.hpp"

class PlantClass;
typedef PlantClass * Plant;

class Pressure {
private:
	float pressure;
	
public:
	void setUnderPressure() {
		this->pressure = 2.0;
	}
	
	void clearUnderPressure() {
		this->pressure = 0.0;
	}

	bool isUnderPressure() {
		//std::cout << "test pressure (" << ( this->pressure > 1.0 ? "yes" : "no" ) << ")" << std::endl;
		return this->pressure > 1.0;
	}
	
	void increasePressure() {
		this->pressure += 1.0;
		//std::cout << "pressure(up) " << this->pressure << std::endl;
	}
	
	void decreasePressure() {
		this->pressure *= 0.5;
		//std::cout << "pressure(down) " << this->pressure << std::endl;
	}
	
public:
	Pressure() : pressure( 0.0 ) {}
};


class MachineClass {
friend class GarbageCollect;

private:
	AppContext &					appg;
	std::auto_ptr<PlantClass>		plant_aptr;
	std::auto_ptr<HeapClass>		heap_aptr;
	Pressure						pressure;
	std::vector< Ref >				queue;

public:
	Registers						registers;
	
private:
	std::auto_ptr<PackageManager>	package_mgr_aptr;
	
public:
	Package * 		getPackage( std::string );
	Pressure &		getPressure();
	
public:
	//	Volatile! Only cached when a garbage collection
	//	might be triggered.
	Ref *			program_counter;
	Ref *			func_of_program_counter;

public:
	void 			check_call_stack_integrity();	//	debug

		
public:
	long			count;			//	Args count
	Ref	*			link;			//	Return address
	Ref	*			func_of_link;
	Ref	*			sp;
	Ref *			sp_base;
	Ref	*			sp_end;
	Ref	*			vp;
	Ref	*			vp_base;
	Ref	*			vp_end;
	
	
public:
	Ref	&			fastPeek() { return *vp; }
	Ref &			fastPeek( int n ) { return *( vp - n ); } 
	Ref				fastPop() { return *vp--; }
	Ref				fastSet( Ref r ) { return *vp = r; }
	void			fastPush( Ref r ) { *++vp = r; }
	Ref				fastSubscr( int n ) { return *( vp - n ); }
	void 			fastDrop( int n ) { vp -= n; }
	ptrdiff_t		stackLength() { return this->vp - this->vp_base; }
	void			checkStackRoom( long n );
	
public:

	void 			printResults( std::ostream & out, float time_taken );
	void 			printResults( float time_taken );

	void			printfn( Ref x );
	void			printfn( std::ostream & out, Ref x );
	Ref*			instructionShow( Ref * pc );
	Ref*			instructionShow( std::ostream & out, Ref * pc );
	void			resetMachine();
	PlantClass *	plant();		
	HeapClass &		heap();
	AppContext &	getAppContext();

	
public:
	virtual Ref sysFastListIterator();
	virtual Ref sysFastVectorIterator();
	virtual Ref sysFastMixedIterator();
	virtual Ref sysFastStringIterator();
	
public:
	virtual Ref * setUpPC( Ref r );
	virtual void execute( Ref r ) = 0;
	virtual const InstructionSet & instructionSet() = 0;

public:
	void executeQueue();
	void addToQueue( Ref r );

public:
	bool getShowCode();
	bool isGCTrace();
	
private:
	int veto_count;
	
public:
	//	TO BE IMPLEMENTED.
	void gcLiftAllVetoes();				//	Makes decision to grow/shrink, level = 0.
	void gcVeto();  					//	Inhibit moving, bump +1 for nesting.
	void gcLiftVeto();					//	bump -1, moving allowed if level = 0.
	bool gcMoveEnabled();
	void log( std::string msg ) {}	
	

public:
	MachineClass( AppContext & appg );
	virtual ~MachineClass();
};

typedef class MachineClass *Machine;

Machine machine_new( int n );
void machine_reset( Machine vm );

#endif

