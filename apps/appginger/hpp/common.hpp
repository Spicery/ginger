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

#ifndef COMMON_HPP
#define COMMON_HPP

/*
	SAFE_MODE is a compile-time flag that controls the execution of
	relatively expensive run-time checks. Should be disabled for release.
	
	Set SAFE_MODE to 0 for faster execution.
*/
#ifdef DEBUG_MODE
#define SAFE_MODE 1
#else
#define SAFE_MODE 0
#endif

typedef long gnglong_t;             //  Should be compatible with (void *).
typedef unsigned long gngulong_t;   //  As above.
typedef unsigned long ulong;        //  Retire this typedef.
typedef double gngdouble_t;         //  So that we can easily swap to long double.

typedef const char *charseq;
class MachineClass;

typedef void * Ref;
typedef void ** RefRef;

typedef Ref * SysCall( Ref * pc, MachineClass * );

#define ToRef( x )  	( ( Ref )( x ) )
#define ToRefRef( x )	( ( RefRef )( x ) )
#define ToULong( x ) 	( ( gngulong_t )( x ) )
#define ToLong( x )		( ( gnglong_t )( x ) )
#define	ToChars( x ) 	( ( char * )( x ) )
#define ToSysCall( x ) 	( ( SysCall * )( x ) )

#define STANDARD_LIBRARY_PACKAGE	"ginger.library"
#define INTERACTIVE_PACKAGE			"ginger.interactive"

#endif

