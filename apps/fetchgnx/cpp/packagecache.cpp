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

#include <stddef.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "packagecache.hpp"
//#include "sax.hpp"

using namespace std;

PackageCache::PackageCache( ProjectCache * project, std::string pkg_name ) : 
	project( project ),
	package_name( pkg_name ) 
{
}

PackageCache::~PackageCache() {
}

/*
	GOT HERE - CONSIDERING HOW TO COPE WITH RELOADING. Repeats could
	cause the cached imports to grow without limit, unless there is a good
	composite primary key.
		from
		match*
		into*
	This rather suggests that match and into should not be tag sets but
	single tags. That means that you have to be able to support multiple
	imports sharing the same alias identifer.
	FetchGNX will indeed permit exactly that, because the imports are not
	amalgamated. And tags are blown away by appginger, as definitions come
	from outside (usually) (Which raises the issue of definitions that are
	made interactively. What tags do they end up having? The simple answer
	is none, although that is not entirely satisfactory.
	
*/
void PackageCache::readImports( string ifile ) {
	this->imports.readFile( ifile );
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

/*
	Performs an absolute search for the variable in this
	package. Note that this should use pervasive (or included)
	imports (but doesn't yet).
*/
VarInfo * PackageCache::absoluteVarInfo( const string & var_name ) {
	VarInfo * v = this->absoluteVarInfo( var_name, NULL );
	if ( v == NULL ) {
		//	Is it already pre-populated in the database? If so we 
		//	will bring it into the local cache and set v.
		
	}
	return v;
}

/*
	NULL match means unrestricted.
*/
VarInfo * PackageCache::absoluteVarInfo( const string & var_name, const FacetSet * match ) {
	std::map< std::string, VarInfo >::iterator it = this->cache.find( var_name );
	if ( it == this->cache.end() ) {
		//	We should not give up just yet. We should search all included 
		//	(pervasively imported) packages whose "into" tags overlap with
		//	the "match" tags.
		
		//cout << "Looking for inclusions into " << this->package_name << endl;
		
		VarInfo * var_info = NULL;
		const FacetSet * tags = NULL;
		string from;
		std::vector< ImportInfo > & iv = importVector();
		for ( 
			vector< ImportInfo >::iterator it = iv.begin();
			it != iv.end();
			++it
		) {
			if ( it->intoTags()->isEmpty() ) continue;
			if ( match != NULL && it->intoTags()->isEmptyIntersection( match ) ) continue;
			//cout << "Found inclusion " << it->getFrom() << endl;
			
			PackageCache * c = this->project->fetchPackageCache( it->getFrom() );
			VarInfo * v = c->absoluteVarInfo( var_name, it->matchTags() );
			
			//cout << "  Matched? " << ( v == NULL ? "no" : "yes" ) << endl;	
			if ( v != NULL ) {
				if ( var_info != NULL ) {
					throw (
						Mishap( "Ambiguous inclusion" ).
						culprit( "Package", this->package_name ).
						culprit( "Variable", var_name ).
						culprit( "Import 1", from ).
						culprit( "Import 2", it->getFrom() )
					);
				}
				//cout << "  Setting var_info" << endl;
				var_info = v;
				from = it->getFrom();
				tags = it->intoTags();
			}
		}
		if ( var_info == NULL ) {
			return NULL;
		} else {
			VarInfo * ans = &this->cache[ var_name ];
			ans->init( var_info );
			ans->setTags( tags );
			ans->freeze();
			return ans;
		}
	} else if ( match !=NULL && !match->isntEmptyIntersection( it->second.varInfoTags() ) ) {	//	Check that the tag is contained in the "match" tags.			
		//	Not part of the export. Do NOT search the pervasive imports, except
		//	for detecting errors (i.e. to detect clashes with protected includes).
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

/*VarInfo * PackageCache::varInfo( const string & vname ) {
	return & this->cache[ vname ];
}*/

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
