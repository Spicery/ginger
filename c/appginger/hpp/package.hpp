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

#ifndef PACKAGE_HPP
#define PACKAGE_HPP

#include <map>
#include <string>
#include <vector>
#include "dict.hpp"
#include "ident.hpp"

class Package;

class Import {
friend class Package;
private:
	const Facet *     	facet;			//	Never NULL.
	Package *			from;			//	Never NULL.
	std::string			alias;
	bool				is_protected;
	const Facet * 		into;			//	NULL = not into, the usual case.

public:
	Import(
		const Facet *     	facet,
		Package *			from,
		std::string			alias,
		bool				is_protected,
		const Facet * 		into
	) :
		facet( facet ),
		from( from ),
		alias( alias ),
		is_protected( is_protected ),
		into( into )
	{
	}

};

class PackageManager {
public:		//	Will need to make this private.
	MachineClass * vm;
	std::map< std::string, class Package * > packages;
	
public:
	Package * getPackage( std::string title );
	
public:
	PackageManager( MachineClass * vm );
};

class Package {
friend class GarbageCollect;	//	I would like to get rid of this soon.
protected:
	PackageManager * 		pkgmgr;
	const std::string		title;
	DictClass				dict;
	std::vector< Import >	imports;

	virtual Ident autoload( const std::string & c ) = 0;
	
public:
	void import( const Import & imp );		
	
public:
	Package * getPackage( const std::string title );
	Ident exported( const std::string & c, const Facet * facet );
	Ident lookup( const std::string & c );
	Ident add( const std::string & c, const Facet * facet );
	Ident lookup_or_add( const std::string & c, const Facet * facet );
	
public:
	Package( PackageManager * pkgmgr, const std::string title ) :
		pkgmgr( pkgmgr ),
		title( title )
	{
	}
	
	~Package() {}
};

class OrdinaryPackage : public Package {
protected:
	virtual Ident autoload( const std::string & c );
public:
	OrdinaryPackage( PackageManager * pkgmgr, const std::string title );
	~OrdinaryPackage() {}
};

class StandardLibraryPackage : public Package {
protected:
	virtual Ident autoload( const std::string & c );
public:
	StandardLibraryPackage( PackageManager * pkgmgr, std::string title ) :
		Package( pkgmgr, title )
	{
	}
	~StandardLibraryPackage() {}
};


#endif