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

#ifndef PROJECT_CACHE_HPP
#define PROJECT_CACHE_HPP

#include <string>

#include "packagecache.hpp"

class ProjectCache {
private:
	std::string project_folder;
	std::map< std::string, PackageCache * > cache;
	
public:
	PackageCache * getPackageCache( std::string pkg_name );
	void putPackageCache( std::string pkg_name, PackageCache * cache );


public:
	ProjectCache( std::string project_folder );
	~ProjectCache();
};

#endif
