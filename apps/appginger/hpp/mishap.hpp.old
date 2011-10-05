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

#ifndef MISHAP_HPP
#define MISHAP_HPP

#include <string>
#include <vector>
#include <utility>
#include <stdexcept>

#include "common.hpp"

void this_never_happens( void );

//	Abstract.
class Throwable : public std::exception {
protected:
	Throwable() {}
	virtual ~Throwable() throw() {}
};

class NormalExit : public Throwable {
public:
	NormalExit() {}
	virtual ~NormalExit() throw() {}
};

//	Abstract
class Problem : public Throwable {
private:
	std::string message;
	std::vector< std::pair< std::string, std::string > > culprits;

public:
	void setMessage( const std::string & msg ) { this->message = msg; }
	Problem & culprit( const std::string reason, const std::string arg );
	Problem & culprit( const std::string arg );
	Problem & culprit( const std::string reason, Ref ref );
	Problem & culprit( const std::string reason, const long N );
	void report();
	
protected:
	Problem( const std::string & msg ) : message( msg ), culprits() {}
	virtual ~Problem()  throw() {}
};

class SystemError : public Problem {
public:
	SystemError & culprit( const std::string reason, const std::string arg ) { this->Problem::culprit( reason, arg ); return *this; }
	SystemError & culprit( const std::string arg ) { this->Problem::culprit( arg ); return *this; }
	SystemError & culprit( const std::string arg, const long N ) { this->Problem::culprit( arg, N ); return *this; }
	SystemError( const std::string & msg ) : Problem( msg ) {}
	SystemError() : Problem( "System Error (see log file)" ) {}
	virtual ~SystemError()  throw() {}
};

class Mishap : public Problem {
public:
	Mishap & culprit( const std::string reason, const std::string arg ) { this->Problem::culprit( reason, arg ); return *this; }
	Mishap & culprit( const std::string arg ) { this->Problem::culprit( arg ); return *this; }
	Mishap & culprit( const std::string arg, const long N ) { this->Problem::culprit( arg, N ); return *this; }
	Mishap & culprit( const std::string reason, Ref ref ) { 
		this->Problem::culprit( reason, ref ); 
		return *this; 
	}
	Mishap( const std::string & msg ) : Problem( msg ) {}
	virtual ~Mishap()  throw() {}
};

class ToBeDone : public Mishap {
public:
	ToBeDone() :  Mishap( "To be done" ) {}
	virtual ~ToBeDone()  throw() {}
};

class Unreachable : public SystemError {
public:
	Unreachable( const char * file, int line );
	virtual ~Unreachable()  throw() {}
};

class ArgsMismatch : public Mishap {
public:
	ArgsMismatch() :  Mishap( "Argument mismatch (wrong number of args?)" ) {}
	virtual ~ArgsMismatch()  throw() {}
};

class TypeError : public Mishap {
public:
	TypeError( const std::string arg ) :  Mishap( arg ) {}
	TypeError() : Mishap( "Type Error" ) {}
	virtual ~TypeError()  throw() {}
};

class OutOfRange : public Mishap {
public:
	OutOfRange( const std::string arg ) :  Mishap( arg ) {}
	OutOfRange() : Mishap( "Out of Range" ) {}
	virtual ~OutOfRange()  throw() {}
};

#endif

