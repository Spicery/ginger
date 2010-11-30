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

#include "folderscan.hpp"
#include "mishap.hpp"



FolderScan::FolderScan( string pathname ) : pathname( pathname ) {
	this->dp = NULL;
	this->dirp = opendir( pathname .c_str() );
	if ( this->dirp == NULL ) throw Mishap( "Cannot open directory" ).culprit( "Pathname", pathname );
}

FolderScan::~FolderScan() {
	closedir( this->dirp );
}

bool FolderScan::nextFilteredByType( __uint8_t flags ) {
	for (;;) {
		this->dp = readdir( dirp );
		if ( this->dp == NULL ) return false;
		if ( ( dp->d_type & flags ) != 0 ) return true;
	}
}

bool FolderScan::nextFile() {
	return this->nextFilteredByType( DT_REG );
}

bool FolderScan::nextFolder() {
	return this->nextFilteredByType( DT_DIR );
}

std::string FolderScan::entryName() {
	//std::string ans;
	//ans.append( dp->d_name, dp->d_namlen );
	return std::string( dp->d_name );
}

std::string FolderScan::folderName() {
	return this->pathname;
}
