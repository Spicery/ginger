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

#include "gnxconstants.hpp"
#include "mishap.hpp"

#include "vident.hpp"

#include "package.cpp"


VIdent::VIdent( CodeGen codegen, shared< Ginger::Mnx > vid ) {
	const string & nm = vid->name();
	if ( nm == ID || nm == VAR ) {
		if ( vid->hasAttribute( VID_SCOPE, "local" ) ) {
			this->flavour = LOCAL_FLAVOUR;
			this->slot = vid->attributeToInt( VID_SLOT );
			this->valof = NULL;
		} else {	
			this->flavour = GLOBAL_FLAVOUR;
			Package * def_pkg = codegen->vm->getPackage( vid->attribute( VID_DEF_PKG ) );
			this->valof = def_pkg->fetchDefinitionValof( vid->attribute( VID_NAME ) );
			this->slot = 0;
		}
	} else if ( nm == CONSTANT ) {
		this->flavour = CONSTANT_FLAVOUR;
		this->valof = NULL;
		this->slot = NULL;
		this->ref = codegen->calcConstant( vid );
	}
}
