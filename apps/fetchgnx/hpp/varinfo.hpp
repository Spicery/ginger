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

#ifndef VAR_INFO_HPP
#define VAR_INFO_HPP

#include <string>
#include <map>
#include <memory>
#include <set>

#include "mishap.hpp"

class VarInfo {
public:
	bool						frozen;
	
private:
	std::string		 			var_name;
	std::set< std::string >		tag_set;
	std::string 				pathname;
	
public:
	Mishap *					mishap;
	
public:
	const std::string & getPathName();
	void init( const std::string & vname, const std::string & pathname );
	void freeze();
	void addTag( const std::string & tag );
	std::set< std::string > & 	tagSet();
	
public:
	VarInfo();
	~VarInfo();
};

#endif
