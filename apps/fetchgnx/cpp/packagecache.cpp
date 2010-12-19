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

void PackageCache::fillFromList( vector< string > & from_list ) {
	this->imports.fillFromList( from_list );
}


string PackageCache::getPackageName() {
	return package_name;
}

bool PackageCache::hasVariable( std::string var_name ) {
	return this->cache.find( var_name ) != this->cache.end();
}

std::string PackageCache::getPathName( std::string var_name ) {
	return this->cache[ var_name ].getPathName();
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

VarInfo * PackageCache::varInfo( const string & vname ) {
	return & this->cache[ vname ];
}

VarInfo & PackageCache::varInfoRef( const string & vname ) {
	return this->cache[ vname ];
}

void PackageCache::printImports() {
	this->imports.printImports();
}

void PackageCache::setLoadPath( const string & path ) {
	this->load_path = path;
}

string PackageCache::getLoadPath() {
	return this->load_path;
}

vector< ImportInfo > & PackageCache::importVector() {
	return this->imports.importVector();
}
