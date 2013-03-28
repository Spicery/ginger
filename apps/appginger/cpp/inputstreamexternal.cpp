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


#include <iostream>
#include <fstream>

#include "inputstreamexternal.hpp"
#include "machine.hpp"
#include "key.hpp"

namespace Ginger {
using namespace std;

/// @todo Can this leak?
InputStreamExternal::InputStreamExternal( std::string & _fname ) :
    file_name( _fname ),
    input( new ifstream( _fname.c_str() ) )
{
}

InputStreamExternal::~InputStreamExternal() {
    cerr << "Deleting input stream for: " << this->file_name << endl;
}


void InputStreamExternal::print( std::ostream & out ) {
    out << "<InputStream \"" << this->file_name << "\">";
}

Ref * InputStreamExternal::sysApply( Ref * pc, MachineClass * vm ) {
    // Safe to assign to top, replacing the ref to the input stream.
    char c;
    vm->fastPeek() = this->input->get( c ).good() ? IntToChar( c ) : SYS_TERMIN;
    return pc;
}

bool InputStreamExternal::isGood() const {
    return this->input->good();
}

} // namespace Ginger