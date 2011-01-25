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
#include <utility>
#include "dict.hpp"
#include "ident.hpp"

class Package;



class PackageManager {
public:		//	Will need to make this private.
	MachineClass * vm;
	std::map< std::string, class Package * > packages;
	
public:
	Package * getPackage( std::string title );
	void reset();
	
public:
	PackageManager( MachineClass * vm );
};

class Package {
friend class GarbageCollect;	//	I would like to get rid of this soon.
protected:
	PackageManager * 					pkgmgr;
	const std::string					title;
	DictClass							dict;

public:	//	Should be private.
	std::map< std::string, Package * > 	unqualifiedResolutions;
	std::map< std::pair< std::string, std::string >, Package * > 	qualifiedResolutions;
	
public:
	virtual void loadIfNeeded() = 0;

public:
	MachineClass * getMachine() { return this->pkgmgr->vm; }
	Package * getPackage( const std::string title );
	Ident add( const std::string & c ); //, const FacetSet * facets );
	Valof * valof( const std::string & c );
	Ident forwardDeclare( const std::string & c );
	void retractForwardDeclare( const std::string & c );
	const std::string & getTitle() { return this->title; }
	
public:
	void reset() { this->dict.reset(); }
	Ident fetchDefinitionIdent( const std::string & c ); //, const FacetSet * facets );
	Ident fetchUnqualifiedIdent( const std::string & c );
	Ident fetchQualifiedIdent( const std::string & alias, const std::string & c );
	Ident fetchAbsoluteIdent( const std::string & c );

protected:	
	virtual Ident absoluteAutoload( const std::string & c ) = 0;
	virtual Ident unqualifiedAutoload( const std::string & c ) = 0;
	virtual Ident qualifiedAutoload( const std::string & alias, const std::string & c ) = 0;

public:
	Package( PackageManager * pkgmgr, const std::string title ) :
		pkgmgr( pkgmgr ),
		title( title ),
		dict()
	{
	}
	
	virtual ~Package();
};

class OrdinaryPackage : public Package {
private:
	bool loaded;

public:
	void loadIfNeeded();
protected:
	virtual Ident absoluteAutoload( const std::string & c );
	virtual Ident unqualifiedAutoload( const std::string & c );
	virtual Ident qualifiedAutoload( const std::string & alias, const std::string & c );
	
public:
	OrdinaryPackage( PackageManager * pkgmgr, const std::string title );
	virtual ~OrdinaryPackage() {}
};

class StandardLibraryPackage : public Package {
public:
	void loadIfNeeded();
protected:
	virtual Ident absoluteAutoload( const std::string & c );
	virtual Ident unqualifiedAutoload( const std::string & c );
	virtual Ident qualifiedAutoload( const std::string & alias, const std::string & c );
public:
	StandardLibraryPackage( PackageManager * pkgmgr, std::string title ) :
		Package( pkgmgr, title )
	{
	}
	virtual ~StandardLibraryPackage() {}
};

#endif
