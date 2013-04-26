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

#ifndef GNG_OUTPUT_STREAM_HPP
#define GNG_OUTPUT_STREAM_HPP

#include <memory>
#include <ostream>
#include <string>

#include "external.hpp"

namespace Ginger {

class OutputStreamExternal : public External {
private:
    std::string file_name; 
    std::auto_ptr< std::ofstream > output;

public:
    OutputStreamExternal( std::string & _fname );
    virtual ~OutputStreamExternal();

public:
    void print( std::ostream & out );
    Ref * sysApply( Ref * pc, MachineClass * vm );

public:
    bool isOpen() const;
    bool isGood() const;
    void close();
};

} // namespace Ginger

#endif