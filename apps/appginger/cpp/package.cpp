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

#include <unistd.h>


#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <exception>

#include <syslog.h>

//#include <boost/iostreams/stream_buffer.hpp>
//#include <boost/iostreams/device/file_descriptor.hpp>

#include "common.hpp"
#include "package.hpp"
#include "sys.hpp"
#include "key.hpp"
#include "makesysfn.hpp"
#include "mishap.hpp"
#include "rcep.hpp"
#include "command.hpp"

//using namespace boost; 
using namespace std;

#define FETCHGNX ( INSTALL_BIN "/fetchgnx" )

//#define DBG_PACKAGE

Package::~Package() {
}


OrdinaryPackage::OrdinaryPackage( PackageManager * pkgmgr, const std::string title ) :
	Package( pkgmgr, title ),
	loaded( false )
{
	#ifdef DBG_PACKAGE
		std::cout << "NEW ORDINARY PACKAGE: " << title << std::endl;
	#endif
}


PackageManager::PackageManager( MachineClass * vm ) :
	vm( vm )
{
	this->packages[ STANDARD_LIBRARY_PACKAGE ] = new StandardLibraryPackage( this, std::string( STANDARD_LIBRARY_PACKAGE ) );
}

Package * PackageManager::getPackage( std::string title ) {
	std::map< std::string, class Package * >::iterator it = this->packages.find( title );
	if ( it == this->packages.end() ) {
		Package * p = this->packages[ title ] = new OrdinaryPackage( this, title );
		p->loadIfNeeded();
		return p;
	} else {
		return it->second;
	}
}

void PackageManager::reset() {
	for ( 
		map< string, class Package * >::iterator it = this->packages.begin();
		it != this->packages.end();
		++it
	) {
		it->second->reset();
	}
}

Package * Package::getPackage( std::string title ) {
	return this->pkgmgr->getPackage( title );
}

//	Additions always happen in the dictionary of the package itself.
//	However it is necessary to check that there are no protected imports.
Ident Package::add( const std::string & c ) { //, const FacetSet * facets ) {
	return this->dict.add( c ); //, facets );
}

Ident Package::fetchUnqualifiedIdent( const std::string & c ) {
	Ident id = this->dict.lookup( c );
	if ( id ) {
		return id;
	} else {
		Package * p = this->unqualifiedResolutions[ c ];
		if ( p != NULL ) {
			Ident id = p->dict.lookup( c );
			if ( not id ) throw Ginger::SystemError( "Unqualified lookup has been incorrectly recorded" );
			return id;
		} else {
			return this->unqualifiedAutoload( c );
		}
	}
}

Ident Package::fetchQualifiedIdent( const std::string & alias, const std::string & c ) {
	Ident id = this->dict.lookup( c );
	if ( id ) {
		return id;
	} else {
		Package * p = this->qualifiedResolutions[ pair< string, string >( alias, c ) ];
		if ( p != NULL ) {
			Ident id = p->dict.lookup( c );
			if ( not id ) throw Ginger::SystemError( "Qualified lookup has been incorrectly recorded" );
			return id;
		} else {
			return this->qualifiedAutoload( alias, c );
		}
	}
}

Ident Package::fetchAbsoluteIdent( const std::string & c ) {
	//cout << "Absolute fetch" << endl;
	Ident id = this->dict.lookup( c );
	if ( id ) {
		return id;
	} else {
		return this->absoluteAutoload( c );
	}
}

Ident Package::fetchDefinitionIdent( const std::string & c ) { //, const FacetSet * facets ) {
	Ident id = this->dict.lookup( c );
	if ( not id ) {
    	return this->add( c ); //, facets );
    } else {
    	//	What about the consistency of the old and new facets?
    	//	TO BE DONE!
    	return id;
    }
}

Ident Package::forwardDeclare( const std::string & c ) {
	return this->dict.add( c ); //, fetchEmptyFacetSet() );
}

void Package::retractForwardDeclare( const std::string & c ) {
	return this->dict.remove( c );
}

Valof * Package::valof( const std::string & c ) {
	Ident id = this->dict.lookup( c );
	if ( not id ) return NULL;
	return id->value_of;
}
