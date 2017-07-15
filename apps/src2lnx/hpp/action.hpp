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

#ifndef SRC2LNX_ACTION_HPP
#define SRC2LNX_ACTION_HPP

//	Local libs
#include "state.hpp"


namespace SRC2LNX_NS {

class Action {
protected:
	State * state;
	
public:
	virtual void run( char ch ) = 0;
	
public:
	Action( State * state ) : state( state ) {}
};

class CucharPushAction : public Action {
private:
	int prop_index;
public:
	void run( char ch );
public:
	CucharPushAction( State * state, const std::string & name );
};

class ValuePushAction : public Action {
private:
	int prop_index;
	std::string value;
public:
	void run( char ch );
public:
	ValuePushAction( State * state, const std::string & name, const std::string & value );
};

class PushBackAction : public Action {
public:
	void run( char ch );
public:
	PushBackAction( State * state );
};

class FromPropertyPushAction : public Action {
private:
	int prop_index;
	int from_prop_index;
public:
	void run( char ch );
public:
	FromPropertyPushAction( State * state, const std::string & name, const std::string & from_property );
};

class EmitAction : public Action {
public:
	void run( char ch );
public:
	EmitAction( State * state );
};

class ResetAction : public Action {
private:
	int prop_index;
public:
	void run( char ch );
public:
	ResetAction( State * state, const std::string & property );
};

class GoAction : public Action {
private:
	int node_index;
public:
	void run( char ch );
public:
	GoAction( State * state, int node_index );
};

class DehexAction : public Action {
private:
	int prop_index;
public:
	void run( char ch );
public:
	DehexAction( State * state, const std::string & property );
};

} // namespace

#endif
