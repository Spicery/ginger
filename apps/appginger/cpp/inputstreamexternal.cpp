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

#include "listlayout.hpp"
#include "inputstreamlayout.hpp"
#include "inputstreamexternal.hpp"
#include "machine.hpp"
#include "key.hpp"
#include "cell.hpp"

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
    Ref * input_stream_K = RefToPtr4( vm->fastPeek() );
    Ref pushed = input_stream_K[ PUSHBACK_OFFSET_INPUT_STREAM ];
    if ( pushed == SYS_NIL ) {
        // Safe to assign to top, replacing the ref to the input stream.
        char c;
        vm->fastPeek() = this->input->get( c ).good() ? IntToChar( c ) : SYS_TERMIN;
    } else {
        Ref * pair_K = RefToPtr4( pushed );
        Ref item = pair_K[ PAIR_HEAD_OFFSET ];
        vm->fastPeek() = item;
        input_stream_K[ PUSHBACK_OFFSET_INPUT_STREAM ] = pair_K[ PAIR_TAIL_OFFSET ];
    }
    return pc;
}

bool InputStreamExternal::isGood() const {
    return this->input->good();
}

void InputStreamExternal::close() {
    this->input->close();
}

bool InputStreamExternal::getline( std::string & line ) {
    return std::getline( *this->input, line ).good();
}

} // namespace Ginger