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

#include <sys/errno.h>

#ifdef GNU_FD_TO_IFSTREAM
    #include <ext/stdio_filebuf.h> // __gnu_cxx::stdio_filebuf
#else
    #include "fdifstream.hpp"
#endif


#include <iostream>
#include <sstream> 
#include <list>
#include <string>

#include "mishap.hpp"

#include "debug.hpp"
#include "component.hpp"


namespace Ginger {
using namespace std;

Component::Component( AppContext & cxt ) :
    started( false ),
    context( cxt )
{
}

Component::~Component() {
    if ( this->started ) {
        fclose( this->fout );
    }
}

void Component::initIfNeeded() {
    if ( not this->started ) {
        command.runWithInputAndOutput();
        this->started = true;
        this->fout = fdopen( command.getOutputFD(), "w" );
    }    
}

} // namespace Ginger
