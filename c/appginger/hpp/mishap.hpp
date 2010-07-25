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


//#include <setjmp.h>
#include "common.hpp"

void this_never_happens( void );
void to_be_done( charseq msg );
//void mishap( charseq msg, ... );
//void reset( charseq msg, ... );
//void warning( charseq msg, ... );

//extern jmp_buf mishap_jump_buffer;

//	Abstract.
class Throwable {
protected:
	Throwable() {}
};

class NormalExit : public Throwable {
public:
	NormalExit() {}
};

//	Abstract
class Problem : public Throwable {
private:
	std::string message;
	std::vector< std::pair< std::string, std::string > > culprits;

public:
	Problem & culprit( const std::string reason, const std::string arg );
	Problem & culprit( const std::string arg );
	void report();
	
protected:
	Problem( const std::string & msg ) : message( msg ) {}
};

class SystemError : public Problem {
public:
	SystemError & culprit( const std::string reason, const std::string arg ) { this->Problem::culprit( reason, arg ); return *this; }
	SystemError & culprit( const std::string arg ) { this->Problem::culprit( arg ); return *this; }
	SystemError( const std::string & msg ) : Problem( msg ) {}
};

class Mishap : public Problem {
public:
	Mishap & culprit( const std::string reason, const std::string arg ) { this->Problem::culprit( reason, arg ); return *this; }
	Mishap & culprit( const std::string arg ) { this->Problem::culprit( arg ); return *this; }
	Mishap( const std::string & msg ) : Problem( msg ) {}
};

class ToBeDone : public Mishap {
public:
	ToBeDone() :  Mishap( "To be done" ) {}
};

class Unreachable : public SystemError {
public:
	Unreachable() : SystemError( "Unreachable" ) {}
};


#endif
