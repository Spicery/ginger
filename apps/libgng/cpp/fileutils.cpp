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

#include <string>

#include <sys/stat.h>


#include "fileutils.hpp"


namespace Ginger {
using namespace std;


bool fileExists( const string & pathname ) {
	static struct stat buf;
	int st = stat( pathname.c_str(), &buf );
	return st == 0;
}

string shellSafeName( const string & filename ) {
    string safe;
    for ( string::const_iterator it = filename.begin(); it != filename.end(); ++it ) {
        const char ch = *it;
        if ( not isalnum( ch ) ) {
            safe.push_back( '\\' );
        }
        safe.push_back( ch );
    }
    return safe;
}



} // end namespace
