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

#include <stddef.h>

#include "folderscan.hpp"
#include "mishap.hpp"

#define PATH_SEPARATOR "/"

using namespace std;
using namespace Ginger;

FolderScan::FolderScan( string pathname ) : pathname( pathname ) {
	this->dp = NULL;
	this->dirp = opendir( pathname.c_str() );
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

bool FolderScan::nextFolderOrFile() {
	return this->nextFilteredByType( DT_DIR | DT_REG );
}

std::string FolderScan::entryName() const {
	//std::string ans;
	//ans.append( dp->d_name, dp->d_namlen );
	return std::string( dp->d_name );
}

std::string FolderScan::folderName() const {
	return this->pathname;
}

std::string FolderScan::fullPath() const {
	return this->pathname + PATH_SEPARATOR + this->entryName();
}
