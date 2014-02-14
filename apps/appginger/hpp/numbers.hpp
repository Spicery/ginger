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

#ifndef GNG_NUMBERS_HPP
#define GNG_NUMBERS_HPP

#include "common.hpp"
#include "key.hpp"

namespace Ginger {
namespace Numbers {

    const int BITS_PER_BYTE = 8;
	const gnglong_t MIN_SMALL = -( 1L << ( sizeof( Ref ) * BITS_PER_BYTE - 1 - TAG ) );
	const gnglong_t MAX_SMALL = ( 1L << ( sizeof( Ref ) * BITS_PER_BYTE - 1 - TAG ) ) - 1L;


} // end-namespace.
} // end-namespace.

#endif