/******************************************************************************\
	Copyright (c) 2010 Stephen Leach. AppGinger is distributed under the terms 
	of the GNU General Public License. This file is part of Ginger.

    Ginger is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Ginger is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Ginger.  If not, see <http://www.gnu.org/licenses/>.
\******************************************************************************/

#ifndef PACKAGE_CACHE_HPP
#define PACKAGE_CACHE_HPP

#include <string>
#include <map>
#include <list>
#include <memory>

#include "mishap.hpp"
#include "varinfo.hpp"
#include "importsetinfo.hpp"
#include "projectcache.hpp"

class ProjectCache;

//	A mapping from variable names to file names.
class PackageCache {
private:
	ProjectCache *						project;
	std::string 						package_name;
	std::string							init_load_file;
	std::string							load_folder_path;
	std::map< std::string, VarInfo > 	cache;
	ImportSetInfo 						imports;
	
public:
	std::string getPackageName();
	std::vector< VarInfo * > allVarInfo();
	bool hasVariable( std::string var_name );
	std::string getPathName( std::string name );
	VarInfo * absoluteVarInfo( const std::string & var_name );	//	Any facet.
	VarInfo * absoluteVarInfo( const std::string & var_name, const FacetSet * match );
	VarInfo & varInfoRef( const std::string & var_name );
	void readImports( std::string import_file );
	void printImports();
	//void fillFromList( std::vector< std::string > & from_list );
	void setInitLoadPath( const std::string path );
	std::string getInitLoadPath();
	std::string getLoadFolder();
	void setLoadFolder( const std::string path );
	std::vector< ImportInfo > & importVector();

public:
	PackageCache( ProjectCache * project, std::string pkg_name );
	~PackageCache();
};

#endif
