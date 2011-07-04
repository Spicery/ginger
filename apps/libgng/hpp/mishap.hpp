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


#ifndef GINGER_MISHAP_HPP
#define GINGER_MISHAP_HPP

#include <string>
#include <vector>
#include <utility>
#include <stdexcept>

namespace Ginger {

class Problem : public std::exception {
protected:
	std::string message;
	std::vector< std::pair< std::string, std::string > > culprits;

protected:	
	Problem & culprit( const char * reason, const std::string arg );
	Problem & culprit( const char * reason, const char * arg );
	Problem & culprit( const std::string reason, const std::string arg );
	Problem & culprit( const std::string arg );
	Problem & culprit( const std::string reason, const long N );
	Problem & culprit( const std::string reason, const char N );
	
public:
	void setMessage( const std::string & msg ) { this->message = msg; }
	void report();
	void gnxReport();
	std::string getMessage();
	std::pair< std::string, std::string > & getCulprit( int n );
	int getCount();
	
protected:
	Problem( const std::string & msg ) : message( msg ), culprits() {}
	virtual ~Problem() throw() {}
};


//	Abstract
class Mishap : public Problem {
public:
	Mishap & culprit( const std::string reason, const std::string arg ) { this->Problem::culprit( reason, arg ); return *this; }
	Mishap & culprit( const std::string arg ) { this->Problem::culprit( arg ); return *this; }
	Mishap & culprit( const std::string arg, const long N ) { this->Problem::culprit( arg, N ); return *this; }
	Mishap & culprit( const std::string arg, const char N ) { this->Problem::culprit( arg, N ); return *this; }
	
public:
	Mishap( const std::string & msg ) : Problem( msg ) {}
	virtual ~Mishap() throw() {}
};

class SystemError : public Problem {
public:
	SystemError & culprit( const std::string reason, const std::string arg ) { this->Problem::culprit( reason, arg ); return *this; }
	SystemError & culprit( const std::string arg ) { this->Problem::culprit( arg ); return *this; }
	SystemError & culprit( const std::string arg, const long N ) { this->Problem::culprit( arg, N ); return *this; }
	SystemError & culprit( const std::string arg, const char N ) { this->Problem::culprit( arg, N ); return *this; }
	
public:
	SystemError( const std::string & msg ) : Problem( msg ) {}
	SystemError() : Problem( "System Error (see log file)" ) {}
	virtual ~SystemError()  throw() {}
};

} // namespace Ginger

#endif

