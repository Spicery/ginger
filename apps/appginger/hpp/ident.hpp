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
//#include "facet.hpp"
class FnTermClass;

#include <string>
#include <memory>

#include "common.hpp"


class IdentClass {
private:
	const std::string	name_data;
	bool				is_local;
	bool 				is_outer;
	bool				is_assigned;
	bool				is_shared;
	int					slot;
	FnTermClass *		func;

public:
	//const FacetSet * 	facets;
	Valof * 			value_of;		//	used for global idents
	int 				level;			//	level of scope

public:
	bool isSame( IdentClass * other );
	bool isLocal() { return this->is_local; }
	void setLocal() { this->is_local = true; }
	bool isGlobal() { return !this->is_local; }
	void setGlobal() { this->is_local = false; }
	
	void setAssigned() { this->is_assigned = true; }
	bool isAssigned() { return this->is_assigned; }
	void setOuter() { this->is_outer = true; }
	bool isOuter() { return this->is_outer; }
	bool isInner() { return this->is_local && !this->is_outer; }
	bool isShared() { return this->is_shared || ( this->is_outer && this->is_assigned ); }
	void setShared() { this->is_shared = true; }
	
public:
	void setSlot( int n ) { this->slot = n; }
	int getFinalSlot() const { return this->slot; }
	void swapSlot( const int a, const int b ) {
		if ( this->slot == a ) {
			this->slot = b;
		} else if ( this->slot == b ) {
			this->slot = a;
		}
	}
	
public:
	FnTermClass * function() { return this->func; }
	
public:
	IdentClass( const std::string & nm ); /*, const FacetSet * facets );*/	//	Global
	IdentClass( const std::string & nm, FnTermClass * fn ); 		//	Local
	const std::string & getNameString() const;
};

typedef shared< IdentClass > Ident;


//Ident identNewLocal( const std::string & nm, FnTermClass * fn );
//Ident identNewLastArg( const std::string & nm, FnTermClass * fn );
Ident identNewTmp( int n );
Ident identNewGlobal( const std::string nm ); /*, const FacetSet * facets );*/

#endif

