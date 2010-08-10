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

#include <vector>
#include <iostream>
using namespace std;

#include "package.hpp"
#include "sys.hpp"
#include "key.hpp"
#include "makesysfn.hpp"

#define DBG_PACKAGE

OrdinaryPackage::OrdinaryPackage( PackageManager * pkgmgr, const std::string title ) :
	Package( pkgmgr, title )
{
	std::cout << "NEW ORDINARY PACKAGE: " << title << std::endl;
}


PackageManager::PackageManager( MachineClass * vm ) :
	vm( vm )
{
	this->packages[ STANDARD_LIBRARY ] = new StandardLibraryPackage( this, std::string( STANDARD_LIBRARY  ) );
}

Package * PackageManager::getPackage( std::string title ) {
	std::map< std::string, class Package * >::iterator it = this->packages.find( title );
	if ( it == this->packages.end() ) {
		return this->packages[ title ] = new OrdinaryPackage( this, title );
	} else {
		return it->second;
	}
}

Package * Package::getPackage( std::string title ) {
	return this->pkgmgr->getPackage( title );
}

Ident Package::exported( const std::string & name, const Facet * facet ) {
	#ifdef DBG_PACKAGE
		cout << "Looking up an exported ID: " << name << " from package: " << this->title << endl;
	#endif
	Ident id = this->dict.lookup( name );
	if ( id ) {
		#ifdef DBG_PACKAGE
			cout << "Found in local dictionary with facet: " << id->facet->name() << endl;
		#endif
		return id->facet == facet ? id : shared< IdentClass >( (IdentClass *)NULL );
	}
	
	for ( 
		vector< Import >::iterator it = this->imports.begin(); 
		it != this->imports.end();
		++it
	) {
		if ( it->into != facet ) {
			Package * from_pkg = it->from;
			Ident id = from_pkg->exported( name, it->facet );
			if ( id ) {
				return id->facet == facet ? id : shared< IdentClass >( (IdentClass *)NULL );
			}
		}
	}	
	
	id = this->autoload( name );
	if ( id ) {
		#ifdef DBG_PACKAGE
			cout << "Autoload: Exporting with facet: " << ( id->facet ? id->facet->name() : "<null>" ) << endl;
			cout << "Autoload: Importing with facet: " << ( facet ? facet->name() : "<null>" ) << endl;
		#endif
		return id->facet == facet ? id : shared< IdentClass >( (IdentClass *)NULL );
	}
	return id;
}

//	Lookup starts in the dictionary of the local package. It then scans all
//	imports packages looking for a unique answer.
Ident Package::lookup( const std::string & c ) {
	//	Imports are not implemented yet, so this is simple.
	Ident id = this->dict.lookup( c );
	if ( id ) return id;

	for ( 
		vector< Import >::iterator it = this->imports.begin(); 
		it != this->imports.end();
		++it
	) {
		Package * from_pkg = it->from;
		Ident id = from_pkg->exported( c, it->facet );
		if ( id ) return id;
	}	
	
	id = this->autoload( c );
	return id;
}

//	Additions always happen in the dictionary of the package itself.
//	However it is necessary to check that there are no protected imports.
Ident Package::add( const std::string & c, const Facet * facet ) {
	//	Imports are not implemented yet, so this is simple.
	return this->dict.add( c, facet );
}


Ident Package::lookup_or_add( const std::string & c, const Facet * facet ) {
    Ident id = this->lookup( c );
	if ( not id ) {
    	return this->add( c, facet );
    } else {
    	return id;
    }
}


Ident OrdinaryPackage::autoload( const std::string & c ) {
	//	No autoloading implemented yet - just fail.
	return shared< IdentClass >();
}

Ident StandardLibraryPackage::autoload( const std::string & c ) {
	Ref r = makeSysFn( this->pkgmgr->vm->plant(), c, sys_undef );
	if ( r == sys_undef ) {
		//	Doesn't match a system call. Fail.
		return shared< IdentClass >();
	} else {
		Ident id = this->add( c, fetchFacet( "public" ) );
		id->value_of->valof = r;
		return id;
	}
}


void Package::import( const Import & imp ) {
	//	Strictly speaking we should not pushback until we have verified
	//	we have a non-duplicate import. Imports are indexed by
	//		-	facet
	//		-	from package
	//		-	alias
	for ( 
		std::vector< Import >::iterator it = this->imports.begin(); 
		it != this->imports.end();
		++it
	) {
		if ( 
			it->facet == imp.facet &&
			it->from == imp.from &&
			it->alias == imp.alias
		) {
			*it = imp;
			return;
		}
	}
	//	No previous matching imports, so add.
	this->imports.push_back( imp );
}

