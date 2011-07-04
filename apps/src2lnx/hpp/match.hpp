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

#ifndef MATCH_SRC2LNX_HPP
#define MATCH_SRC2LNX_HPP

// STL
#include <string>

#include <ctype.h>

namespace SRC2LNX_NS {

class Match {
private:

public:
	virtual bool matches( char ch ) = 0;
};

class IncludeMatch : public Match {
private:
	const std::string inc;
public:
	bool matches( char ch ) { return this->inc.find( ch ) != std::string::npos; }
public:
	IncludeMatch( const std::string & inc ) : inc( inc ) {}
};

class AnyMatch : public Match {
public:
	bool matches( char ch ) { return true; }
};

class IsSpaceMatch : public Match {
public:
	bool matches( char ch ) { return isspace( ch ); }
};

class IsAlphaMatch : public Match {
public:
	bool matches( char ch ) { return isalpha( ch ); }
};

class IsDigitMatch : public Match {
public:
	bool matches( char ch ) { return isdigit( ch ); }
};

class IsAlnumMatch : public Match {
public:
	bool matches( char ch ) { return isalnum( ch ); }
};

class IsGraphMatch : public Match {
public:
	bool matches( char ch ) { return isgraph( ch ); }
};

class IsPunctMatch : public Match {
public:
	bool matches( char ch ) { return ispunct( ch ); }
};

class IsCntrlMatch : public Match {
public:
	bool matches( char ch ) { return iscntrl( ch ); }
};

class IsPrintMatch : public Match {
public:
	bool matches( char ch ) { return isprint( ch ); }
};

class IsLowerMatch : public Match {
public:
	bool matches( char ch ) { return islower( ch ); }
};

class IsUpperMatch : public Match {
public:
	bool matches( char ch ) { return isupper( ch ); }
};



} // namespace

#endif
