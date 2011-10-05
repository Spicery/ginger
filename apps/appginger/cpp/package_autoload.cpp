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
#include <list>

#include <syslog.h>
#include <sys/errno.h>

//#include <boost/iostreams/stream_buffer.hpp>
//#include <boost/iostreams/device/file_descriptor.hpp>

#include "sax.hpp"

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

//#define DBG_PACKAGE_AUTOLOAD

class ResolveHandler : public Ginger::SaxHandler {
public:
	Ginger::Mishap mishap;
	string enc_pkg_name;
	string alias_name;
	string def_pkg_name;
	bool qualified;	
        
public:
	void startTag( std::string & name, std::map< std::string, std::string > & attrs ) {
		if ( name == "resolve" ) {
			qualified = attrs.find( "alias" ) != attrs.end();
			this->enc_pkg_name = attrs[ "enc.pkg" ];
			this->alias_name = attrs[ "alias" ];
			this->def_pkg_name = attrs[ "def.pkg" ];
		} else if ( name == "mishap" ) {
			this->mishap.setMessage( attrs[ "message" ] );
		} else if ( name == "culprit" ) {
			mishap.culprit( attrs[ "name" ], attrs[ "value" ] );
		} else {
			throw Ginger::Mishap( "Unexpected element in response" ).culprit( "Element name", name );
		}
	}
	
	void endTag( std::string & name ) {
		if ( name == "mishap" ) {
			throw mishap;
		}
	}

public:
	void report() {
		cout <<
		( qualified ? "QUALIFIED" : "UNQUALIFIED" ) << "( " <<
		this->enc_pkg_name << ", " << 
		this->alias_name << ", " <<
		this->def_pkg_name << " )" << endl;
	}
        
public:
	ResolveHandler() : mishap( "" ), qualified( false ) {}
};



Ident OrdinaryPackage::unqualifiedAutoload( const std::string & c ) {
	syslog( LOG_INFO, "Autoloading unqualified %s", c.c_str() );
	
	Ginger::Command cmd( FETCHGNX );
	cmd.addArg( "-R" );
	{
		list< string > & folders = this->getMachine()->getAppContext().getProjectFolderList();
		for ( 
			list< string >::iterator it = folders.begin();
			it != folders.end();
			++it
		) {
			cmd.addArg( "-f" );
			cmd.addArg( *it );
		}
	}
	cmd.addArg( "-p" );
	cmd.addArg( this->title );
	cmd.addArg( "-v" );
	cmd.addArg( c );

	//cerr << "About to run the command" << endl;
	int fd = cmd.runWithOutput();		
	//cerr << "Command run " << endl;
	stringstream prog;
	for (;;) {
		static char buffer[ 1024 ];
		int n = read( fd, buffer, sizeof( buffer ) );
		//cerr << "READ " << n << " bytes from the file descriptor" << endl;
		if ( n == 0 ) break;
		if ( n == -1 ) {
			if ( errno != EINTR ) {
				perror( "PACKAGE AUTOLOAD" );
				throw Ginger::Mishap( "Failed to read" );
			}
		} else if ( n > 0 ) {
			//cerr << "|";
			//write( 2, buffer, n );
			//cerr << "|" << endl;
			prog.write( buffer, n );
		}
	}
		
	#ifdef DBG_PACKAGE_AUTOLOAD
		cout << "[[" << prog.str() << "]]" << endl;
	#endif

	ResolveHandler resolve;
	Ginger::SaxParser parser( prog, resolve );
	parser.readElement();
	
	//resolve.report();
	
	Package * def_pkg = this->getPackage( resolve.def_pkg_name );
	this->unqualifiedResolutions[ c ] = def_pkg;
	
	return def_pkg->fetchAbsoluteIdent( c );

}

Ident OrdinaryPackage::qualifiedAutoload( const std::string & alias, const std::string & c ) {
	syslog( LOG_INFO, "Autoloading qualified %s", c.c_str() );
	
	Ginger::Command cmd( FETCHGNX );
	cmd.addArg( "-R" );
	{
		list< string > & folders = this->getMachine()->getAppContext().getProjectFolderList();
		for ( 
			list< string >::iterator it = folders.begin();
			it != folders.end();
			++it
		) {
			cmd.addArg( "-f" );
			cmd.addArg( *it );
		}
	}
	cmd.addArg( "-p" );
	cmd.addArg( this->title );
	cmd.addArg( "-a" );
	cmd.addArg( alias );
	cmd.addArg( "-v" );
	cmd.addArg( c );

	//cerr << "About to run the command" << endl;
	int fd = cmd.runWithOutput();		
	//cerr << "Command run " << endl;
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
			//cerr << "|";
			//write( 2, buffer, n );
			//cerr << "|" << endl;
			prog.write( buffer, n );
		}
	}
		
	#ifdef DBG_PACKAGE_AUTOLOAD
		cout << "[[" << prog.str() << "]]" << endl;
	#endif

	ResolveHandler resolve;
	Ginger::SaxParser parser( prog, resolve );
	parser.read();
	
	//resolve.report();
	
	Package * def_pkg = this->getPackage( resolve.def_pkg_name );
	this->qualifiedResolutions[ pair< string, string >( alias, c ) ] = def_pkg;
	
	return def_pkg->fetchAbsoluteIdent( c );
}

