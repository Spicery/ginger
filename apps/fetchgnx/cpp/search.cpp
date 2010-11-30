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
using namespace std;

#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>

#include "search.hpp"
#include "mishap.hpp"
#include "folderscan.hpp"


#define AUTO_SUFFIX 			".auto"
#define AUTO_SUFFIX_SIZE 		sizeof( AUTO_SUFFIX )

#define GNX_SUFFIX 				".gnx"
#define GNX_SUFFIX_SIZE 		sizeof( GNX_SUFFIX )

Search::Search( std::string project_folder ) :
	project_folder( project_folder ),
	project_cache( project_folder )
{
}

Search::~Search() {
}


/* bool Search::file_exists( std::string fullpathname ) {
	static struct stat stat_file_info;
	return 0 == stat( fullpathname.c_str(), &stat_file_info );
}

static void dumpFile( string fullname ) {
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
}

bool Search::try_serve( string fullname ) {
	if ( !this->file_exists( fullname ) ) return false;
	dumpFile( fullname );
	return true;
}*/


void Search::find_definition( string pkg, string name ) {
	PackageCache * c = this->project_cache.getPackageCache( pkg );
	if ( c != NULL ) {
		//cout << "FOUND" << endl;
		c->printVariable( name );
	} else {
		//cout << "NOT FOUND" << endl;
		PackageCache * newc = new PackageCache( pkg );
		this->project_cache.putPackageCache( pkg, newc );
		
		FolderScan fscan( this->project_folder + "/" + pkg );
		while ( fscan.nextFolder() ) {
			string entry = fscan.entryName();
	
			//	Check that -entry- matches *.auto
			if ( entry.find( AUTO_SUFFIX, entry.size() - AUTO_SUFFIX_SIZE ) == string::npos ) continue;
			
			string fullname = fscan.folderName() + "/" + entry + "/" + name + ".gnx";
			
			FolderScan files( fscan.folderName() + "/" + entry );
			while ( files.nextFile() ) {
				string fname = files.entryName();
				if ( fname.find( GNX_SUFFIX, fname.size() - GNX_SUFFIX_SIZE ) == string::npos ) continue;
				
				size_t n = fname.rfind( '.' );
				if ( n == string::npos ) continue;
				
				string root = fname.substr( 0, n );
				string extn = fname.substr( n + 1 );
				
				newc->putPathName( root, files.folderName() + "/" + fname );		
			}	
		}
		
		newc->printVariable( name );
	}
}
