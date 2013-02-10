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

//	C++ STL header files.
#include <fstream>
#include <iostream>
#include <utility>

//	C standard Unix header files.
#include <unistd.h>

//	Ginger library header files.
#include "mishap.hpp"

//	Project specific header files.
#include "importsetinfo.hpp"
#include "sax.hpp"

using namespace std;

//#define DBG_IMPORT_SET_INFO

typedef std::map< std::string, std::string > Dict;
typedef std::vector< ImportInfo > ImportList;


class ImportHandler : public Ginger::SaxHandler {
private:
	ImportList & imports;
	
public:
	void startTag( std::string & name, Dict & attrs );
	void endTag( std::string & name );
	
public:
	ImportHandler( ImportList & imports ) :
		imports( imports )
	{
	}
};

void ImportHandler::startTag( string & name, Dict & attrs ) {
	#ifdef DBG_IMPORT_SET_INFO
		cerr << "START " << name << endl;
	#endif
	if ( name != "import" ) return;
	this->imports.push_back( ImportInfo( attrs ) );
}


void ImportHandler::endTag( std::string & name ) {
	#ifdef DBG_IMPORT_SET_INFO
		cerr << "END " << name << endl;
	#endif
}

/*
<package>
	<import from="ginger.library" match0="public" />
	<import from="ginger.constants" match0="public" />
</package>
*/

typedef std::map< std::string, std::string > Dict;

static Dict defPkg( const char * from ) {
	Dict dict;
	dict[ "from" ] = from;
	dict[ "match0" ] = "public";
	return dict;
}

static Dict GINGER_LIBRARY( defPkg( "ginger.library") );
static Dict GINGER_CONSTANTS( defPkg( "ginger.constants") );

void ImportSetInfo::addDefaultImports() {
	this->imports.push_back( ImportInfo( GINGER_LIBRARY ) );
	this->imports.push_back( ImportInfo( GINGER_CONSTANTS ) );
}


void ImportSetInfo::readFile( string filename ) {
	#ifdef DBG_IMPORT_SET_INFO
		cerr << "READING " << filename << endl;
	#endif
	ifstream stream( filename.c_str() );
	if ( stream.good() ) {
		ImportHandler handler( this->imports );
		Ginger::SaxParser saxp( stream, handler );
		saxp.readElement();
	} else if ( access( filename.c_str(), F_OK ) != 0 ) {
		//	File does not exist. Supply default.
		this->addDefaultImports();
	} else {
		//	We have a problem with the file.
		throw Ginger::Mishap( "Cannot read imports file" ).culprit( "Filename", filename );
	}
}



void ImportSetInfo::printImports() {
	for ( 
		ImportList::iterator it = this->imports.begin();
		it != this->imports.end();
		++it
	) {
		it->printInfo();	
	}
}

void ImportSetInfo::fillFromList( std::vector< std::string > & from_list ) {
	for ( 
		ImportList::iterator it = this->imports.begin();
		it != this->imports.end();
		++it
	) {
		from_list.push_back( it->getFrom() );
	}	
}

vector< ImportInfo > & ImportSetInfo::importVector() {
	return this->imports;
}
