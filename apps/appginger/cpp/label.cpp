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

#include <iostream>

#include "label.hpp"
#include "codegen.hpp"
#include "mishap.hpp"

namespace Ginger {
using namespace std;


/** This is used to set the destination of a label.
*/
void LabelClass::labelSet() {
	int here = this->codegen->codePosition();
	#ifdef DBG_LABEL
		clog << "Setting label at [" << here << "]" << endl;
	#endif
	if ( this->is_set ) {
		throw UnreachableError();
	}
	this->the_location = here;
	this->is_set = true;
	
	//	Now update any pending insertions.
	for ( std::vector< int >::iterator it = this->pending_vector.begin(); it != this->pending_vector.end(); ++it ) {
		int there = *it;
		#ifdef DBG_LABEL
			clog << "Pending update of [" << there << "] with jump " << ( here - there ) << endl;
		#endif
		this->codegen->codeUpdate( there, IntToRef( here - there ) );
	}
}

/* 	This plants a placeholder into the instruction stream that is suitable as
	an argument to a virtual instruction.
*/
void LabelClass::labelInsert() {
	int where = this->codegen->codePosition();
	if ( this->is_set ) {
		#ifdef DBG_LABEL
			clog << "Backward goto at [" << where << "] with jump " << where - this->the_location << endl;
		#endif
		this->codegen->codegenRef( IntToRef( this->the_location - where ) );
	} else {
		#ifdef DBG_LABEL
			clog << "Forward goto at [" << where << "]" << endl;
		#endif
		this->pending_vector.push_back( where );
		this->codegen->codegenRef( (Ref)(-1) );
	}
}

LabelClass * LabelClass::jumpToJump( LabelClass * contn ) {
	if ( contn == CONTINUE_LABEL ) {
		return this;
	} else {
		return contn;
	}
}

} // namespace Ginger
