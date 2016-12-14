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

#ifndef MNX_SOURCE_HPP
#define MNX_SOURCE_HPP

#include <deque>
#include <vector>
#include <iostream>
#include <ostream>

namespace Ginger {


class MnxSource {
public:
	MnxSource() {}
	virtual ~MnxSource() {}
public:
	virtual bool hasNext() = 0;

	virtual bool get( char & ch ) = 0;
	
	virtual bool peek( char & ch ) = 0;

	virtual void unget() = 0;
};

class InputStreamMnxSource : public MnxSource {
private:
	std::istream & src;
	std::deque< int > buffer;
	char prev_ch;
public:
	InputStreamMnxSource( std::istream & src ) : src( src ), prev_ch( '\0' ) {
		this->src >> std::noskipws;
	}
	virtual ~InputStreamMnxSource() {
	}
public:
	virtual bool hasNext();
	
	virtual bool get( char & ch );
	
	virtual bool peek( char & ch );

	virtual void unget();
};

class ProxyMnxSource : public MnxSource {
private:
	MnxSource & msrc;
public:
	ProxyMnxSource( MnxSource & _src ) : msrc( _src ) {
	}
	virtual ~ProxyMnxSource() {
	}
public:
	virtual bool hasNext();
	
	virtual bool get( char & ch );
	
	virtual bool peek( char & ch );

	virtual void unget();
};

}

#endif
