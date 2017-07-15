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
#ifndef SYS_SYMBOL_HPP
#define SYS_SYMBOL_HPP

#include <string>

#include "common.hpp"
#include "machine.hpp"

namespace Ginger {

extern void preGCSymbolTable( const bool gctrace );
extern void postGCSymbolTable( const bool gctrace );
extern void gcTouchSymbol( Ref r );
extern Ref * sysMakeSymbol( Ref *pc, MachineClass * vm );
extern const std::string & symbolToStdString( Ref r );
extern Ref refMakeSymbol( const std::string & );

} // namespace Ginger

#endif
