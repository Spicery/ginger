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

#ifndef READLINE_MNX_SOURCE_HPP
#define READLINE_MNX_SOURCE_HPP

#include <deque>
#include <string>

#include "mnxsrc.hpp"

namespace Ginger {

class ReadlineMnxSource : public Ginger::MnxSource {
private:
    static const char DEFAULT_CHAR = '\n'; 
    bool refill();
private:
    std::deque< char > buffer;
    char prev_ch;
    bool is_closed;
    std::string prompt;
public:
    ReadlineMnxSource() : prev_ch( DEFAULT_CHAR ), is_closed( false ) {
    }
    virtual ~ReadlineMnxSource() {
    }
public:
    virtual void setPrompt( const std::string & prompt );

    virtual bool hasNext();
    
    virtual bool get( char & ch );
    
    virtual bool peek( char & ch );

    virtual void unget();
};

} // namespace

#endif
