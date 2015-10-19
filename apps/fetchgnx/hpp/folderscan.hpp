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

#ifndef FOLDER_SCAN_HPP
#define FOLDER_SCAN_HPP

#include <string>

#include <sys/types.h>
#include <dirent.h>


class FolderScan {
private:
	std::string pathname;
	DIR * dirp;
	struct dirent * dp;

public:
	bool nextFilteredByType( __uint8_t flags );
	bool nextFile();
	bool nextFolder();
	bool nextFolderOrFile();
	
	std::string entryName() const;
	std::string folderName() const;
	std::string fullPath() const;

public:
	FolderScan( std::string );
	~FolderScan();

};

#endif
