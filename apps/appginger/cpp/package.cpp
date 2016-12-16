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

//#include <boost/iostreams/stream.hpp>
//#include <boost/iostreams/device/file_descriptor.hpp>

#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <exception>

#include <stddef.h>
#include <unistd.h>
#include <sys/errno.h>
#include <syslog.h>

#include "debug.hpp"

#include "sax.hpp"
#include "command.hpp"
#include "mishap.hpp"

#include "common.hpp"
#include "package.hpp"
#include "sys.hpp"
#include "key.hpp"
#include "makesysfn.hpp"
#include "rcep.hpp"

#include "wellknownpaths.hpp"

namespace Ginger {
using namespace std;

//#define DBG_PACKAGE_AUTOLOAD 1


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
	//this->packages[ STANDARD_LIBRARY_PACKAGE ] = new StandardLibraryPackage( this, std::string( STANDARD_LIBRARY_PACKAGE ) );
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

Valof * Package::add( const std::string & s, bool prot ) { 
    return this->table[ s ] = new Valof( this, s, prot );
}


Valof * Package::fetchValof( const std::string & c, bool prot, bool isID ) {
	Valof * id = this->lookup( c );
	if ( id ) {
		return id;
	} else if ( isID ) {
		Valof * id = this->absoluteAutoload( c );
		return id;
	} else {
    	Valof * id = this->add( c, prot );
    	return id;
	}
}


Valof * Package::fetchAbsoluteValof( const std::string & c ) {
	Valof * id = this->lookup( c );
	if ( id ) {
		return id;
	} else {
		Valof * id = this->absoluteAutoload( c );
		return id;
	}
}


Valof * Package::fetchDefinitionValof( const std::string & c, bool prot ) { 
	Valof * id = this->lookup( c );
	if ( not id ) {
    	Valof * id = this->add( c, prot );
    	return id;
    } else {
    	return id;
    }
}

Valof * Package::forwardDeclare( const std::string & c ) {
	return this->add( c, false );
}

void Package::retractForwardDeclare( const std::string & c ) {
	return this->remove( c );
}

Valof * Package::valof( const std::string & c ) {
	Valof * id = this->lookup( c );
	return id;
}


void Package::reset() {
	for (
		map< string, Valof * >::iterator it = this->table.begin();
		it != this->table.end();
	) {
		Valof * id = it->second;
		if ( id->valof == SYS_UNASSIGNED ) {
			this->table.erase( it++ );
		} else {
			++it;
		}
	}
}

Valof * Package::lookup( const std::string & s ) {
	std::map< std::string, Valof * >::iterator it = this->table.find( s );
	return it == this->table.end() ? NULL : it->second;
}

void Package::remove( const std::string & s ) {
	this->table.erase( s );
}

Valof * Package::lookup_or_add( const std::string & c, bool prot ) {
    Valof * id = this->lookup( c );
	if ( not id ) {
    	return this->add( c, prot ); 
    } else {
    	return id;
    }
}


/*******************************************************************************
*	Autoloading in Ordinary Packages
*******************************************************************************/

static void fRenderMnx( FILE * foutd, shared< Ginger::Mnx > mnx ) {
	#if 0
		namespace io = boost::iostreams;
		//cerr << "fd = " << fd << endl;
		io::file_descriptor_sink fdsink( fd, io::never_close_handle );
		io::stream_buffer< io::file_descriptor_sink > output_stream_buffer( fdsink );
		std::ostream output( &output_stream_buffer );
		mnx->render( output );
		output.flush();
	#else
		mnx->frender( foutd );
		fflush( foutd );
	#endif
}

Valof * OrdinaryPackage::absoluteAutoload( const std::string & c ) {
	syslog( LOG_INFO, "Autoloading is_absolute_ref %s", c.c_str() );
	
	Ginger::Command cmd( ( USESNAP ? getenv( "SNAP" ) : nullptr ), FETCHGNX );
	cmd.addArg( "-X" );
	{
		list< string > & folders = this->getMachine()->getAppContext().getProjectFolderList();
		for ( 
			list< string >::iterator it = folders.begin();
			it != folders.end();
			++it
		) {
			cmd.addArg( "-j" );
			cmd.addArg( *it );
		}
	}
	
	#ifdef DBG_PACKAGE_AUTOLOAD
		cerr << "Autoloading" << endl;
		cerr << "  Package 	: " << this->title << endl;
		cerr << "  Variable : " << c << endl;
		list< string > & folders = this->getMachine()->getAppContext().getProjectFolderList();
		for ( 
			list< string >::iterator it = folders.begin();
			it != folders.end();
			++it
		) {
			cerr << "  Folder  : " << *it << endl;
		}
		cerr << "  Command  : " << cmd.asPrintString() << endl;
	#endif

	Ginger::MnxBuilder qb;
	qb.start( "fetch.definition" );
	qb.put( "pkg.name", this->title );
	qb.put( "var.name", c );
	qb.end();
	shared< Ginger::Mnx > query( qb.build() );

	#ifdef DBG_PACKAGE_AUTOLOAD
		cerr << "appginger asking for definition, using fetchgnx -X" << endl;
		cerr << "  [[";
		query->render( cerr );
		cerr << "]]" << endl;
	#endif	

	cmd.runWithInputAndOutput();
	int fd = cmd.getInputFD();   
	FILE * foutd = fdopen( cmd.getOutputFD(), "w" );
	fRenderMnx( foutd, query );
	
	stringstream prog;
	for (;;) {
		static char buffer[ 1024 ];
		int n = read( fd, buffer, sizeof( buffer ) );
		if ( n == 0 ) break;
		if ( n == -1 ) {
			if ( errno != EINTR ) {
				perror( "PACKAGE AUTOLOAD" );
				throw Ginger::Mishap( "Failed to read" );
			}
		} else if ( n > 0 ) {
			prog.write( buffer, n );
		}
	}
		
	fclose( foutd );
	
	#ifdef DBG_PACKAGE_AUTOLOAD
		cerr << "appginger/package autoloading, reply from fetchgnx -X" << endl;
		cerr << "  [[" << prog.str() << "]]" << endl;
		cerr << "ok" << endl;
	#endif
	
		
	//	Now we establish a forward declaration - to be justified by the success.
	#ifdef DBG_PACKAGE_AUTOLOAD
		cerr << "appginger/package doing a forward declaration" << endl;
	#endif
	Valof * id = this->forwardDeclare( c );

	try {
		//	And we load the stream.
		RCEP rcep( this );
		rcep.setPrinting( false );	//	Turn off result printing.
		#ifdef DBG_PACKAGE_AUTOLOAD
			cerr << "appginger/package loading into RCEP" << endl;
			cerr << "  [[" << prog.str() << "]]" << endl;
		#endif	
		rcep.unsafe_read_comp_exec_print( prog, cout );		
		return id;		
	} catch ( Ginger::Mishap & e ) {
		//	Undo the forward declaration.
		if ( id->valof == SYS_UNASSIGNED ) {
			//	The autoloading failed. Undo the declaration.
			this->retractForwardDeclare( c );
		}
		syslog( LOG_ERR, "Autoloading %s failed due to an exception", c.c_str() );
		throw Ginger::Mishap( "Autoloading failed (rethrowing)" ).culprit( "Name", c ).cause( e );
	}
	
	//	No autoloading implemented yet - just fail.
	return NULL;
}

void OrdinaryPackage::loadIfNeeded() {
	if ( this->loaded ) return;
	
	syslog( LOG_INFO, "Loading package %s", this->title.c_str() );

	Ginger::Command cmd( ( USESNAP ? getenv( "SNAP" ) : nullptr ), FETCHGNX );
	cmd.addArg( "-I" );
	{
		list< string > & folders = this->getMachine()->getAppContext().getProjectFolderList();
		for ( 
			list< string >::iterator it = folders.begin();
			it != folders.end();
			++it
		) {
			cmd.addArg( "-j" );
			cmd.addArg( *it );
		}
	}
	cmd.addArg( "-p" );
	cmd.addArg( this->title );

	#ifdef DBG_PACKAGE_AUTOLOAD
		cerr << "loadIfNeeded" << endl;
		cerr << "  Package  : " << this->title << endl;
		list< string > & folders = this->getMachine()->getAppContext().getProjectFolderList();
		for ( 
			list< string >::iterator it = folders.begin();
			it != folders.end();
			++it
		) {
			cerr << "  Folder  : " << *it << endl;
		}
		cerr << "  Command  : " << cmd.asPrintString() << endl;
	#endif

	int fd = cmd.runWithOutput();		
	stringstream prog;
	for (;;) {
		static char buffer[ 1024 ];
		int n = read( fd, buffer, sizeof( buffer ) );
		if ( n == 0 ) break;
		if ( n == -1 ) {
			if ( errno != EINTR ) {
				perror( "PACKAGE AUTOLOAD" );
				throw Ginger::Mishap( "Failed to read" );
			}
		} else if ( n > 0 ) {
			prog.write( buffer, n );
		}
	}
		
	#ifdef DBG_PACKAGE_AUTOLOAD
		cerr << "appginger/package loading via loadIfNeeded" << endl;
		cerr << "  [[" << prog.str() << "]]" << endl;
	#endif	

	if ( prog.str().length() > 0 ) {
		try {
			//	And we load the stream.
			RCEP rcep( this );
			rcep.setPrinting( false );	//	Turn off result printing.
			rcep.unsafe_read_comp_exec_print( prog, cout );		
		} catch ( std::exception & e ) {
			syslog( LOG_ERR, "Package loading failed due to an exception" );
			syslog( LOG_ERR, "Was trying to load %s", this->title.c_str() );
		}
	}
	
	this->loaded = true;
}

} // namespace Ginger
