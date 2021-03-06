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
#include <sstream>

#include <stddef.h>

#include "projectcache.hpp"
#include "folderscan.hpp"

#include "fileutils.hpp"

using namespace std;

#define AUTO_SUFFIX 			".auto"
#define AUTO_SUFFIX_SIZE 		sizeof( AUTO_SUFFIX )

#define LOAD					"load"
#define LOAD_SIZE				sizeof( LOAD )

//#define DBG_SEARCH

ProjectCache::ProjectCache( Search * parent, vector< string > & project_paths ) :
	//parent( parent ),
	project_folders( project_paths )
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

PackageCache * ProjectCache::getPackageCache( const string & pkg_name ) {
	return this->cache[ pkg_name ];
}

PackageCache * ProjectCache::putPackageCache( const string & pkg_name, PackageCache * pkg ) {
	return this->cache[ pkg_name ] = pkg;
}

enum STATE { NORMAL, PERCENT1, PERCENT2 };

static int unhex( char ch ) {
	if ( '0' <= ch && ch <= '9' ) {
		return ch - '0';
	} else if ( 'A' <= ch && ch <= 'F' ) {
		return 10 + ch - 'A';
	} else if ( 'a' <= ch && ch <= 'f' ) {
		return 10 + ch - 'a';
	} else {
		stringstream character;
		character << ch;
		throw Mishap( "Invalid hex character" ).culprit( "Character", character.str() );
	}
}

/*
	Reverse the HTML escape process that is used to escape autoloadable
	file names.
	
	WARNING!!!!!! We have to decode the root, otherwise the
	encoded names (such as for "+" and "++" will not work)					
*/
static string URLdecode( string name ) {
	//cout << "URLdecoding: " << name << endl;
	stringstream sofar;
	enum STATE state = NORMAL;
	int n;
	for ( 
		string::iterator it = name.begin();
		it != name.end();
		++it
	) {
		char ch = *it;
		//cerr << "State = " << state << endl;
		switch ( state ) {
			case NORMAL: {
				if ( ch == '%' ) {
					//cerr << "Switching state" << endl;
					state = PERCENT1;
				} else {
					sofar << ch;
				}
				break;
			}
			case PERCENT1: {
				n = unhex( ch );
				state = PERCENT2;
				break;
			}
			case PERCENT2: {
				sofar << (char)( ( n << 4 ) | unhex( ch ) );
				state = NORMAL;
				break;
			}
		}
	}
	//cout << "Decoding " << name << " into " << sofar.str() << endl;
	return sofar.str();
}

PackageCache * ProjectCache::createRawPackageCache( const string & pkg_folder, const string & pkg_name ) {
	PackageCache * newc = new PackageCache( this, pkg_name );

	#ifdef DBG_SEARCH
		cerr << "package: " << pkg_name << endl;
	#endif

	newc->readImports( pkg_folder + "/imports.gnx" );
	//newc->printImports();

	FolderScan fscan( pkg_folder );
	while ( fscan.nextFolder() ) {
		string entry = fscan.entryName();
		
		#ifdef DBG_SEARCH
			cerr << "subfolder: " << entry << endl;
		#endif


		//	Check that -entry- matches *.auto
		if ( entry.find( AUTO_SUFFIX, entry.size() - AUTO_SUFFIX_SIZE ) != string::npos ) {
		
			const string default_tag = entry.substr( 0, entry.size() + 1 - AUTO_SUFFIX_SIZE );
			//cout << "TAG = " << default_tag << endl;
		
			#ifdef DBG_SEARCH
				cerr << "*.auto: " << entry << endl;
			#endif
			
			FolderScan files( fscan.folderName() + "/" + entry );
			while ( files.nextFile() ) {
				string fname = files.entryName();
				#ifdef DBG_SEARCH
					cerr << "Entry : " << fname << endl;
				#endif
				
				size_t n = fname.rfind( '.' );
				if ( n == string::npos ) continue;
				
				const string root = URLdecode( fname.substr( 0, n ) );
				const string extn = fname.substr( n + 1 );
				
				#ifdef DBG_SEARCH
					cerr << "Adding " << root << " -> " << ( files.folderName() + "/" + fname ) <<  endl;
				#endif
				
				
				VarInfo & v = newc->varInfoRef( root );
				v.init( root, files.folderName() + "/" + fname );
				v.addTag( default_tag );
				//cout << "old path name " << v->getPathName() << endl;
				v.freeze();					
				//cout << "new path name 1 = " << newc->varInfo( root )->getPathName() << endl;
				//cout << "new path name 2 = " << v->getPathName() << endl;
				//newc->putPathName( root, files.folderName() + "/" + fname );		
			}	
		} else if ( entry == "load" ) {
			//	It doesn't match *.auto but it is a load folder though.
			const string p( fscan.folderName() + "/" + entry );
			newc->setLoadFolder( p );
		}
	}	
	return newc;
}

PackageCache * ProjectCache::registerPackageCache( PackageCache * pkgc ) {
	return this->putPackageCache( pkgc->getPackageName(), pkgc );
}

PackageCache * ProjectCache::createRegisteredPackageCache( const string & prj_folder, const string & pkg_name ) {
	return this->registerPackageCache( this->createRawPackageCache( prj_folder, pkg_name ) );
}

PackageCache * ProjectCache::cachePackage( const string & pkg ) {
	for (
		vector< string >::iterator it = this->project_folders.begin();
		it != this->project_folders.end();
		++it
	) {
		//cout << "NOT FOUND" << endl;
		string pkg_folder = *it + "/" + pkg;
		
		if ( Ginger::fileExists( pkg_folder ) ) {
			return this->createRegisteredPackageCache( pkg_folder, pkg );
		}
	}
	throw Mishap( "Cannot find package" ).culprit( "Package", pkg );
}

PackageCache * ProjectCache::fetchPackageCache( const string & pkg ) {
	PackageCache * c = this->getPackageCache( pkg );
	if ( c != NULL ) return c;
	return this->cachePackage( pkg );
}

std::vector< PackageCache * > ProjectCache::allPackageCaches() {
	std::vector< PackageCache * > pkgs;
	for (
		vector< string >::iterator it = this->project_folders.begin();
		it != this->project_folders.end();
		++it
	) {
		const string prj_folder = *it;
		FolderScan prjscan( prj_folder );
		while ( prjscan.nextFolder() ) {
			const string pkg_name = prjscan.entryName();
			const string pkg_folder = prj_folder + "/" + pkg_name;
			if ( pkg_name == "." || pkg_name == ".." ) continue;
			pkgs.push_back( this->createRegisteredPackageCache( pkg_folder, pkg_name ) ); 
		}
	}	
	return pkgs;
}
