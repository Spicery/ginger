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

#ifndef DESTINATION_HPP
#define DESTINATION_HPP

#include <vector>

namespace Ginger {

typedef class CodeGenClass * CodeGen;

class DestinationClass {
private:
	std::vector< int >	pending_vector;
	
public:
	CodeGen 			codegen;
	bool				is_set;
	int					location;
	
public:
	DestinationClass( CodeGen codegen );
	
public:
	void destinationSet();
	void destinationInsert();
		
};

typedef DestinationClass * Destination;

} // namespace Ginger

#endif
