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
#include <string>


#include "search.hpp"
#include "gnxconstants.hpp"

#include "resolvervisitor.hpp"

#define DBG_FETCHGNX_RESOLVER_VISITOR

using namespace std;

ResolverVisitor::ResolverVisitor( Search * search, const string &  ) :
    search( search )
{
    //  Nothing.
}

//
//  We are looking for nodes marked as fetchgnx="resolve". These will
//  additionally be marked with the enclosing package (enc.pkg), the
//  identifer name (name) and optionally an import alias (alias).
//  If marked with an alias then it is a qualified-resolution that is
//  required, otherwise an unqualified resolution.
//
void ResolverVisitor::startVisit( Ginger::Mnx & element ) {
    #ifdef DBG_FETCHGNX_RESOLVER_VISITOR
        cerr << "Resolving " << element.name() << endl;
    #endif
    if ( 
        //element.hasAttribute( "fetchgnx", "resolve" ) &&
        element.hasAttribute( GNX_VID_ENC_PKG ) &&
        element.hasAttribute( GNX_VID_NAME )
    ) {
        //  We haz one!
        const string name = element.attribute( GNX_VID_NAME );
        const string enc_pkg = element.attribute( GNX_VID_ENC_PKG );
        if ( element.hasAttribute( GNX_VID_ALIAS ) ) {
            const string alias = element.attribute( GNX_VID_ALIAS );
            #ifdef DBG_FETCHGNX_RESOLVER_VISITOR
                cerr << "Gotcha: qualified reference" << endl;
            #endif
            const string def_pkg = this->search->findDefPkgQualified( enc_pkg, alias, name );
            element.putAttribute( GNX_VID_DEF_PKG, def_pkg );
        } else {
            #ifdef DBG_FETCHGNX_RESOLVER_VISITOR
                cerr << "Gotcha: unqualified reference" << endl;
            #endif
            const string def_pkg = this->search->findDefPkgUnqualified( enc_pkg, name );
            element.putAttribute( GNX_VID_DEF_PKG, def_pkg );
        }
        #ifdef DBG_FETCHGNX_RESOLVER_VISITOR
            cerr << ":: ";
            element.render( cerr );
            cerr << endl;
        #endif
    }
}

void ResolverVisitor::endVisit( Ginger::Mnx & element ) {
    //  Nothing.
}
