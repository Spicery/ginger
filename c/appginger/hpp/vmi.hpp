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

void vmiOPERATOR( Plant plant, Functor fnc );
void vmiINCR( Plant plant, int n );
void vmiPUSHQ( Plant plant, Ref obj );
void vmiPUSHID( Plant plant, Ident id );
void vmiPOPID( Plant plant, Ident id );
void vmiAPPSPC( Plant plant, Instruction instr );
void vmiSYS_CALL( Plant plant, Ref r );
void vmiSYS_RETURN( Plant plant );
void vmiRETURN( Plant plant );
void vmiENTER( Plant plant );
void vmiFUNCTION( Plant plant, int N, int A );
Ref vmiENDFUNCTION( Plant plant );
void vmiSTART( Plant plant, int N );
void vmiEND( Plant plant, int N );
void vmiCHECK1( Plant plant, int N );
void vmiCHECK0( Plant plant, int N );
void vmiCALLS( Plant plant );
void vmiIFNOT( Plant plant, DestinationClass & d );
void vmiIFSO( Plant plant, DestinationClass & d );
void vmiIF( bool sense, Plant plant, DestinationClass & d );
void vmiGOTO( Plant plant, DestinationClass & d );
void vmiIF_RELOP( Plant plant, bool sense, char flag1, int arg1, char op, char flag2, int arg2, DestinationClass & dst );
void vmiIFSO_RELOP( Plant plant, char flag1, int arg1, char op, char flag2, int arg2, DestinationClass & dst );
void vmiIFNOT_RELOP( Plant plant, char flag1, int arg1, char op, char flag2, int arg2, DestinationClass & dst );
void vmiEND1_CALLS( Plant plant, int var );
void vmiSET_CALLS( Plant plant, int in_arity );
void vmiEND_CALL_ID( Plant plant, int var, Ident ident );
void vmiSET_CALL_ID( Plant plant, int in_arity, Ident ident );
void vmiSET( Plant plant, int A );
void vmiNOT( Plant plant );

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
