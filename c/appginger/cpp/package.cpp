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

#include "package.hpp"
#include "sys.hpp"
#include "key.hpp"
#include "makesysfn.hpp"

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

//	Lookup starts in the dictionary of the local package. It then scans all
//	imports packages looking for a unique answer.
Ident Package::lookup( const std::string & c ) {
	//	Imports are not implemented yet, so this is simple.
	Ident id = this->dict.lookup( c );
	if ( not id ) {
		id = this->autoload( c );
	}
	return id;
}

//	Additions always happen in the dictionary of the package itself.
//	However it is necessary to check that there are no protected imports.
Ident Package::add( const std::string & c ) {
	//	Imports are not implemented yet, so this is simple.
	return this->dict.add( c );
}


Ident Package::lookup_or_add( const std::string & c ) {
    Ident id = this->lookup( c );
	if ( not id ) {
    	return this->add( c );
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
		Ident id = this->add( c );
		id->valof = r;
		return id;
	}
}