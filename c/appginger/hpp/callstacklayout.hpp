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

#ifndef CALL_STACK_LAYOUT_HPP
#define CALL_STACK_LAYOUT_HPP

//	Don't choose 0 for these as that would be error prone if the
//	include files were not included.
#define CSLS_ORIGINAL 1
#define CSLS_VARIANT 2
#define CALL_STACK_LAYOUT_STYLE CSLS_VARIANT

#if CALL_STACK_LAYOUT_STYLE == CSLS_ORIGINAL

/*	UP is INCREASING addresses
+----------+
|   func   |    ^
+----------+    |
|   link   |    |   direction of growth
+----------+    |
|   prev   |
+----------+
|    N     |
+----------+
| slot[0]  | <-<
+----------+
| slot[1]  |
+----------+
| slot[2]  |
+----------+
|   ...    |
+----------+
|slot[N-1] |
+----------+
*/


	#define SP_OVERHEAD		4
	#define SP_FUNC			-4
	#define SP_LINK 		-3
	#define SP_PREV_SP 		-2
	#define SP_NSLOTS 		-1
	
	#define LOCAL( N ) 			VMSP[N]
	#define LOCAL_OF( X, N )	(X)[ N ]

#elif CALL_STACK_LAYOUT_STYLE == CSLS_VARIANT

//	Variant
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
|    N     |
+----------+    |
|   prev   |    |   direction of growth
+----------+    |
|   link   |    v
+----------+
|   func   |
+----------+
*/


	#define SP_OVERHEAD		4
	#define SP_FUNC			-4
	#define SP_LINK 		-3
	#define SP_PREV_SP 		-2
	#define SP_NSLOTS 		-1
	
	#define LOCAL( N ) 			VMSP[N]
	#define LOCAL_OF( X, N ) 	(X)[N]
	
#else
	#error
#endif

#endif