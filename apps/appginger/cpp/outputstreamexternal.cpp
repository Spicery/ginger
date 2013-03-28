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

#include "outputstreamexternal.hpp"
#include "machine.hpp"
#include "key.hpp"
#include "cell.hpp"

namespace Ginger {
using namespace std;

/// @todo Can this leak?
OutputStreamExternal::OutputStreamExternal( std::string & _fname ) :
    file_name( _fname ),
    output( new ofstream( _fname.c_str() ) )
{
}

OutputStreamExternal::~OutputStreamExternal() {
    cerr << "Deleting output stream for: " << this->file_name << endl;
}


void OutputStreamExternal::print( std::ostream & out ) {
    out << "<OutputStream \"" << this->file_name << "\">";
}

Ref * OutputStreamExternal::sysApply( Ref * pc, MachineClass * vm ) {

    cerr << "There are " << vm->count << " arguments on the stack" << endl;

    for ( int i = vm->count - 1; i > 0; i-- ) {
        Cell c( vm->fastPeek( i ) );
        if ( c.isTermin() ) {
            this->output->close();
        } else if ( c.isCharacter() ) {
            CharacterCell cc( c );
            *this->output << cc.getChar();
        } else if ( c.isStringObject() ) {
            StringObject strobj( c );
            *this->output << strobj.getCharPtr();
        } else {
            throw Mishap( "Unexpected item to print" ).culprit( "Item", c.toPrintString() );
        }
    }
    vm->fastDrop( vm->count );

    // Safe to assign to top, replacing the ref to the output stream.
    //char c;
    //vm->fastPeek() = this->output->get( c ).good() ? IntToChar( c ) : SYS_TERMIN;
    return pc;
}

bool OutputStreamExternal::isGood() const {
    return this->output->good();
}

} // namespace Ginger