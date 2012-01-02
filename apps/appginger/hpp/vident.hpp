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

#ifndef VIDENT_HPP
#define VIDENT_HPP

#include "mnx.hpp"

#include "shared.hpp"
#include "valof.hpp"

typedef class CodeGenClass *CodeGen;


class VIdent {
private:
	enum VIdentFlavour {
		CONSTANT_FLAVOUR,
		LOCAL_FLAVOUR,
		GLOBAL_FLAVOUR
	}					flavour;
	int					slot;
	Valof *				valof;
	Ref					ref;
	
public:
	bool isLocal() const { return this->flavour == LOCAL_FLAVOUR; }
	bool isGlobal() const { return this->flavour == GLOBAL_FLAVOUR; }
	bool isConstant() const { return this->flavour == CONSTANT_FLAVOUR; }
	
public:
	int getSlot() const { return this->slot; }
	Valof * getValof() const { return this->valof; }
	Ref getRef() const { return this->ref; }
	
public:
	VIdent( const int slot ) : flavour( LOCAL_FLAVOUR ), slot( slot ), valof( NULL ), ref( 0 ) {}
	VIdent( Valof * valof ) : flavour( GLOBAL_FLAVOUR ), slot( -1 ), valof( valof ), ref( 0 ) {} 
	VIdent( CodeGen codegen, shared< Ginger::Mnx > vid );
	VIdent( Ref ref ) : flavour( CONSTANT_FLAVOUR ), slot( -1 ), valof( NULL ), ref( ref ) {}
};

#endif

