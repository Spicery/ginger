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

#ifndef SPECIAL_HPP
#define SPECIAL_HPP

typedef Ref *Special1( Ref *pc, Machine vm );
typedef void Special2( void );
typedef void * Special3

typedef union {
	Special1	special1;
	Special2	special2;
	Special3	special3;
} Special;


#include "implementation.hpp"

void enter_error( int B, int A );
void call_error( Ref r );
const char *special_name( Special );
Ref *special_show( Ref * );

#ifdef IMPLEMENTATION3
	void special_interpreter( int mode, Ref *pc, Machine vm );
#endif

#include "special.h.auto"

#endif
