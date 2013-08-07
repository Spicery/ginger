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

//	Should refactor NormalExit into a separate file.
class NormalExit : public std::exception {
public:
	NormalExit() {}
	virtual ~NormalExit() throw() {}
};

class Mishap : public std::exception {
public:
	enum SEVERITY {
		EXECUTION_TIME_SEVERITY,
		SYSTEM_ERROR_SEVERITY,
		COMPILE_TIME_SEVERITY
	};
	static enum SEVERITY codeToSeverity( char s ) { return s == 'r' ? EXECUTION_TIME_SEVERITY : s == 'c' ? COMPILE_TIME_SEVERITY : SYSTEM_ERROR_SEVERITY; }
	static enum SEVERITY codeToSeverity( const char * s ) { return s == NULL ? SYSTEM_ERROR_SEVERITY : s[ 0 ] == 'r' ? EXECUTION_TIME_SEVERITY : s[0] == 'c' ? COMPILE_TIME_SEVERITY : SYSTEM_ERROR_SEVERITY; }
	static char severityToCode( enum SEVERITY s ) { return s == EXECUTION_TIME_SEVERITY ? 'r' : s == COMPILE_TIME_SEVERITY ? 'c' : 's'; }
private:
	std::string message;
	std::vector< std::pair< std::string, std::string > > culprits;
	enum SEVERITY mishap_severity;

public:
	Mishap( const std::string & msg, enum SEVERITY severity = EXECUTION_TIME_SEVERITY ) : 
		message( msg ),
		mishap_severity( severity )
	{}
	virtual ~Mishap() throw() {}


public:	
	Mishap & culprit( const char * reason, const std::string arg );
	Mishap & culprit( const char * reason, const char * arg );
	Mishap & culprit( const std::string reason, const std::string arg );
	Mishap & culprit( const std::string arg );
	Mishap & culprit( const std::string reason, const long N );
	Mishap & culprit( const std::string reason, const double N );
	Mishap & culprit( const std::string reason, const char N );

	Mishap & cause( Mishap & problem );
	
public:
	void setMessage( const std::string & msg ) { this->message = msg; }
	virtual std::string severity() const; 
	bool isCompileTimeError() { return this->mishap_severity == COMPILE_TIME_SEVERITY; }
	bool isExecutionTimeError() { return this->mishap_severity == EXECUTION_TIME_SEVERITY; }
	bool isSystemError() { return this->mishap_severity == SYSTEM_ERROR_SEVERITY; }

	void report();
	void gnxReport();
	
	std::string getMessage();
	std::pair< std::string, std::string > & getCulprit( int n );
	int getCount();
	
	const char * what() const throw() { return this->message.c_str(); }
};

#define SystemError( Message ) Ginger::Mishap( (Message), Ginger::Mishap::SYSTEM_ERROR_SEVERITY )
#define CompileTimeError( Message ) Ginger::Mishap( (Message), Ginger::Mishap::COMPILE_TIME_SEVERITY )
#define Unreachable() Ginger::Mishap( "Internal error", Ginger::Mishap::SYSTEM_ERROR_SEVERITY ).culprit( "FILE", __FILE__ ).culprit( "LINE", (long)__LINE__ )

} // namespace Ginger

#endif

