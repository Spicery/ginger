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

#ifndef GINGER_USER_INPUT_HPP
#define GINGER_USER_INPUT_HPP

#include <string>

#include "command.hpp"
#include "mnx.hpp"

namespace Ginger {

class UserInput : public MnxRepeater {
private:
    Command cmd;
    std::string buffer;
    std::string prompt;

public:
    UserInput( const Command & _cmd ) : cmd( _cmd ) {}

private:
    char * gnu_readline();

public:
    void setPrompt( const char * _prompt ) { this->prompt = _prompt; }
    Ginger::SharedMnx nextMnx();
};

}

#endif
