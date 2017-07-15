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

#ifndef GNG_DOUBLE_LAYOUT_HPP
#define GNG_DOUBLE_LAYOUT_HPP

#include "common.hpp"

namespace Ginger {

#define DOUBLE_VALUE_SIZE           ( ( sizeof( gngdouble_t ) + sizeof( Ref ) - 1 ) / sizeof( Ref ) )
#define DOUBLE_SIZE                 ( 1 + DOUBLE_VALUE_SIZE )
#define DOUBLE_OFFSET_VALUE         1

} // namespace Ginger

#endif
