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

#ifndef CMP_HPP
#define CMP_HPP

#include "instruction.hpp"


enum CMP_OP {
	CMP_GTE,
	CMP_GT,
	CMP_EQ,
	CMP_LT,
	CMP_LTE,
	CMP_NEQ
};

extern CMP_OP revCmpOp( const CMP_OP );
extern Instruction cmpOpInstruction( const CMP_OP cmp );
extern Instruction cmpLocalSmallInstruction( const CMP_OP cmp );
extern Instruction cmpLocalLocalInstruction( const CMP_OP cmp );

#endif
