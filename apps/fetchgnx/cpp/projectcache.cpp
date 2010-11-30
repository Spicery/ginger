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

using namespace std;

#include "projectcache.hpp"

ProjectCache::ProjectCache( std::string project_path ) :
	project_folder( project_path )
{
}

ProjectCache::~ProjectCache() {
	for (
		map< string, PackageCache * >::iterator it = this->cache.begin();
		it != this->cache.end();
		++it
	) {
		delete it->second;
	}
}

PackageCache * ProjectCache::getPackageCache( string pkg_name ) {
	return this->cache[ pkg_name ];
}

void ProjectCache::putPackageCache( string pkg_name, PackageCache * pkg ) {
	this->cache[ pkg_name ] = pkg;
}
