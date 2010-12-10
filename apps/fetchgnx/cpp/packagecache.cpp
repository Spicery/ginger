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

#include <fstream>
#include <iostream>
#include <cstdlib>

#include <sys/types.h>
#include <sys/wait.h>

#include "packagecache.hpp"
#include "sax.hpp"

using namespace std;

PackageCache::PackageCache( std::string pkg_name ) : 
	package_name( pkg_name ) 
{
}

PackageCache::~PackageCache() {
}

void PackageCache::readImports( string ifile ) {
	this->imports.readFile( ifile );
}

void PackageCache::fillFromList( list< string > & from_list ) {
	this->imports.fillFromList( from_list );
}


string PackageCache::getPackageName() {
	return package_name;
}

bool PackageCache::hasVariable( std::string var_name ) {
	return this->cache.find( var_name ) != this->cache.end();
}

std::string PackageCache::getPathName( std::string var_name ) {
	return this->cache[ var_name ].pathname;
}

void PackageCache::putPathName( std::string var_name, std::string path_name ) {
	if ( this->cache.find( var_name ) == this->cache.end() ) {
		this->cache[ var_name ].pathname = path_name;
	} else if ( this->cache[ var_name ].mishap == NULL ) {
		Mishap * m = new Mishap( "Multiple possible files providing definition" );
		m->culprit( "Variable", var_name );
		m->culprit( "Pathname 1", this->cache[ var_name ].pathname );
		m->culprit( "Pathname 2", path_name );
		this->cache[ var_name ].mishap = m;
	}
	//cout << "PUT " << var_name << " = " << path_name << endl;
}

VarInfo * PackageCache::variableFile( string var_name ) {
	std::map< std::string, VarInfo >::iterator it = this->cache.find( var_name );
	if ( it == this->cache.end() ) {
		return NULL;
	} else {
		Mishap * m = it->second.mishap;
		if ( m != NULL ) {
			throw *m;
		} else {
			return & it->second;
		}
	}
}

void PackageCache::printImports() {
	this->imports.printImports();
}
