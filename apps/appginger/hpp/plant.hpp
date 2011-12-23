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

#ifndef PLANT_HPP
#define PLANT_HPP

#include <vector>
#include <stack>
#include "shared.hpp"

#include "machine.hpp"
#include "common.hpp"
#include "instruction_set.hpp"
#include "destination.hpp"
#include "term.hpp"
#include "sysfunction.hpp"

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
	void emitSPC( Instruction instr );
	void emitRef( Ref ref );
	void emitValof( Valof *v );
	
private:
	void vmiCMP_ID_CONSTANT( bool flag, Ident id, Ref r, DestinationClass & d );
	void vmiCMP_ID_ID( bool flag, Ident id1, Ident id2, DestinationClass & d );

public:
	void vmiOPERATOR( Functor fnc );
	void vmiFIELD( long index );
	void vmiINCR( int n );
	void vmiPUSHQ( Ref obj );
	void vmiNEWID( Ident id );
	void vmiCOPYID( Ident id );
	void vmiPUSHID( Ident id );
	void vmiPUSH_INNER_SLOT( int slot );
	void vmiPOPID( Ident id );
	void vmiPOP_INNER_SLOT( int slot );
	void vmiINSTRUCTION( Instruction instr );
	void vmiINVOKE();
	void vmiSYS_CALL( SysCall * r );
	void vmiSYS_CALL_ARG( SysCall * r, Ref ref );
	void vmiSYS_CALL_DAT( SysCall * r, unsigned long data );
	void vmiSYS_CALL_ARGDAT( SysCall * r, Ref ref, unsigned long data );
	void vmiSET_SYS_CALL( SysCall * r, int A );
	void vmiSYS_RETURN();
	void vmiRETURN();
	void vmiENTER();
	void vmiFUNCTION( int N, int A );
	Ref vmiENDFUNCTION();
	Ref vmiENDFUNCTION( bool in_heap );
	Ref vmiENDFUNCTION( Ref fnkey );
	Ref vmiENDFUNCTION( bool in_heap, Ref fnkey );
	void vmiSTART_MARK( int N );
	void vmiEND_MARK( int N );
	void vmiCHAIN_LITE( Ref fn, long N );
	void vmiCHECK_COUNT( int N );
	void vmiCHECK_MARK1( int N );
	void vmiCHECK_MARK0( int N );
	void vmiCALLS();
	void vmiIFNOT( DestinationClass & d );
	void vmiIFSO( DestinationClass & d );
	void vmiIF( bool sense, DestinationClass & d );
	void vmiGOTO( DestinationClass & d );
	void vmiIF_RELOP( bool sense, char flag1, int arg1, char op, char flag2, int arg2, DestinationClass & dst );
	void vmiIFSO_RELOP( char flag1, int arg1, char op, char flag2, int arg2, DestinationClass & dst );
	void vmiIFNOT_RELOP( char flag1, int arg1, char op, char flag2, int arg2, DestinationClass & dst );
	void vmiEND1_CALLS( int var );
	void vmiSET_CALLS( int in_arity );
	void vmiEND_CALL_ID( int var, Ident ident );
	void vmiSET_CALL_ID( int in_arity, Ident ident );
	void vmiSET( int A );
	void vmiNOT();
	void vmiIF_EQ_ID_CONSTANT( Ident id, Ref constant, DestinationClass & d );
	void vmiIF_EQ_ID_ID( Ident id1, Ident id2, DestinationClass & d );
	void vmiIF_NEQ_ID_CONSTANT( Ident id, Ref constant, DestinationClass & d );
	void vmiIF_NEQ_ID_ID( Ident id1, Ident id2, DestinationClass & d );
	void vmiSETCONT();
	void vmiDEREF();
	void vmiMAKEREF();


private:
	void compileQueryInit( Term query );
	void compileQueryNext( Term query );
	void compileQueryIfSo( Term query, DestinationClass & dst );
	void compileFor( Term query, Term body );
	Ident newTmpIdent();


public:
	void compileTerm( Term term );
	void compileArgs( Term term );

	Ref detach( bool in_heap, Ref fnkey );
	void compile1( Term term );
	void compile0( Term term );
	void compileIf( bool sense, Term term, DestinationClass & dst );
	void compileIfNot( Term, DestinationClass& );
	void compileIfSo( Term, DestinationClass& );

public:
	PlantClass( MachineClass * machine );
};


typedef class PlantClass *Plant;



#endif

