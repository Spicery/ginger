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

#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <string>

#include "rdlmnxsrc.hpp"

namespace Ginger {
using namespace std;

void ReadlineMnxSource::setPrompt( const std::string & _prompt ) {
    this->prompt = _prompt;
}


bool ReadlineMnxSource::refill() {
    if ( not this->is_closed ) {
        char * line = readline( this->prompt.c_str() );
        if ( line == 0 ) {
            this->is_closed = true;
        } else {
            string s( line );
            this->buffer.insert( this->buffer.end(), s.begin(), s.end() );
            this->buffer.push_back( '\n' );
            if ( *line != '\0' ) {
                add_history( line );
            }
            free( line );
        }
    }
    return this->is_closed;
}

bool ReadlineMnxSource::hasNext() {
    if ( this->is_closed ) return false;
    if ( this->buffer.empty() ) {
        if ( this->refill() ) return false;
    }
    return true;
}

bool ReadlineMnxSource::get( char & ch ) {
    if ( this->is_closed ) return false;
    if ( this->buffer.empty() ) {
        if ( this->refill() ) return false;
    }
    ch = this->prev_ch = this->buffer.front();
    this->buffer.pop_front();
    return true;
}

bool ReadlineMnxSource::peek( char & ch ) {
    if ( this->is_closed ) return false;
    if ( this->buffer.empty() ) {
        if ( this->refill() ) return false;
    }
    ch = this->buffer.front();
    return true;
   
}

void ReadlineMnxSource::unget() {
    this->buffer.push_back( this->prev_ch );
    this->prev_ch = ReadlineMnxSource::DEFAULT_CHAR;
}

} // namespace
