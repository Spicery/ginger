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

#ifndef REF_PRINT_HPP
#define REF_PRINT_HPP

#include "key.hpp"

class RefPrint {
private:
	std::ostream & out;
	bool showing;

public:
	RefPrint( std::ostream & out ) : out( out ), showing( false ) {}

	void setShowing( const bool showing ) {
		this->showing = showing;
	}

private:
	//	Relies on null termination, which is potentially dodgy - except this
	//	is only for formatting printable characters.
	void refStringPrint( Ref * str_K ) ;

	void refListPrint( Ref sofar );

	void refVectorPrint( Ref * vec_K );

	void refMixedPrint( Ref * mix_K );

	enum Tag {
		TAG_OPEN,
		TAG_CLOSE,
		TAG_EMPTY
	};

	void refAttrMapPrint( Ref * attrmap_K, enum Tag flag );

	void refElementPrint( Ref * mix_K );

	void refRecordPrint( Ref * rec_K );

	void refWRecordPrint( Ref * rec_K );

	//	<TYPE FIELD ...>
	void refInstancePrint( Ref * rec_K );

	void refDoublePrint( const Ref r );
	
	void refKeyPrint( const Ref r );

	void refFunctionPrint( const Ref fn );

public:
	void refPrint( const Ref r );

};	//	end class.

#endif