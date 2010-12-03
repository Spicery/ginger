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

#define FILE2GNX "/usr/local/bin/file2gnx"

#include <fstream>
#include <iostream>
using namespace std;

#include "packagecache.hpp"

PackageCache::PackageCache( std::string pkg_name ) : 
	package_name( pkg_name ) 
{
}

PackageCache::~PackageCache() {
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

//
//	Insecure. We need to do this more neatly. It would be best if common2gnx
//	and lisp2gnx could handle being passed a filename as an argument. This
//	would be both more secure and efficient.
//
static void run( string command, string pathname ) {
	const char * cmd = command.c_str();
	execl( cmd, cmd, pathname.c_str(), NULL );
}


static void dumpFile( string fullname ) {
	run( FILE2GNX, fullname );
	/*
	ifstream file( fullname.c_str() );
	if ( file.is_open() ) {
		//cout << "Found: " << fullname << endl;
		string line;
		while ( file.good() ) {
			getline( file, line );
			cout << line << endl;
		}
		file.close();
	} else {
		throw Mishap( "Cannot open file" ).culprit( "Filename", fullname );
	}
	*/
}

void PackageCache::printVariable( string var_name ) {
	std::map< std::string, PkgInfo >::iterator it = this->cache.find( var_name );
	if ( it == this->cache.end() ) {
		throw 
			Mishap( "Cannot find variable" ).
			culprit( "Variable", var_name ).
			culprit( "Package", this->package_name );
	} else {
		Mishap * m = it->second.mishap;
		if ( m != NULL ) {
			throw *m;
		} else {
			dumpFile( it->second.pathname );
		}
	}
}


PkgInfo::PkgInfo() : mishap( NULL ) {
}

PkgInfo::~PkgInfo() {
	delete this->mishap; 
}

