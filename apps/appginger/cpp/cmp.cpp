/******************************************************************************\
	Copyright (c) 2010 Stephen Leach. AppGinger is distributed under the terms 
	of the GNU General Public License. This file is part of Ginger.

    Ginger is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Ginger is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Ginger.  If not, see <http://www.gnu.org/licenses/>.
\******************************************************************************/

#include "cmp.hpp"

namespace Ginger {

CMP_OP revCmpOp( const CMP_OP cmp ) {
	switch ( cmp ) {
		case CMP_GTE: 	return CMP_LT;
		case CMP_GT: 	return CMP_LTE;
		case CMP_EQ:	return CMP_NEQ;
		case CMP_NEQ:	return CMP_EQ;
		case CMP_LT:	return CMP_GTE;
		case CMP_LTE:	return CMP_GT;
		default:		throw;			//	Unreachable - inhibits compiler warning.
	}
}

Instruction cmpOpInstruction( const CMP_OP cmp ) {
	switch ( cmp ) {
		case CMP_GTE: 	return vmc_gte;
		case CMP_GT: 	return vmc_gt;
		case CMP_EQ:	return vmc_eq;
		case CMP_NEQ:	return vmc_neq;
		case CMP_LT:	return vmc_lt;
		case CMP_LTE:	return vmc_lte;
		default:		throw;			//	Unreachable - inhibits compiler warning.
	}
}

Instruction cmpLocalSmallInstruction( const CMP_OP cmp ) {
	switch ( cmp ) {
		case CMP_GTE: 	return vmc_gte_si;
		case CMP_GT: 	return vmc_gt_si;
		case CMP_EQ:	return vmc_eq_si;
		case CMP_NEQ:	return vmc_neq_si;
		case CMP_LT:	return vmc_lt_si;
		case CMP_LTE:	return vmc_lte_si;
		default:		throw;			//	Unreachable - inhibits compiler warning.
	}
}

Instruction cmpLocalLocalInstruction( const CMP_OP cmp ) {
	switch ( cmp ) {
		case CMP_GTE: 	return vmc_gte_ss;
		case CMP_GT: 	return vmc_gt_ss;
		case CMP_EQ:	return vmc_eq_ss;
		case CMP_NEQ:	return vmc_neq_ss;
		case CMP_LT:	return vmc_lt_ss;
		case CMP_LTE:	return vmc_lte_ss;
		default:		throw;			//	Unreachable - inhibits compiler warning.
	}
}

} // namespace Ginger
