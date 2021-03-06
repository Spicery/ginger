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

#ifndef FETCHGNX_RESOLVER_VISITOR_HPP
#define FETCHGNX_RESOLVER_VISITOR_HPP

#include <string>
#include "mnx.hpp"

class ResolverVisitor : public Ginger::MnxVisitor {
private:
    Search * search;
public:
    ResolverVisitor( Search * search, const std::string & enc_pkg );
    void startVisit( Ginger::Mnx & element );
    void endVisit( Ginger::Mnx & element );
};

#endif
