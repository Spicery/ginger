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

#include "functionlayout.hpp"
#include "key.hpp"

namespace Ginger {

unsigned long sizeInstructionsAfterKeyOfFn( Ref * fn_K ) {
    return ToULong( fn_K[ OFFSET_TO_FUNCTION_LENGTH ] ) >> TAGGG;
}


unsigned long sizeAfterKeyOfFn( Ref * fn_K ) {
	unsigned long instr_width = sizeInstructionsAfterKeyOfFn( fn_K );
    Ref * name_P = fn_K + 1 + instr_width;
    unsigned long d = ToULong( name_P[ 0 ] );
    return instr_width + 1 + ( d + sizeof( Ref ) - 1 ) / sizeof( Ref );
}

long numOutputsOfFn( Ref * key ) {
	return ToLong( key[ OFFSET_TO_NUM_OUTPUTS ] );
}

long numSlotsOfFn( Ref * key ) {
	return ToLong( key[ OFFSET_TO_NUM_SLOTS ] );
}

long numInputsOfFn( Ref * key ) {
	return ToLong( key[ OFFSET_TO_NUM_INPUTS ] );
}



const std::string nameOfFn( Ref * fn_K ) {
    Ref * name_P = fn_K + 1 + sizeInstructionsAfterKeyOfFn( fn_K );
    unsigned long d = ToULong( name_P[ 0 ] );

    char * bytes = reinterpret_cast< char * >( name_P + 1 );
    std::string sofar;
    for ( int i = 0; i < d; i++ ) {
        sofar.push_back( bytes[ i ] );
    }
    return sofar;
}

} // namespace Ginger