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

#ifndef GNG_COMPONENT_HPP
#define GNG_COMPONENT_HPP

#include <memory>

#include "shared.hpp"
#include "mnx.hpp"
#include "command.hpp"

#include "appcontext.hpp"
#include "package.hpp"

namespace Ginger {

typedef shared< Ginger::Mnx > Gnx;

class Component {
protected:
    bool started;
    AppContext & context;
    std::unique_ptr< Ginger::Command > command;
    FILE * fout;

protected:
    void initIfNeeded();

public:
    AppContext & getAppContext() { return this->context; }  // todo: may not be needed any more.

public:
    Component( AppContext & cxt );
    ~Component();
};

} // namespace Ginger

#endif
