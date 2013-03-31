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

#ifndef CODE_GEN_HPP
#define CODE_GEN_HPP

#include <vector>
#include <stack>
#include <memory>
#include "shared.hpp"

#include "arity.hpp"

#include "machine.hpp"
#include "common.hpp"
#include "instruction_set.hpp"
#include "label.hpp"
#include "mnx.hpp"
#include "sysfunction.hpp"
#include "vident.hpp"
#include "cmp.hpp"
#include "valof.hpp"

class MachineClass;
typedef MachineClass * Machine;

typedef class shared< Ginger::Mnx > Gnx;


struct CodeGenState {
	std::string 	props;
	int				nlocals;
	int 			ninputs;
	int 			nresults;				//	Unsure about this.
	int 			current_slot;
	shared< std::vector< Ref > >	code_data;
};

class CompileQuery {
private:
	CodeGenClass * codegen;
	VIdent * loop_var;

private:
	void setLoopVar( VIdent * v ) {
		this->loop_var = v;
	}

	VIdent & getLoopVar();

private:
	void compileQueryInit( Gnx query, LabelClass * contn );
	void compileQueryNext( Gnx query, LabelClass * contn );
	void compileQueryIfSo( Gnx query, LabelClass * dst, LabelClass * contn );

public:
	void compileFor( Gnx query, Gnx body, LabelClass * contn );

public:
	CompileQuery( CodeGenClass * cg ) : codegen( cg ), loop_var( NULL ) {}

	~CompileQuery() {
		if ( this->loop_var ) {
			delete this->loop_var;
		}
	}
};


class CodeGenClass {
friend class CompileQuery;

private:
	shared< std::vector< Ref > >	code_data;
	std::stack< CodeGenState > 	dump;
	
public:
	void save( const std::string name, int N, int A ) {
		CodeGenState s;
		s.current_slot = this->current_slot;
		s.props = this->props;
		s.ninputs = this->ninputs;
		s.nlocals = this->nlocals;
		s.nresults = this->nresults;		//	 Not too sure about this
		s.code_data = this->code_data;
		this->dump.push( s );
		
		this->code_data = shared< std::vector< Ref > >( new std::vector< Ref >() );
		this->props = name;
		this->nlocals = N;
		this->ninputs = A;
		this->current_slot = N;
		//	To-do: what about nresults????
	}

