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

#include <iostream>
using namespace std;

#include "packagecache.hpp"

PackageCache::PackageCache() {
}

PackageCache::~PackageCache() {
}

std::string PackageCache::getPathName( std::string var_name ) {
	return this->cache[ var_name ];
}

void PackageCache::putPathName( std::string var_name, std::string path_name ) {
	this->cache[ var_name ] = path_name;
	cout << "PUT " << var_name << " = " << path_name << endl;
}


