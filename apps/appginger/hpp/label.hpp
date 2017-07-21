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

#ifndef LABEL_HPP
#define LABEL_HPP

#include <vector>

namespace Ginger {

class CodeGenClass;

class LabelClass {
private:
	std::vector< int >	pending_vector;
	
private:
	CodeGenClass *		codegen;
	bool				is_set;
	bool				is_return;
	int					the_location;
	bool				has_been_used;
	
public:
	int location() const { return this->the_location; }
	CodeGenClass * codeGen() const { return this->codegen; }
	bool isSet() const { return this->is_set; }
	bool isReturn() const { return this->is_return; }
	bool isntReturn() const { return not this->is_return; }
	LabelClass * jumpToJump( LabelClass * contn );
	bool hasBeenUsed() const { return this->has_been_used; }
	
public:
	LabelClass( CodeGenClass * codegen, bool is_return = false ) : 
		codegen( codegen ), 
		is_set( false ), 
		is_return( is_return ), 
		the_location( -1 ) 
	{}
	
public:
	void labelSet();
	void labelInsert();
		
};

typedef LabelClass * Label;

#define CONTINUE_LABEL ((LabelClass*)0)

} // namespace Ginger

#endif

