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


//	Abstract.
class Throwable {
protected:
	Throwable() {}
	virtual ~Throwable() {}
};

class NormalExit : public Throwable {
public:
	NormalExit() {}
	virtual ~NormalExit() {}
};

//	Abstract
class Problem : public Throwable {
private:
	std::string message;
	std::vector< std::pair< std::string, std::string > > culprits;

public:
	Problem & culprit( const std::string reason, const std::string arg );
	Problem & culprit( const std::string arg );
	Problem & culprit( const std::string reason, const long N );
	void report();
	void gnxReport();
	std::string getMessage();
	std::pair< std::string, std::string > & getCulprit( int n );
	int getCount();
	
protected:
	Problem( const std::string & msg ) : message( msg ), culprits() {}
	virtual ~Problem() {}
};

class SystemError : public Problem {
public:
	SystemError & culprit( const std::string reason, const std::string arg ) { this->Problem::culprit( reason, arg ); return *this; }
	SystemError & culprit( const std::string arg ) { this->Problem::culprit( arg ); return *this; }
	SystemError( const std::string & msg ) : Problem( msg ) {}
	virtual ~SystemError() {}
};

class Mishap : public Problem {
public:
	Mishap & culprit( const std::string reason, const std::string arg ) { this->Problem::culprit( reason, arg ); return *this; }
	Mishap & culprit( const std::string arg ) { this->Problem::culprit( arg ); return *this; }
	Mishap & culprit( const std::string arg, const long N ) { this->Problem::culprit( arg, N ); return *this; }
	Mishap( const std::string & msg ) : Problem( msg ) {}
	virtual ~Mishap() {}
};

class ToBeDone : public Mishap {
public:
	ToBeDone() :  Mishap( "To be done" ) {}
	virtual ~ToBeDone() {}
};

class Unreachable : public SystemError {
public:
	Unreachable() : SystemError( "Unreachable" ) {}
	virtual ~Unreachable() {}
};

class ArgsMismatch : public Mishap {
public:
	ArgsMismatch() :  Mishap( "Argument mismatch (wrong number of args?)" ) {}
	virtual ~ArgsMismatch() {}
};

class TypeError : public Mishap {
public:
	TypeError( const std::string arg ) :  Mishap( arg ) {}
	TypeError() : Mishap( "Type Error" ) {}
	virtual ~TypeError() {}
};

class OutOfRange : public Mishap {
public:
	OutOfRange( const std::string arg ) :  Mishap( arg ) {}
	OutOfRange() : Mishap( "Out of Range" ) {}
	virtual ~OutOfRange() {}
};

#endif