	void restore() {
		CodeGenState & s = dump.top();
		this->current_slot = s.current_slot;
		this->props = s.props;
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
	
	void codegenRef( Ref r ) {
		this->code_data->push_back( r );
	}
	
	const InstructionSet & instructionSet();
	
private:
	std::vector< LabelClass > 	label_vector;
	
public:
	LabelClass * newLabel();
	
public:
	std::vector< Ref > dump_data;
	int 		nresults;
	int			nlocals;
	int 		ninputs;
private:
	int 		current_slot;
 	std::string props;
public:
	int			tmpvar();				//	Change this name later.
public:
	MachineClass 	*vm;

private:	//	Helper functions.
	Valof * resolveGlobal( Gnx var_or_id );

public:
	void emitSPC( Instruction instr );
	void emitRef( Ref ref );
	void emitValof( Valof *v );
	void emitVAR_REF( Gnx id );
	void emitVIDENT_REF( const VIdent & id );
	
public:
	Ref vmiENDFUNCTION( bool in_heap );
	Ref vmiENDFUNCTION( bool in_heap, Ref fnkey );
	Ref vmiENDFUNCTION( Ref fnkey );
	Ref vmiENDFUNCTION();
	void vmiCALLS();
	void vmiCHAIN_LITE( Ref fn, long N );
	void vmiCHECK_COUNT( int N );
	void vmiCHECK_MARK( int local_N, int count_K );
	void vmiCHECK_MARK0( int local_N );
	void vmiCHECK_MARK1( int local_N );
	void vmiDECR( const long d );
	void vmiDEREF();
	void vmiEND_CALL_ID( int var, const VIdent & ident );
	void vmiEND_MARK( int N );
	void vmiEND1_CALLS( int var );
	void vmiENTER();
	void vmiERASE_MARK( int var );
	void vmiFIELD( long index );
	void vmiFUNCTION( int N, int A );
	void vmiFUNCTION( const std::string name, int N, int A );
	void vmiGOTO( LabelClass * d );
	void vmiHALT();
	void vmiIF( bool sense, LabelClass * d, LabelClass * contn );
	void vmiIFNOT( LabelClass * d, LabelClass * contn );
	void vmiIFNOT( LabelClass * dst );
	void vmiIFSO( LabelClass * d, LabelClass * contn );
	void vmiIFSO( LabelClass * dst );
	void vmiIFTEST( const bool sense, LabelClass * dst );
	void vmiINCR( const long d );
	void vmiINSTRUCTION( Instruction instr );
	void vmiINVOKE();
	void vmiMAKEREF();
	void vmiNOT();
	void vmiPOP_INNER_SLOT( int slot );
	void vmiPOP( Gnx var_or_id, const bool assign_vs_bind = true );
	void vmiPOP(const VIdent & vid, const bool assign_vs_bind = true );
	void vmiPUSH_INNER_SLOT( int slot );
	void vmiPUSH_INNER_SLOT( int slot, LabelClass * contn );
	void vmiPUSH( const VIdent & vid );
	void vmiPUSH( const VIdent & vid, LabelClass * contn );
	void vmiPUSH( Gnx var_or_id );
	void vmiPUSHQ_RETURN( Ref obj );
	void vmiPUSHQ( Ref obj );
	void vmiPUSHQ( Ref obj, LabelClass * contn );
	void vmiPUSHQ_STRING( const std::string & s, LabelClass * contn );
	void vmiRETURN();
	void vmiSELF_CALL();
	void vmiSELF_CALL_N( const int n );
	void vmiSELF_CONSTANT();
	void vmiSET_CALL_ID( int in_arity, const VIdent & ident );
	void vmiSET_CALL_INNER_SLOT( int in_arity, int slot );
	void vmiSET_CALLS( int in_arity );
	void vmiSET_SYS_CALL( SysCall * r, int A );
	void vmiSET( int A );
	void vmiSETCONT();
	void vmiSTART_MARK( int N );
	void vmiSYS_CALL_ARG( SysCall * r, Ref ref );
	void vmiSYS_CALL_ARGDAT( SysCall * r, Ref ref, unsigned long data );
	void vmiSYS_CALL_DAT( SysCall * r, unsigned long data );
	void vmiSYS_CALL( SysCall * r );
	void vmiSYS_RETURN();
	void vmiTEST( const VIdent & vid0, CMP_OP cmp_op, const VIdent & vid1, LabelClass * dst );
	void vmiAND( LabelClass * dst );
	void vmiOR( LabelClass * dst );
	void vmiABS_AND( LabelClass * dst );
	void vmiABS_OR( LabelClass * dst );

public:
	Ref calcConstant( Gnx mnx );

public:
	void continueFrom( LabelClass * contn );
	void compileComparison( bool sense, const VIdent & vid0, CMP_OP cmp_op,	const VIdent & vid1, LabelClass * dst, LabelClass * contn );	
	void compileComparison( const VIdent & vid0, CMP_OP cmp_op,	const VIdent & vid1, LabelClass * dst, LabelClass * contn );


private:
	void compileQueryInit( Gnx query, LabelClass * contn );
	void compileQueryNext( Gnx query, LabelClass * contn );
	void compileQueryIfSo( Gnx query, LabelClass * dst, LabelClass * contn );
	void compileFor( Gnx query, Gnx body, LabelClass * contn );
	void compileIfTest( bool sense, Gnx mnx, LabelClass * dst, LabelClass * contn );
	void compileGnxConstant( Gnx mnx, LabelClass * contn );
	void compileGnxIf( Gnx mnx, LabelClass * contn );
	void compileGnxIf( int offset, Gnx mnx, LabelClass * contn );
	void compileGnxSwitch( Gnx mnx, LabelClass * contn );
	void compileGnxSwitch( const int offset, const int switch_slot, int tmp_slot, Gnx mnx, LabelClass * contn );
	void compileGnxFn( Gnx mnx, LabelClass * contn );
	void compileGnxApp( Gnx mnx, LabelClass * contn );
	void compileGnxSysApp( Gnx mnx, LabelClass * contn );
	void compileGnxFor( Gnx mnx, LabelClass * contn );
	void compileGnxDeref( Gnx mnx, LabelClass * contn );
	void compileGnxMakeRef( Gnx mnx, LabelClass * contn );
	void compileGnxSetCont( Gnx mnx, LabelClass * contn );
	void compileGnxSelfCall( Gnx mnx, LabelClass * contn );
	void compileBoolAbsAndOr( bool bool_vs_abs, bool and_vs_or, Gnx mnx, LabelClass * contn );

	void compileErase( Gnx mnx, LabelClass * contn );
	void compileTry( Gnx mnx, LabelClass * contn );
	void compileThrow( Gnx mnx, LabelClass * contn );

	bool tryFlatten( Gnx expr, std::vector< Gnx > & vars );

public:
	void compileGnx( Gnx mnx, LabelClass * contn );
	void compileChildren( Gnx mnx, LabelClass * contn );
	void compileChildrenChecked( Gnx mnx, Ginger::Arity arity );

	Ref detach( bool in_heap, Ref fnkey );
	void compileN( Gnx mnx, int N, LabelClass * contn );
	void compile1( Gnx mnx, LabelClass * contn );
	void compile0( Gnx mnx, LabelClass * contn );
	void compileIf( bool sense, Gnx mnx, LabelClass * dst, LabelClass * contn );
	void compileIfNot( Gnx, LabelClass * dst, LabelClass * contn );
	void compileIfSo( Gnx, LabelClass * dst, LabelClass * contn );

public:
	CodeGenClass( MachineClass * machine );
};


typedef class CodeGenClass *CodeGen;



#endif

