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

#ifndef IDENT_HPP
#define IDENT_HPP

#include "shared.hpp"
#include "valof.hpp"
#include "facet.hpp"

#include <string>
#include <memory>

#include "common.hpp"


class IdentClass {
private:
	bool				is_local;

public:
	std::string			name;
	//const Facet * 		facet;			//	restricted to a single facet at the moment.
	const FacetSet * 	facets;
	int					slot; 			//	used for local idents
	Valof * 			value_of;		//	used for global idents
	int 				level;			//	level of scope
	IdentClass			*next;      	//	chain used for env linking

public:
	bool isSame( IdentClass * other );
	bool isLocal() { return this->is_local; }
	bool setLocal() { return this->is_local = true; }
	bool isGlobal() { return !this->is_local; }
	bool setGlobal() { return this->is_local = false; }
	
public:
	IdentClass( const std::string & nm, /*const Facet * facet,*/ const FacetSet * facets );
	const std::string & getNameString();
};

typedef shared< IdentClass > Ident;


Ident ident_new_local( const std::string & nm );
Ident ident_new_tmp( int n );
Ident ident_new_global( const std::string & nm, /*const Facet * facet,*/ const FacetSet * facets );

#endif
