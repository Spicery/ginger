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

#ifndef VMI_HPP
#define VMI_HPP

#include "destination.hpp"
#include "plant.hpp"
#include "common.hpp"
#include "ident.hpp"
#include "instruction.hpp"

extern void vmiOPERATOR( Plant plant, Functor fnc );
extern void vmiINCR( Plant plant, int n );
extern void vmiPUSHQ( Plant plant, Ref obj );
extern void vmiPUSHID( Plant plant, Ident id );
extern void vmiPOPID( Plant plant, Ident id );
extern void vmiINSTRUCTION( Plant plant, Instruction instr );
extern void vmiSYS_CALL( Plant plant, SysCall * r );
extern void vmiSYS_RETURN( Plant plant );
extern void vmiRETURN( Plant plant );
extern void vmiENTER( Plant plant );
extern void vmiFUNCTION( Plant plant, int N, int A );
extern Ref vmiENDFUNCTION( Plant plant );
extern Ref vmiENDFUNCTION( Plant plant, bool in_heap );
extern void vmiSTART( Plant plant, int N );
extern void vmiEND( Plant plant, int N );
extern void vmiCHECK1( Plant plant, int N );
extern void vmiCHECK0( Plant plant, int N );
extern void vmiCALLS( Plant plant );
extern void vmiIFNOT( Plant plant, DestinationClass & d );
extern void vmiIFSO( Plant plant, DestinationClass & d );
extern void vmiIF( bool sense, Plant plant, DestinationClass & d );
extern void vmiGOTO( Plant plant, DestinationClass & d );
extern void vmiIF_RELOP( Plant plant, bool sense, char flag1, int arg1, char op, char flag2, int arg2, DestinationClass & dst );
extern void vmiIFSO_RELOP( Plant plant, char flag1, int arg1, char op, char flag2, int arg2, DestinationClass & dst );
extern void vmiIFNOT_RELOP( Plant plant, char flag1, int arg1, char op, char flag2, int arg2, DestinationClass & dst );
extern void vmiEND1_CALLS( Plant plant, int var );
extern void vmiSET_CALLS( Plant plant, int in_arity );
extern void vmiEND_CALL_ID( Plant plant, int var, Ident ident );
extern void vmiSET_CALL_ID( Plant plant, int in_arity, Ident ident );
extern void vmiSET( Plant plant, int A );
extern void vmiNOT( Plant plant );
extern void vmiIF_EQ_ID_CONSTANT( Plant plant, Ident id, Ref constant, DestinationClass & d );
extern void vmiIF_EQ_ID_ID( Plant plant, Ident id1, Ident id2, DestinationClass & d );
extern void vmiIF_NEQ_ID_CONSTANT( Plant plant, Ident id, Ref constant, DestinationClass & d );
extern void vmiIF_NEQ_ID_ID( Plant plant, Ident id1, Ident id2, DestinationClass & d );


class VmiRelOpFactory {
private:
	Plant plant;
	char flag1;
	Ident ident1;
	int int1;
	char op;
	char flag2;
	Ident ident2;
	int int2;
	
public:
	void setLeft( int arg1 );
	void setLeft( Ident id );
	void setRight( int arg1 );
	void setRight( Ident id );
	void setLT();
	void setGT();
	void setLTE();
	void setGTE();
	void setEQ();
	void setNEQ();
	void negate();
	
private:
	void compilePushLeft();
	void compilePushRight();
	void compileOp();

	
public:
	void ifSo( DestinationClass &dst );
	void ifNot( DestinationClass &dst );
	
public:
	VmiRelOpFactory( Plant p ) :
		plant( p ),
		flag1( '?' ),
		op( '?' ),
		flag2( '?' )
	{
	}
};

#endif

