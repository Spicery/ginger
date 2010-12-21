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

#ifndef IMPORT_INFO_HPP
#define IMPORT_INFO_HPP

#include <string>
#include <map>

#include "facet.hpp"

using namespace Ginger;

class ImportInfo {
private:
	std::map< std::string, std::string > attrs;
	std::string from;
	const FacetSet * into_tags;
	const FacetSet * match_tags;
	
public:
	void printInfo();
	const std::string & getFrom();
	bool matches( const std::string & tag );
	const FacetSet * matchTags() const { return this->match_tags; }
	const FacetSet * intoTags() const { return this->into_tags; }
	
public:
	ImportInfo( std::map< std::string, std::string > & attrs );
};


#endif
