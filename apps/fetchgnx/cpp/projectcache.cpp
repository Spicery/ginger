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

#include "projectcache.hpp"
#include "folderscan.hpp"

using namespace std;

#define AUTO_SUFFIX 			".auto"
#define AUTO_SUFFIX_SIZE 		sizeof( AUTO_SUFFIX )



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

PackageCache * ProjectCache::getPackageCache( string & pkg_name ) {
	return this->cache[ pkg_name ];
}

void ProjectCache::putPackageCache( string & pkg_name, PackageCache * pkg ) {
	this->cache[ pkg_name ] = pkg;
}

PackageCache * ProjectCache::cachePackage( string & pkg ) {
	//cout << "NOT FOUND" << endl;
	PackageCache * newc = new PackageCache( pkg );
	
	newc->readImports( this->project_folder + "/" + pkg + "/imports.gnx" );
	//newc->printImports();
	
	FolderScan fscan( this->project_folder + "/" + pkg );
	while ( fscan.nextFolder() ) {
		string entry = fscan.entryName();

		//	Check that -entry- matches *.auto
		if ( entry.find( AUTO_SUFFIX, entry.size() - AUTO_SUFFIX_SIZE ) == string::npos ) continue;
		#ifdef DBG_SEARCH
			cout << "*.auto: " << entry << endl;
		#endif
		
		FolderScan files( fscan.folderName() + "/" + entry );
		while ( files.nextFile() ) {
			string fname = files.entryName();
			#ifdef DBG_SEARCH
				cout << "Entry : " << fname << endl;
			#endif
			
			size_t n = fname.rfind( '.' );
			if ( n == string::npos ) continue;
			
			
			string root = fname.substr( 0, n );
			string extn = fname.substr( n + 1 );
			
			#ifdef DBG_SEARCH
				cout << "Adding " << root << " -> " << ( files.folderName() + "/" + fname ) <<  endl;
			#endif
			newc->putPathName( root, files.folderName() + "/" + fname );		
		}	
	}	
	
	this->putPackageCache( pkg, newc );
	
	return newc;
}

PackageCache * ProjectCache::fetchPackageCache( string & pkg ) {
	PackageCache * c = this->getPackageCache( pkg );
	if ( c != NULL ) return c;
	return this->cachePackage( pkg );
}

