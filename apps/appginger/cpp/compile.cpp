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
#include <memory>

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
#include "compile.hpp"


namespace Ginger {
using namespace std;

#define FN2CODE     ( INSTALL_TOOL "/fn2code" )

static std::string executable() {
    if ( USESNAP ) {
        const char * snap = getenv( "SNAP" );
        if ( snap ) {
            return std::string( snap ) + FN2CODE;
        }
        
    }
    return FN2CODE;
}

Compile::Compile( AppContext & cxt ) :
    Component( cxt )
{
    this->command = std::make_unique< Command >( executable() );
}

Compile::~Compile() {
}

// void Compile::initIfNeeded() {
//     if ( not this->started ) {
//         command.runWithInputAndOutput();
//         this->started = true;
//         this->fout = fdopen( command.getOutputFD(), "w" );
//     } 
// }

Gnx Compile::compile( Gnx x ) {
    this->initIfNeeded();

    #ifdef DBG_COMPILE
        cerr << "appginger/simplify submitting GNX" << endl;
        cerr << "  [[";
        x->render( cerr );
        cerr << "]]" << endl;
    #endif 
    
    x->frender( this->fout );
    fflush( this->fout );


#ifdef GNU_FD_TO_IFSTREAM
    __gnu_cxx::stdio_filebuf<char> buf( command->getInputFD(), ios_base::in );
    istream input( &buf );
#else
    FileDescriptorIFStream input( command->getInputFD() );
#endif

    MnxReader reader( input );
    Gnx r = reader.readMnx();

    #ifdef DBG_SIMPLIFY
        cerr << "appginger/simplify receives GNX" << endl;
        cerr << "  [[";
        r->render( cerr );
        cerr << "]]" << endl;
    #endif 

    return x;
}

} // namespace Ginger
