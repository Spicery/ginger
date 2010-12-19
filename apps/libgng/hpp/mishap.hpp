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

namespace Ginger {

//	Abstract
class Mishap {
private:
	std::string message;
	std::vector< std::pair< std::string, std::string > > culprits;

public:
	Mishap & culprit( const std::string reason, const std::string arg );
	Mishap & culprit( const char * reason, const std::string arg );
	Mishap & culprit( const char * reason, const char * arg );
	Mishap & culprit( const std::string arg );
	Mishap & culprit( const std::string reason, const long N );
	void report();
	void gnxReport();
	std::string getMessage();
	std::pair< std::string, std::string > & getCulprit( int n );
	int getCount();
	
public:
	Mishap( const std::string & msg ) : message( msg ), culprits() {}
	virtual ~Mishap() {}
};

}

#endif

