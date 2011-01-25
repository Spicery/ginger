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

#ifndef SEARCH_HPP
#define SEARCH_HPP

#include <string>

#include "database.hpp"

#include "projectcache.hpp"
#include "packagecache.hpp"
#include "defn.hpp"

class Search {
private:
	//Ginger::SqliteDb 				sqlite_db;	
	ProjectCache					project_cache;
	
public:
	//Ginger::SqliteDb & 	sqliteDb() { return this->sqlite_db; }

private:
	bool file_exists( std::string fullpathname );
	//void printUnqualifiedDefinition( PackageCache * c, const std::string name );
	void printAbsoluteDefinition( PackageCache * c, const std::string name );
	//void printQualifiedDefinition( PackageCache * c, const std::string alias, const std::string name );
	void cacheDefinitionFile( PackageCache * c, const std::string & name, const std::string & pathname );
	//void cachePackageFile( PackageCache * c, const std::string & pathname );
	//void cachePackage( PackageCache * c );
	//void cacheUnqualifiedLookup( const std::string & pkg_cxt, const std::string & name_cxt, const std::string & pkg_defn, const std::string & name_defn );
	//void assertDefinition( std::string & pkg, const std::string & var, std::string & body );
	//void assertPackageLoad( std::string & pkg, std::string & body );
	//bool isCached( const std::string & pkg, const std::string & var );
	//bool hasLoadedPackage( std::string & pkg );
	
public:
	//bool try_serve( std::string fullname );
	//void findDefinition( const Defn & defn );

	void resolveQualified( const std::string & pkg, const std::string & alias, const std::string & vname );
	void resolveUnqualified( const std::string & pkg, const std::string & vname );
	void fetchDefinition( const std::string & pkg, const std::string & vname );
	void loadPackage( std::string & pkg );
	
public:
	//Search( std::string & sqlite_db_file, std::vector< std::string > & project_path );
	Search( std::vector< std::string > & project_path );
	~Search();
};

#endif
