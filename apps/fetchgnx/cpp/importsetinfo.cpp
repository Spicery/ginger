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
#include <utility>

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
		cout << "START " << name << endl;
	#endif
	if ( name != "import" ) return;
	this->imports.push_back( ImportInfo( attrs ) );
}


void ImportHandler::endTag( std::string & name ) {
	#ifdef DBG_IMPORT_SET_INFO
		cout << "END " << name << endl;
	#endif
}


void ImportSetInfo::readFile( string filename ) {
	#ifdef DBG_IMPORT_SET_INFO
		cout << "READING " << filename << endl;
	#endif
	ImportHandler handler( this->imports );
	ifstream stream( filename.c_str() );
	if ( stream.good() ) {
		Ginger::SaxParser saxp( stream, handler );
		saxp.readElement();
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
