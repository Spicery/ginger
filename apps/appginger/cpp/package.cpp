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

#include <boost/iostreams/stream_buffer.hpp>
#include <boost/iostreams/device/file_descriptor.hpp>

#include "package.hpp"
#include "sys.hpp"
#include "key.hpp"
#include "makesysfn.hpp"
#include "mishap.hpp"
#include "rcep.hpp"
#include "command.hpp"

using namespace boost; 
using namespace std;

#define FETCHGNX ( INSTALL_BIN "/fetchgnx" )

//#define DBG_PACKAGE

OrdinaryPackage::OrdinaryPackage( PackageManager * pkgmgr, const std::string title ) :
	Package( pkgmgr, title )
{
	#ifdef DBG_PACKAGE
		std::cout << "NEW ORDINARY PACKAGE: " << title << std::endl;
	#endif
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

Import * Package::getAlias( const std::string alias ) {
	for ( 
		vector< Import >::iterator it = this->imports.begin(); 
		it != this->imports.end();
		++it
	) {
		if ( alias == it->alias ) {
			return &*it;
		}
	}	
	return NULL;
}

Ident Package::exported( const std::string & name, const FacetSet * facets ) {
	#ifdef DBG_PACKAGE
		cout << "Looking up an exported ID: " << name << " from package: " << this->title << endl;
	#endif
	Ident id = this->dict.lookup( name );
	if ( id ) {
		#ifdef DBG_PACKAGE
			cout << "Found in local dictionary with facet: " << id->facet->name() << endl;
		#endif
		return id->facets != NULL && id->facets->isntEmptyIntersection( facets ) ? id : shared< IdentClass >( (IdentClass *)NULL );
	}
	
	for ( 
		vector< Import >::iterator it = this->imports.begin(); 
		it != this->imports.end();
		++it
	) {
		if ( it->intos != NULL && it->intos->isntEmptyIntersection( facets ) ) {
			Package * from_pkg = it->from;
			Ident id = from_pkg->exported( name, it->matchingTags() );
			/*
			if ( id ) {
				return id->facets.contain( facet ) ? id : shared< IdentClass >( (IdentClass *)NULL );
			}*/
			return id;
		}
	}	
	
	id = this->autoload( name );
	if ( id ) {
		#ifdef DBG_PACKAGE
			cout << "Autoload: Exporting with facet: " << ( id->facet ? id->facet->name() : "<null>" ) << endl;
			cout << "Autoload: Importing with facet: " << ( facet ? facet->name() : "<null>" ) << endl;
		#endif
		return id->facets!= NULL && id->facets->isntEmptyIntersection( facets ) ? id : shared< IdentClass >( (IdentClass *)NULL );
	}
	return id;
}

//	Lookup starts in the dictionary of the local package. It then scans all
//	imports packages looking for a unique answer.
Ident Package::lookup( const std::string & c, bool search, bool autoload ) {
	//	Imports are not implemented yet, so this is simple.
	Ident id = this->dict.lookup( c );
	if ( id ) return id;

	if ( search ) {
		for ( 
			vector< Import >::iterator it = this->imports.begin(); 
			it != this->imports.end();
			++it
		) {
			Package * from_pkg = it->from;
			Ident id = from_pkg->exported( c, it->matchingTags() );
			if ( id ) return id;
		}	
	}
	
	if ( autoload ) {
		id = this->autoload( c );
	}
	return id;
}

//	Additions always happen in the dictionary of the package itself.
//	However it is necessary to check that there are no protected imports.
Ident Package::add( const std::string & c, const FacetSet * facets ) {
	for ( 
		vector< Import >::iterator it = this->imports.begin(); 
		it != this->imports.end();
		++it
	) {
		//cout << "Checking that " << it->from->title << "is protected? " << it->is_protected << endl;
		if ( it->is_protected ) {
			Package * from_pkg = it->from;			
			Ident id = from_pkg->exported( c, it->matchingTags() );
			if ( id ) {
				throw Mishap( "Trying to shadow a protected import" );
			}
		}
	}	
	return this->dict.add( c, /*facet,*/ facets );
}


Ident Package::lookupOrAdd( const std::string & c, const FacetSet * facets ) {
    Ident id = this->lookup( c, false, false );
	if ( not id ) {
    	return this->add( c, facets );
    } else {
    	return id;
    }
}

void Package::forwardDeclare( const std::string & c ) {
	this->dict.add( c, fetchEmptyFacetSet() );
}

typedef iostreams::stream_buffer< iostreams::file_descriptor_source > fdistreambuf;

Ident OrdinaryPackage::autoload( const std::string & c ) {
	syslog( LOG_INFO, "Autoloading %s", c.c_str() );
	//	Then we invoke fetchgnx.
	
	//cout << "Invoking fetchgnx" << endl;
	
	Command cmd( FETCHGNX );
	cmd.addArg( "-j" );
	cmd.addArg( this->getMachine()->getAppContext().getProjectFolder() );
	cmd.addArg( "-p" );
	cmd.addArg( this->title );
	cmd.addArg( "-v" );
	cmd.addArg( c );
	
	//	We convert the returning GNX to a stream.
	int fd = cmd.run();
	iostreams::file_descriptor_source fdsource( fd, iostreams::close_handle );
	fdistreambuf input_stream_buf( fdsource );
	istream input_stream( &input_stream_buf );

	//cout << "Loading" << endl;
	
	stringstream prog;
	{
		string s;
		while ( getline( input_stream, s ) ) {
			prog << s;
		}
	}
	cout << "[[" << prog.str() << "]]" << endl;
		

	//	Now we establish a forward declaration.
	this->forwardDeclare( c );
	//	Hopefully the forward declaration will be justified.
	Ident id = this->dict.lookup( c );

	try {
		//	And we load the stream.
		RCEP rcep( this );
		rcep.setPrinting( false );	//	Turn off result printing.
		//cout << "Do load" << endl;
		rcep.unsafe_read_comp_exec_print( prog, cout );
		//cout << "Done" << endl;

		if ( id->value_of->valof == sys_undef ) {
			//	The autoloading failed. Undo the declaration.
			this->dict.remove( c );
			syslog( LOG_ERR, "Autoloading %c failed due to the loaded code not establishing a binding", c.c_str() );
			return shared< IdentClass >();
		} else {
			return id;
		}
	} catch ( std::exception & e ) {
		//	Undo the forward declaration.
		if ( id->value_of->valof == sys_undef ) {
			//	The autoloading failed. Undo the declaration.
			this->dict.remove( c );
		}
		syslog( LOG_ERR, "Autoloading %c failed due to an exception", c.c_str() );
	}
	
	//	No autoloading implemented yet - just fail.
	return shared< IdentClass >();
}

Ident StandardLibraryPackage::autoload( const std::string & c ) {
	Ref r = makeSysFn( this->pkgmgr->vm->plant(), c, sys_undef );
	if ( r == sys_undef ) {
		//	Doesn't match a system call. Fail.
		return shared< IdentClass >();
	} else {
		Ident id = this->add( c, fetchFacetSet( "public" ) );
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
			it->matching_tags == imp.matching_tags &&
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

const FacetSet * Import::matchingTags() { 
	return this->matching_tags;
}

Package * Import::package() { 
	return this->from; 
}

Valof * Package::valof( const std::string & c ) {
	Ident id = this->lookup( c, true, true );
	if ( not id ) return NULL;
	return id->value_of;
}
