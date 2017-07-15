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

#ifndef TOKTYPE_HPP
#define TOKTYPE_HPP

namespace LNX2MNX_NS {

/*
	These are the predefined token tokens. I am not sure what 
	degree of configurability I think they should have as yet.
	
	I think we are looking at a type/subtype classfication. This
	is the main type.
*/

typedef enum TokenType {
	tokty_start,		//	Artificial initial value, signifying no reading done.
	tokty_number,
	tokty_string,		//	Probably needs to subtype on quote used.
	tokty_sign,			//	Custom subtyping.
	tokty_name,			//	Custom subtyping.
	tokty_end			//	Artificial final value, signifying end of file.
} TokType;


const char *tok_type_name( TokType fnc );
enum TokenType name_to_tok_type( const char * ty );

class TokenTypeMask {
private:
	unsigned int mask;
public:
	void add( const char * name );
	void add( const std::string & name );
	void add( enum TokenType ty );
	bool contains( enum TokenType ty );
public:
	TokenTypeMask() : mask( 0 ) {}
};

} // namespace

#endif

