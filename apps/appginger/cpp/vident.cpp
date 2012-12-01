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

#include <stddef.h>

#include "debug.hpp"

#include "gnxconstants.hpp"
#include "mishap.hpp"
#include "vident.hpp"
#include "package.cpp"

using namespace std;

VIdent::VIdent( CodeGen codegen, shared< Ginger::Mnx > vid ) {
	const string & nm = vid->name();
	#ifdef DBG_VIDENT
		cerr << "VIdent::VIdent" << endl;
		cerr << "  " << nm << endl;
		cerr << "  [[";
		vid->render( cerr );
		cerr << "]]" << endl;
	#endif
	const bool nmIsID = nm == ID;
	if ( nmIsID || nm == VAR ) {
		if ( vid->hasAttribute( VID_SCOPE, "local" ) ) {
			this->flavour = LOCAL_FLAVOUR;
			this->slot = vid->attributeToInt( VID_SLOT );
			this->valof = NULL;
		} else {	
			this->flavour = GLOBAL_FLAVOUR;
			#ifdef DBG_VIDENT
				cerr << "VIdent::VIdent > getPackage" << endl;
				cerr << "  " << vid->attribute( VID_DEF_PKG, "<def.pkg undefined>" ) << endl;
				cerr << "  ok" << endl;
				cerr << flush;
			#endif
			const string dpnm = vid->attribute( VID_DEF_PKG );
			Package * def_pkg = codegen->vm->getPackage( dpnm );
			#ifdef DBG_VIDENT
				cerr << "VIdent::VIdent < getPackage" << endl;
			#endif
			//	TODO: This is interfering with correct autoloading.
			this->valof = def_pkg->fetchValof( vid->attribute( VID_NAME ), nmIsID );
			
			this->slot = 0;
		}
	} else if ( nm == CONSTANT ) {
		this->flavour = CONSTANT_FLAVOUR;
		this->valof = NULL;
		this->slot = NULL;
		this->ref = codegen->calcConstant( vid );
	} else {
		throw "TODO";
	}
}
