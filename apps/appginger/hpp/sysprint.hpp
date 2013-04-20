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

#ifndef SYS_PRINT_HPP
#define SYS_PRINT_HPP

#include "key.hpp"
#include "common.hpp"
#include <iostream>

extern void refPrint( Ref r );
extern void refShow( const Ref r );
extern void refPtrPrint( Ref * r );
extern void refPrint( std::ostream & out, Ref r );
extern void refShow( std::ostream & out, const Ref r );
extern void refPtrPrint( std::ostream & out, Ref * r );
extern std::string refToPrintString( Ref ref );
extern std::string refToShowString( Ref ref );
extern Ref * sysRefPrint( Ref * pc, MachineClass * );
extern Ref * sysRefPrintln( Ref * pc, MachineClass * );
extern Ref * sysRefShow( Ref * pc, MachineClass * );
extern Ref * sysRefShowln( Ref * pc, MachineClass * );

#endif

