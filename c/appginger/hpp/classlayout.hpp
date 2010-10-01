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

#ifndef CLASS_LAYOUT_HPP
#define CLASS_LAYOUT_HPP

#define CLASS_OFFSET_TITLE 				1
#define CLASS_OFFSET_NFIELDS 			2
#define CLASS_OFFSET_DISPATCH_TABLE		3


#define sizeAfterKeyOfInstance( obj_K ) SmallToLong(RefToPtr4((obj_K)[0])[CLASS_OFFSET_NFIELDS])

#endif