Ident OrdinaryPackage::absoluteAutoload( const std::string & c ) {
	syslog( LOG_INFO, "Autoloading is_absolute_ref %s", c.c_str() );
	
	Ginger::Command cmd( FETCHGNX );
	cmd.addArg( "-D" );
	{
		list< string > & folders = this->getMachine()->getAppContext().getProjectFolderList();
		for ( 
			list< string >::iterator it = folders.begin();
			it != folders.end();
			++it
		) {
			cmd.addArg( "-f" );
			cmd.addArg( *it );
		}
	}
	cmd.addArg( "-p" );
	cmd.addArg( this->title );
	cmd.addArg( "-v" );
	cmd.addArg( c );

	//cerr << "About to run the command" << endl;
	int fd = cmd.runWithOutput();		
	//cerr << "Command run " << endl;
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
			//cerr << "|";
			//write( 2, buffer, n );
			//cerr << "|" << endl;
			prog.write( buffer, n );
		}
	}
		
	#ifdef DBG_PACKAGE_AUTOLOAD
		cout << "[[" << prog.str() << "]]" << endl;
	#endif
	
		
	//	Now we establish a forward declaration - to be justified by the success.
	Ident id = this->forwardDeclare( c );

	try {
		//	And we load the stream.
		RCEP rcep( this );
		rcep.setPrinting( false );	//	Turn off result printing.
		rcep.unsafe_read_comp_exec_print( prog, cout );		
		return id;		
	} catch ( std::exception & e ) {
		//	Undo the forward declaration.
		if ( id->value_of->valof == sys_undef ) {
			//	The autoloading failed. Undo the declaration.
			this->retractForwardDeclare( c );
		}
		syslog( LOG_ERR, "Autoloading %s failed due to an exception", c.c_str() );
	}
	
	//	No autoloading implemented yet - just fail.
	return shared< IdentClass >();	
}

void OrdinaryPackage::loadIfNeeded() {
	if ( this->loaded ) return;
	
	syslog( LOG_INFO, "Loading package %s", this->title.c_str() );
	
	Ginger::Command cmd( FETCHGNX );
	cmd.addArg( "-I" );
	{
		list< string > & folders = this->getMachine()->getAppContext().getProjectFolderList();
		for ( 
			list< string >::iterator it = folders.begin();
			it != folders.end();
			++it
		) {
			cmd.addArg( "-f" );
			cmd.addArg( *it );
		}
	}
	cmd.addArg( "-p" );
	cmd.addArg( this->title );

	//cerr << "About to run the command" << endl;
	int fd = cmd.runWithOutput();		
	//cerr << "Command run " << endl;
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
			//cerr << "|";
			//write( 2, buffer, n );
			//cerr << "|" << endl;
			prog.write( buffer, n );
		}
	}
		
	#ifdef DBG_PACKAGE_AUTOLOAD
		cout << "[[" << prog.str() << "]]" << endl;
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


/*******************************************************************************
*	Standard Library Package
*******************************************************************************/

Ident StandardLibraryPackage::unqualifiedAutoload( const std::string & c ) {
	return this->absoluteAutoload( c );
}

Ident StandardLibraryPackage::qualifiedAutoload( const std::string & alias, const std::string & c ) {
	return shared< IdentClass >();
}

Ident StandardLibraryPackage::absoluteAutoload( const std::string & c ) {
	Ref r = makeSysFn( this->pkgmgr->vm->plant(), c, sys_undef );
	if ( r == sys_undef ) {
		//	Doesn't match a system call. Fail.
		return shared< IdentClass >();
	} else {
		Ident id = this->add( c ); //, fetchFacetSet( "public" ) );
		id->value_of->valof = r;
		return id;
	}
}

void StandardLibraryPackage::loadIfNeeded() {
}
