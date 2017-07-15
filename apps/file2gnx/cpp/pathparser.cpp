/******************************************************************************\
	Copyright (c) 2010 Stephen Leach. AppGinger is distributed under the terms 
	of the GNU General Public License. This file is part of Ginger.

    Ginger is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Ginger is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Ginger.  If not, see <http://www.gnu.org/licenses/>.
\******************************************************************************/

//#define DBG_PATH_PARSER

#include <string>

#ifdef DBG_PATH_PARSER
#include <iostream>
#endif

#include "pathparser.hpp"

using namespace std;

PathParser::PathParser( const string & path ) : 
	stem_start( 0 ),
	stem_finish( path.size() ),
	extn_start( path.size() ),
	path( path )
{
	const size_t n1 = this->path.rfind( '/' );
	if ( n1 != string::npos ) {
		this->stem_start = n1 + 1;
	}
	const size_t n2 = this->path.find( '.', this->stem_start );
	if ( n2 != string::npos ) {
		this->stem_finish = n2;
		this->extn_start = n2 + 1;
	}
}

string PathParser::extension() const {
#ifdef DBG_PATH_PARSER
	cerr << "estart: " << this->extn_start << endl;
#endif
	return this->path.substr( this->extn_start );
}

string PathParser::stem() const {
#ifdef DBG_PATH_PARSER
	cerr << "sstart: " << this->stem_start << endl;
	cerr << "sfinish:" << this->stem_finish << endl;
#endif
	return this->path.substr( this->stem_start, this->stem_finish - this->stem_start );
}

string PathParser::fileName() const {
	return this->path.substr( this->stem_start );	
}

string PathParser::folderPath() const {
	return this->path.substr( 0, this->stem_start - 1 );	
}
