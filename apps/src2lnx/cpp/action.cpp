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

//	STL
#include <iostream>
#include <cstdlib>

//	Local libs
#include "action.hpp"

namespace SRC2LNX_NS {
using namespace std;


CucharPushAction::CucharPushAction( State * state, const std::string & name ) :
	Action( state ),
	prop_index( this->state->propIndex( name ) )
{}

void CucharPushAction::run( char ch ) {
	 this->state->pushToIndex( this->prop_index, ch );
}

ValuePushAction::ValuePushAction( State * state, const std::string & name, const std::string & value ) :
	Action( state ),
	prop_index( this->state->propIndex( name ) ),
	value( value )
{}

void ValuePushAction::run( char ch ) {
	 this->state->pushToIndex( this->prop_index, ch );
}

FromPropertyPushAction::FromPropertyPushAction( State * state, const std::string & name, const std::string & from_property ) :
	Action( state ),
	prop_index( this->state->propIndex( name ) ),
	from_prop_index( this->state->propIndex( from_property ) )
{}

void FromPropertyPushAction::run( char ch ) {
	 this->state->pushToIndexFromIndex( this->prop_index, this->from_prop_index );
}

EmitAction::EmitAction( State * state ) :
	Action( state )
{}

void EmitAction::run( char ch ) {
	this->state->emit();
}

ResetAction::ResetAction( State * state, const std::string & property ) :
	Action( state ),
	prop_index( this->state->propIndex( property ) )
{}

void ResetAction::run( char ch ) {
	this->state->reset( this->prop_index );
}

DehexAction::DehexAction( State * state, const std::string & property ) :
	Action( state ),
	prop_index( this->state->propIndex( property ) )
{}

void DehexAction::run( char ch ) {
	char * p;
	unsigned long u = strtoul( this->state->propValue( this->prop_index ).c_str(), &p, 16 );
	this->state->propValue( this->prop_index ).resize( 0 );
	this->state->pushToIndex( this->prop_index, (char)u );
}

GoAction::GoAction( State * state, int node_index ) :
	Action( state ),
	node_index( node_index )
{}	

void GoAction::run( char ch ) {
	//cerr << "Changing state from " << this->state->nodeIndex() << " to " << this->node_index << endl;
	this->state->nodeIndex() = this->node_index;
}



} // namespace
