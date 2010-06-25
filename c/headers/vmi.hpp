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
void vmiIF_RELOP( Plant plant, char flag1, int arg1, char op, char flag2, int arg2, DestinationClass & dst );
void vmiIFNOT_RELOP( Plant plant, char flag1, int arg1, char op, char flag2, int arg2, DestinationClass & dst );
void vmiEND1_CALLS( Plant plant, int var );
void vmiSET_CALLS( Plant plant, int in_arity );
void vmiEND_CALL_ID( Plant plant, int var, Ident ident );
void vmiSET_CALL_ID( Plant plant, int in_arity, Ident ident );
void vmiSET( Plant plant, int A );
void vmiNOT( Plant plant );

#endif
