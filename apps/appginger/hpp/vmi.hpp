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

#ifndef VMI_HPP
#define VMI_HPP

#include "destination.hpp"
#include "plant.hpp"
#include "common.hpp"
#include "ident.hpp"
#include "instruction.hpp"

class VmiRelOpFactory {
private:
	Plant plant;
	char flag1;
	Ident ident1;
	int int1;
	char op;
	char flag2;
	Ident ident2;
	int int2;
	
public:
	void setLeft( int arg1 );
	void setLeft( Ident id );
	void setRight( int arg1 );
	void setRight( Ident id );
	void setLT();
	void setGT();
	void setLTE();
	void setGTE();
	void setEQ();
	void setNEQ();
	void negate();
	
private:
	void compilePushLeft();
	void compilePushRight();
	void compileOp();

	
public:
	void ifSo( DestinationClass &dst );
	void ifNot( DestinationClass &dst );
	
public:
	VmiRelOpFactory( Plant p ) :
		plant( p ),
		flag1( '?' ),
		op( '?' ),
		flag2( '?' )
	{
	}
};

#endif

