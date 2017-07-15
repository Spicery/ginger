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

#include <iostream>

#include <stddef.h>

#include "varinfo.hpp"
#include "facet.hpp"

using namespace std;
using namespace Ginger;

VarInfo::VarInfo() : 
	frozen( false ),
	var_info_tags( fetchEmptyFacetSet() ),
	parent_var_info( NULL ),
	mishap( NULL ) 
{
}

VarInfo::~VarInfo() {
	delete this->mishap; 
}

void VarInfo::freeze() {
	this->frozen = true;
}

void VarInfo::init( VarInfo * v ) {
	if ( ! this->frozen ) {
		this->var_name = v->var_name;
		this->parent_var_info = v;
	} else {
		throw Mishap( "Unreachable" );
	}
}

const std::string & VarInfo::getPathName() const {
	if ( this->parent_var_info == NULL ) {
		return this->pathname;
	} else {
		return this->parent_var_info->getPathName();
	}
}

const std::string & VarInfo::name() const {
	if ( this->frozen) {
		return this->var_name;
	} else {
		throw Mishap( "Internal error" );
	}
}

void VarInfo::init( const std::string & vname, const std::string & pname  ) {
	//cout << "init " << pname << endl;
	if ( ! this->frozen ) {
		this->var_name = vname;
		this->pathname = pname;
	} else if ( this->mishap == NULL ) {
		Mishap * m = new Mishap( "Multiple possible files providing definition" );
		m->culprit( "Variable", this->var_name );
		m->culprit( "Pathname 1", this->getPathName() );
		m->culprit( "Pathname 2", pname );
		this->mishap = m;
	}
	//cout << "PUT " << this->var_name << " = " << this->pathname << endl;
}

void VarInfo::addTag( const std::string & tag ) {
	if ( !this->frozen ) {
		this->var_info_tags = this->var_info_tags->add( tag );
	} else if ( this->mishap == NULL ) {
		Mishap * m = new Mishap( "Multiple possible files trying to tag variable" );
		m->culprit( "Variable", this->var_name );
		m->culprit( "Pathname", this->getPathName() );
		m->culprit( "Tag", tag );
		this->mishap = m;		
	}
}

const FacetSet * VarInfo::varInfoTags() {
	return this->var_info_tags;
}

void VarInfo::setTags( const FacetSet * fs ) {
	this->var_info_tags = fs;
}

BuiltInFlag VarInfo::builtInFlag() const {
	const string & p = this->getPathName();
	const size_t n = p.rfind( '.' );
	
	if ( n == string::npos ) {
		return NONSYS;
	} else {		
		const string extn( p.substr( n + 1, p.size() ) );
		if ( extn == "sysfn" ) {
			return SYSFN;
		} else if ( extn == "sysclass" ) {
			return SYSCLASS;
		} else {
			return NONSYS;
		}
	}
}
