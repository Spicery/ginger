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

//	C++ STD
#include <iostream>
#include <vector>
#include <sstream>

//	AppGinger Libraries
#include "mishap.hpp"
#include "gnx.hpp"

//	This project header files.
#include "parser.hpp"
#include "item.hpp"

namespace XSON2GNX {
using namespace Ginger;
using namespace std;

#define GRAMMAR				"grammar"
#define START				"start"
#define GROUP 				"group"
#define RULE 				"rule"
#define STATE				"state"
#define ELEMENT				"element"
#define NAME				"name"
#define PUT					"put"
#define MUST_READ			"must.read"
#define IF_READ				"if.read"
#define IF_PEEK				"if.peek"
#define UNLESS_READ			"unless.read"
#define UNLESS_PEEK			"unless.peek"
#define PARSE				"parse"
#define PARSE_LIST_READ		"parse.list.read"
#define PARSE_LIST_PEEK		"parse.list.peek"
#define KEY					"key"
#define VALUE				"value"
#define VALUE_FROM			"value.from"
#define ROLE_EQ				"role.eq"
#define TOKEN_EQ			"token.eq"
#define TOKEN_NEQ			"token.neq"
#define DEFAULT				"default"
#define SAVE				"save"
#define RESTORE				"restore"
#define WHILE_READ			"while.read"
#define WHILE_PEEK			"while.peek"
#define UNTIL_READ			"until.read"
#define UNTIL_PEEK			"until.peek"
#define SEPARATOR			"separator"
#define PRECEDENCE			"precedence"
#define PRECEDENCE_FROM		"precedence.from"
#define PRECEDENCE_GTE		"precedence.gte"

XSONParser::XSONParser( ItemFactory itemf, SharedGnx grammar ) : 
	itemf( itemf )
{
	this->init( grammar );
	this->start = grammar->attribute( START );
}

void XSONParser::init( SharedGnx grammar ) {
	this->loadRules( grammar );
}

Item XSONParser::peek() {
	return this->itemf->peek();
}

void XSONParser::drop() {
	this->itemf->drop();
}


void XSONParser::loadRules( SharedGnx g ) {
	if ( g->hasName( RULE ) ) {
		this->load1Rule( g );
	} else if ( g->hasName( GROUP ) || g->hasName( GRAMMAR ) ) {
		for ( int n = 0; n < g->size(); n++ ) {
			this->loadRules( g->child( n ) );
		}
	} else {
		throw Mishap( "Unrecognised element in grammar" ).culprit( "Element name", g->name() );
	}
}

void XSONParser::load1Rule( SharedGnx g ) {
	if ( ! g->hasAttribute( STATE ) ) {
		throw Mishap( "Misformed rule" );
	}
	this->rules[ g->attribute( STATE ) ].push_back( g );
}

std::vector< Ginger::SharedGnx > & XSONParser::getRule( const std::string & state ) {
	return this->rules[ state ];
}


SharedGnx XSONParser::parse() {
	ItemClass item( *this->peek() );
	RuleParser p( this, this->start, 0, &item );
	p.parse();
	//this->parseFromState( this->start );
	return this->builder.build();
}

RuleParser::RuleParser( 
	XSONParser * parent, 
	const std::string & state, 
	float precedence, 
	Item item 
) :
	parent( parent ),
	builder( parent->getBuilder() ),
	state( state ),
	precedence( precedence ),
	item( item )
{}

Item RuleParser::peek() {
	return this->parent->peek();
}

void RuleParser::drop() {
	this->parent->drop();
}

void RuleParser::parse() {
	cerr << ">> " << this->state << " with prec " << this->precedence << endl;
	SharedGnx rule( this->findMatchingRule( this->state, this->item, true ) );
	this->processChildren( rule );
	cerr << "<< " << state << endl;
}

void RuleParser::parseFromState( const std::string & state, float prec ) {
	ItemClass item( *this->peek() );
	RuleParser r( this->parent, state, prec, &item );
	r.parse();
}
			
void RuleParser::parseFromState( const std::string & state ) {
	this->parseFromState( state, this->precedence );
}
			
void RuleParser::processChildren( SharedGnx & rule ) {
	for ( int i = 0; i < rule->size(); i++ ) {
		SharedGnx & action = rule->child( i );
		this->processAction( action );
	}
}

void RuleParser::processAction( SharedGnx & action ) {
	if ( action->hasName( ELEMENT ) ) {
		this->elementAction( action );
	} else if ( action->hasName( PUT ) ) {
		this->putAction( action );
	} else if ( action->hasName( MUST_READ ) ) {
		this->mustReadAction( action );
	} else if ( action->hasName( IF_PEEK ) ) {
		this->ifUnlessAction( action, true, false );
	} else if ( action->hasName( IF_READ ) ) {
		this->ifUnlessAction( action, true, true );
	} else if ( action->hasName( UNLESS_PEEK ) ) {
		this->ifUnlessAction( action, false, false );
	} else if ( action->hasName( UNLESS_READ ) ) {
		this->ifUnlessAction( action, false, true );
	} else if ( action->hasName( PARSE ) ) {
		this->parseAction( action );
	} else if ( action->hasName( PARSE_LIST_READ ) ) {
		this->parseListAction( action, true );
	} else if ( action->hasName( PARSE_LIST_PEEK ) ) {
		this->parseListAction( action, false );
	} else if ( action->hasName( WHILE_READ ) ) {
		this->whileUntilAction( action, true, true );
	} else if ( action->hasName( WHILE_PEEK ) ) {
		this->whileUntilAction( action, true, false );
	} else if ( action->hasName( UNTIL_READ ) ) {
		this->whileUntilAction( action, false, true );
	} else if ( action->hasName( UNTIL_PEEK ) ) {
		this->whileUntilAction( action, false, false );
	} else if ( action->hasName( SAVE ) ) {
		this->saveAction();
	} else if ( action->hasName( RESTORE ) ) {
		this->restoreAction();
	} else {
		cerr << "Don't know what do do with this action: " << action->name() << endl;
	}
}

void RuleParser::parseAction( SharedGnx & action ) {
	const string & state = action->attribute( STATE );
	if ( action->hasAttribute( PRECEDENCE ) ) {
		float f;
		istringstream ss( action->attribute( PRECEDENCE ) );
		ss >> f;
		this->parseFromState( state, f );
	} else if ( action->hasAttribute( PRECEDENCE_FROM ) ) {
		this->parseFromState( state, this->item->asPrecedence() );
	} else {
		this->parseFromState( state );
	}
}

void RuleParser::elementAction( SharedGnx & action ) {
	this->builder.start( action->attribute( NAME ) );
	this->processChildren( action );
	this->builder.end();
}

void RuleParser::putAction( SharedGnx & action ) {
	if ( action->hasAttribute( VALUE ) ) {
		this->builder.put( action->attribute( KEY ), action->attribute( VALUE ) );
	} else if ( action->hasAttribute( VALUE_FROM ) ) {
		this->builder.put( 
			action->attribute( KEY ), 
			item->asFeature( action->attribute( VALUE_FROM ) ) 
		);
	} else {
		cerr << "Malformed thingy" << endl;
	}
}

void RuleParser::mustReadAction( SharedGnx & action ) { 
	ItemClass item( *this->peek() );
	if ( !this->evaluateCondition( &item, action, true ) ) {
		throw Mishap( "Unexpected token" ).culprit( "Item", item.asValue() );
	}
}


void RuleParser::ifUnlessAction( SharedGnx & action, const bool ifVsUnless, const bool readVsPeek ) {
	ItemClass item( *this->peek() );
	if ( ifVsUnless == this->evaluateCondition( &item, action, readVsPeek ) ) {
		this->processChildren( action );
	}	
}

void RuleParser::whileUntilAction( SharedGnx & action, const bool whileVsUntil, const bool readVsPeek ) { 
	for (;;) {
		ItemClass item( *this->peek() );
		if ( whileVsUntil != this->evaluateCondition( &item, action, readVsPeek ) ) break;
		this->processChildren( action );
	}
}

void RuleParser::parseListAction( SharedGnx & action, const bool readVsPeek ) {
	const string & state = action->attribute( STATE );
	const string & sep = action->attribute( SEPARATOR );
	
	ItemClass item1( *this->peek() );
	if ( not this->evaluateCondition( &item1, action, false ) ) {
		for (;;) {
			this->parseFromState( state );
			Item t = this->peek();
			if ( t->asValue() != sep ) break;
			this->parent->drop();
		}
	}
	Item itemN = this->parent->peek();
	if ( not this->evaluateCondition( itemN, action, readVsPeek ) ) {
		throw Mishap( "Unexpected token" ).culprit( "Item", itemN->asValue() );
	}
}

void RuleParser::saveAction() {
	this->builder.save();
}

void RuleParser::restoreAction() {
	this->builder.restore();
}

SharedGnx RuleParser::findMatchingRule( const string & state, const Item item, const bool throwVsReturnNull ) {
	vector< SharedGnx > & rv = this->parent->getRule( state );
	cerr << "Checking " << rv.size() << " rules" << endl;
	for ( 
		vector< SharedGnx >::iterator it = rv.begin();
		it != rv.end();
		++it
	) {
		SharedGnx & g = *it;
		if ( this->evaluateCondition( item, g, true ) ) {
			cerr << "Found matching rule" << endl;
			return g;
		}
	}
	if ( throwVsReturnNull ) {
		throw Mishap( "Parse failed" );
	} else {
		return shared< Gnx >();
	}
}

bool RuleParser::evaluateCondition( Item item, SharedGnx & g, const bool readVsPeek ) {
	if ( g->hasAttribute( ROLE_EQ ) ) {
		RoleMatch match;
		match.addNamedRole( g->attribute( ROLE_EQ ) );
		if ( match.contains( item ) ) {
			if ( readVsPeek ) this->drop();
			return true;
		} else {
			return false;
		}
	} else if ( g->hasAttribute( TOKEN_EQ ) ) {
		if ( item->asValue() == g->attribute( TOKEN_EQ ) ) {
			if ( readVsPeek ) this->drop();
			return true;
		} else {
			return false;
		}
	} else if ( g->hasAttribute( TOKEN_NEQ ) ) {
		if ( item->asValue() != g->attribute( TOKEN_NEQ ) ) {
			if ( readVsPeek ) this->drop();
			return true;
		} else {
			return false;
		}
	} else if ( g->hasAttribute( PRECEDENCE_GTE ) ) {
		cerr << " - checking precedence" << endl;
		cerr << "   1. " <<  item->asPrecedence() << endl;
		cerr << "   2. " << this->precedence  << endl;
		cerr << "   =  " << ( item->asPrecedence() >= this->precedence ) << endl;
		if ( item->asPrecedence() >= this->precedence ) {
			if ( readVsPeek ) this->drop();
			return true;
		} else {
			return false;
		}
	} else if ( g->hasAttribute( DEFAULT ) ) {
		cerr << "Found matching rule [default]" << endl;
		return true;
	} else {
		cerr << "Don't know how to do find-match for this rule" << endl;
		g->render();
		cerr << endl;
		return false;
	}
}


} // namespace XSON2GNX {
