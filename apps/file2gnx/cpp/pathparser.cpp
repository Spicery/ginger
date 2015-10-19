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

#include <string>

#include "pathparser.hpp"

using namespace std;

PathParser::PathParser( const string & path ) : 
	stem_start( 0 ),
	stem_finish( path.size() ),
	path( path )
{
	const size_t n1 = this->path.rfind( '/' );
	if ( n1 != string::npos ) {
		this->stem_start = n1;
	}
	const size_t n2 = this->path.find( '.', this->stem_start );
	if ( n2 != string::npos ) {
		this->stem_finish = n2;
	}
}

string PathParser::extension() const {
	if ( this->stem_finish >= this->path.size() ) {
		return "";
	} else {
		return this->path.substr( this->stem_finish + 1 );
	}	
}

string PathParser::stem() const {
	return this->path.substr( this->stem_start, this->stem_finish );
}

string PathParser::fileName() const {
	return this->path.substr( this->stem_start );	
}

string PathParser::folderPath() const {
	return this->path.substr( 0, this->stem_start - 1 );	
}
