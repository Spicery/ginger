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

#include <iostream>

#include "varinfo.hpp"

using namespace std;

VarInfo::VarInfo() : 
	frozen( false ),
	mishap( NULL ) 
{
}

VarInfo::~VarInfo() {
	delete this->mishap; 
}

void VarInfo::freeze() {
	this->frozen = true;
}


void VarInfo::init( const std::string & vname, const std::string & pname  ) {
	//cout << "init " << pname << endl;
	if ( ! this->frozen ) {
		this->var_name = vname;
		this->pathname = pname;
	} else if ( this->mishap == NULL ) {
		Mishap * m = new Mishap( "Multiple possible files providing definition" );
		m->culprit( "Variable", this->var_name );
		m->culprit( "Pathname 1", this->pathname );
		m->culprit( "Pathname 2", pname );
		this->mishap = m;
	}
	//cout << "PUT " << this->var_name << " = " << this->pathname << endl;
}

void VarInfo::addTag( const std::string & tag ) {
	if ( !this->frozen ) {
		this->tag_set.insert( tag );
	} else if ( this->mishap == NULL ) {
		Mishap * m = new Mishap( "Multiple possible files trying to tag variable" );
		m->culprit( "Variable", this->var_name );
		m->culprit( "Pathname", this->pathname );
		m->culprit( "Tag", tag );
		this->mishap = m;		
	}
}

const std::string & VarInfo::getPathName() {
	//cout << "Returning " << this->pathname << endl;
	return this->pathname;
}
	
#if 0
	string VarInfo::getVarName() {
		return this->var_name;
	}
#endif

set< string > & VarInfo::tagSet() {
	return this->tag_set;
}
