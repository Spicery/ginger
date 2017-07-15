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

#ifndef CALL_STACK_LAYOUT_HPP
#define CALL_STACK_LAYOUT_HPP

namespace Ginger {

//	Variant without NSLOT
/*	DOWN is DECREASING addresses

+----------+
|slot[N-1] |
+----------+
|   ...    |
+----------+
| slot[2]  |
+----------+
| slot[1]  |
+----------+
| slot[0]  | <-<
+----------+
|   prev   |    |   
+----------+    |	direction of growth
|   link   |    |
+----------+	v
|   func   |
+----------+
*/

#define SP_OVERHEAD		3
#define SP_FUNC			-3
#define SP_LINK 		-2
#define SP_PREV_SP 		-1

#define LOCAL( N ) 			VMSP[N]
#define LOCAL_OF( X, N ) 	(X)[N]
#define NSLOTS( sp )( ToRefRef( (sp)[ SP_PREV_SP ] ) - (sp) ) - SP_OVERHEAD;

} // namespace Ginger

#endif
