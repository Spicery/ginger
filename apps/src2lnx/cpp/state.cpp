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

//	Ginger library
#include "mishap.hpp"
#include "mnx.hpp"

//	Local libs
#include "state.hpp"
#include "mapping.hpp"


namespace SRC2LNX_NS {
using namespace std;
using namespace Ginger;

void State::addProperty( const string & name, const string & def, bool output ) {
	map< string, int >::iterator it = this->property_index.find( name );
	if ( it == this->property_index.end() ) {
		this->property_index[ name ] = this->prop_count++;
		this->property_name.push_back( name );
		this->property_value.push_back( def );
		this->property_default.push_back( def );
		this->property_output.push_back( output );
	} else {
		throw Mishap( "Repeated attribute" ).culprit( "Name", name ).culprit( "Default", def );
	}
}

std::string & State::name( int n ) {
	return this->property_name[ n ];
}

std::string & State::def( int n ) {
	return this->property_default[ n ];
}

int State::propIndex( const std::string & name ) {
	map< string, int >::iterator it = this->property_index.find( name );
	if ( it != this->property_index.end() ) {
		return this->property_index[ name ];
	} else {
		throw Mishap( "Undeclared property" ).culprit( "Property", name );
	}
}

void State::pushToIndex( int n, char ch ) {
	this->property_value[ n ].push_back( ch );
}

void State::pushToIndex( int n, const std::string & value ) {
	this->property_value[ n ].append( value );
}

void State::pushToIndexFromIndex( int to_index, int from_index ) {
	this->property_value[ to_index ].append( this->property_value[ from_index ] );
}

void State::reset( int index ) {
	this->property_value[ index ] = this->property_default[ index ];
}

void State::runMappings() {
	for (
		vector< Mapping * >::iterator it = this->mappings.begin();
		it != this->mappings.end();
		++it
	) {
		(*it)->update( this );
	}
}

void State::emit() {
	this->runMappings();
	this->emitContents();
}

void State::emitContents() {
	cout << "<.";
	for ( int i = 0; i < this->prop_count; i++ ) {
		if ( this->property_output[ i ] ) {
			cout << " " << this->property_name[ i ] << "=\"";
			mnxRenderText( cout, this->property_value[ i ] );
			this->property_value[ i ] = this->property_default[ i ];
			cout << "\"";
		}
	}
	if ( this->lineno_needed ) {
		cout << " " << this->lineno_property << "=\"";
		cout <<	this->lineno;
		cout << "\"";
	}
	cout << "/>" << endl;
}

void State::emitHead() {
	cout << "<item.stream";
	for ( int i = 0; i < this->prop_count; i++ ) {
		if ( this->property_output[ i ] ) {
			string & name = this->property_name[ i ];
			string & def = this->property_default[ i ];
			cout << " " << name << "=\"";
			mnxRenderText( cout, def );
			cout << "\"";
		}
	}
	if ( this->lineno_needed ) {
		cout << " " << this->lineno_property << "=\"\"";
	}
	cout << ">" << endl;
}

void State::emitTail() {
	cout << "</item.stream>" << endl;
}

State::State() :
	lineno_needed( false ),
	node_index( 0 ),
	prop_count( 0 )
{}

} // SRC2LNX_NS
