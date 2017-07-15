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

#include <cctype>
#include <unistd.h>

#include "mishap.hpp"
#include "mnxsrc.hpp"

// #define DBG_MNX_SOURCE

namespace Ginger {
using namespace std;

////////////////////////////////////////////////////////////////////////////////
//  InputStreamMnxSource
////////////////////////////////////////////////////////////////////////////////

bool InputStreamMnxSource::hasNext() {
    #ifdef DBG_MNX_SOURCE
        cerr << "hasNext" << endl;
    #endif
    if ( this->buffer.empty() ) {
        const int pch = this->src.peek();
        #ifdef DBG_MNX_SOURCE
            cerr << "  pch: " << pch << endl;
            cerr << "  good: " << this->src.good() << endl;
        #endif
        return pch != EOF;
    } else {
        return true;
    }
}

bool InputStreamMnxSource::get( char & ch ) {
    #ifdef DBG_MNX_SOURCE
        cerr << "get" << endl;
    #endif
    if ( this->buffer.empty() ) {    
        char gch;    
        this->src.get( gch );
        const bool good = this->src.good();
        #ifdef DBG_MNX_SOURCE
            cerr << "  ch  : " << static_cast< int >( gch ) << endl;
            cerr << "  good: " << good << endl;
        #endif
        if ( good ) {
            ch = this->prev_ch = gch;
        }
        return good;
    } else {
        ch = this->prev_ch = this->buffer.front();
        this->buffer.pop_front();
        return true;
    }
}

bool InputStreamMnxSource::peek( char & ch ) {
    #ifdef DBG_MNX_SOURCE
        cerr << "peek" << endl;
    #endif
    if ( this->buffer.empty() ) {
        const int pch = this->src.peek();
        const bool good = this->src.good();
        #ifdef DBG_MNX_SOURCE
            cerr << "  pch: " << pch << endl;
            cerr << "  good: " << this->src.good() << endl;
        #endif
        if ( pch == EOF ) {
            return false;
        } else {
            if ( good ) {
                ch = static_cast< char >( pch );
            }
            return good;
        }
    } else {
        ch = this->buffer.front();
        return true;
    }
}

void InputStreamMnxSource::unget() {
    #ifdef DBG_MNX_SOURCE
        cerr << "unget" << endl;
        cerr << "  good: " << this->src.good() << endl;
    #endif
    this->buffer.push_back( this->prev_ch );
}


////////////////////////////////////////////////////////////////////////////////
//  ProxyMnxSource
////////////////////////////////////////////////////////////////////////////////

bool ProxyMnxSource::hasNext() {
    return this->msrc.hasNext();
}

bool ProxyMnxSource::get( char & ch ) {
    return this->msrc.get( ch );
}

bool ProxyMnxSource::peek( char & ch ) {
    return this->msrc.peek( ch );
}

void ProxyMnxSource::unget() {
    this->msrc.unget();
}

} // namespace Ginger
