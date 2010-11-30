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

#ifndef SEARCH_HPP
#define SEARCH_HPP

#include <string>

#include "projectcache.hpp"

class Search {
private:
	std::string project_folder;
	ProjectCache project_cache;

private:
	bool file_exists( std::string fullpathname );
	
public:
	//bool try_serve( std::string fullname );
	void find_definition( std::string pkg, std::string name );
	
public:
	Search( std::string project_path );
	~Search();
};

#endif
