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

#ifndef PACKAGE_CACHE_HPP
#define PACKAGE_CACHE_HPP

#include <string>
#include <map>

//	A mapping from variable names to file names.
class PackageCache {
private:
	std::string package_name;
	std::map< std::string, std::string > cache;
	
public:
	std::string getPathName( std::string name );
	void putPathName( std::string name, std::string pathname );

public:
	PackageCache();
	~PackageCache();
};

#endif
