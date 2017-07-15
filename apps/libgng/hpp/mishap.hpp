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


#ifndef GINGER_MISHAP_HPP
#define GINGER_MISHAP_HPP

#include <string>
#include <vector>
#include <utility>
#include <stdexcept>

namespace Ginger {

//	Should refactor NormalExit into a separate file.
class NormalExit : public std::exception {
public:
	NormalExit() {}
	virtual ~NormalExit() throw() {}
};

class Mishap : public std::exception {
private:
	std::string message;
	std::vector< std::pair< std::string, std::string > > culprits;
	std::string mishap_category;

public:
	static constexpr const char * const SystemCategory = "S";
	static constexpr const char * const RunTimeCategory = "R";
	static constexpr const char * const CompileTimeCategory = "C";
	static constexpr const char * const UnexpectedEndOfInputCategory = "CE";
	
	Mishap( const std::string & msg, const std::string & category = RunTimeCategory ) : 
		message( msg ),
		mishap_category( category )
	{}
	virtual ~Mishap() throw() {}


public:	
	Mishap & culprit( const char * reason, const std::string arg );
	Mishap & culprit( const char * reason, const char * arg );
	Mishap & culprit( const std::string reason, const std::string arg );
	Mishap & culprit( const std::string arg );
	Mishap & culprit( const std::string reason, const unsigned long N );
	Mishap & culprit( const std::string reason, const signed long N );
	Mishap & culprit( const std::string reason, const unsigned int N );
	Mishap & culprit( const std::string reason, const signed int N );
	Mishap & culprit( const std::string reason, const double N );
	Mishap & culprit( const std::string reason, const signed char N );
	Mishap & culprit( const std::string reason, const unsigned char N );

	Mishap & hint( const std::string & hint );
	Mishap & cause( Mishap & problem );
	
public:
	void setMessage( const std::string & msg ) { this->message = msg; }
	virtual std::string category() const; 
	bool isCompileTimeError() { return this->mishap_category[0] == 'C'; }
	bool isExecutionTimeError() { return this->mishap_category[0] == 'R'; }
	bool isSystemError() { return this->mishap_category[0] == 'S'; }

	void report();
	void gnxReport();
	
	std::string getMessage();
	std::pair< std::string, std::string > & getCulprit( int n );
	int getCount();
	
	const char * what() const throw() { return this->message.c_str(); }
};

#define SystemError( Message ) Ginger::Mishap( (Message), Ginger::Mishap::SystemCategory ).culprit( "FILE", __FILE__ ).culprit( "LINE", (long)__LINE__ )
#define CompileTimeError( Message ) Ginger::Mishap( (Message), Ginger::Mishap::CompileTimeCategory )
#define UnexpectedEndOfInputError() Ginger::Mishap( "Unexpected end of input", Ginger::Mishap::UnexpectedEndOfInputCategory )
#define UnreachableError() Ginger::Mishap( "Internal error", Ginger::Mishap::SystemCategory  ).culprit( "FILE", __FILE__ ).culprit( "LINE", (long)__LINE__ )
#define ToBeDoneError() Ginger::Mishap( "To be implemented", Ginger::Mishap::SystemCategory  ).culprit( "FILE", __FILE__ ).culprit( "LINE", (long)__LINE__ )

} // namespace Ginger

#endif

