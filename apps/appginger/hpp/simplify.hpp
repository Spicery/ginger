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

#ifndef GNG_SIMPLIFY_HPP
#define GNG_SIMPLIFY_HPP

#include "shared.hpp"
#include "mnx.hpp"
#include "command.hpp"

#include "appcontext.hpp"
#include "package.hpp"
#include "component.hpp"

namespace Ginger {

typedef shared< Ginger::Mnx > Gnx;

class Simplify : Component {
private:
    Package * package;

public:
    Gnx simplify( Gnx x );
    Package * getPackage() const { return this->package; }  // todo: may not be needed any more.

public:
    Simplify( AppContext & cxt, Package * package );
    ~Simplify();
};

} // namespace Ginger

#endif
