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

#include <sys/errno.h>
#include <ext/stdio_filebuf.h> // __gnu_cxx::stdio_filebuf


#include <iostream>
#include <sstream> 
#include <list>

#include "mishap.hpp"

#include "debug.hpp"
#include "simplify.hpp"


namespace Ginger {
using namespace std;

#define SIMPLIFYGNX     ( INSTALL_TOOL "/simplifygnx" )

Simplify::Simplify( AppContext & cxt, Package * package ) :
    started( false ),
    context( cxt ),
    command( SIMPLIFYGNX )
{
    command.addArg( "-suA" );
    {
        list< string > & folders = cxt.getProjectFolderList();
        for ( 
            list< string >::iterator it = folders.begin();
            it != folders.end();
            ++it
        ) {
            command.addArg( "-j" );
            command.addArg( *it );
        }
    }
    command.addArg( "-p" );
    command.addArg( package->getTitle() );
}

Simplify::~Simplify() {
    if ( this->started ) {
        fclose( this->fout );
    }
}

void Simplify::initIfNeeded() {
    if ( not this->started ) {
        command.runWithInputAndOutput();
        this->started = true;
        this->fout = fdopen( command.getOutputFD(), "w" );
    }    
}

Gnx Simplify::simplify( Gnx x ) {
    this->initIfNeeded();

    #ifdef DBG_SIMPLIFY
        cerr << "appginger/simplify submitting GNX" << endl;
        cerr << "  [[";
        x->render( cerr );
        cerr << "]]" << endl;
    #endif 
    
    x->frender( fout );
    fflush( fout );

    __gnu_cxx::stdio_filebuf<char> buf( command.getInputFD(), ios_base::in );
    istream input( &buf );

    MnxReader reader( input );
    Gnx r = reader.readMnx();

    #ifdef DBG_SIMPLIFY
        cerr << "appginger/simplify receives GNX" << endl;
        cerr << "  [[";
        r->render( cerr );
        cerr << "]]" << endl;
    #endif 

    return r;
}

} // namespace Ginger