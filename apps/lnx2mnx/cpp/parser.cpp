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
#include "mnx.hpp"

//	This project header files.
#include "parser.hpp"
#include "item.hpp"

namespace LNX2MNX_NS {
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
//#define ROLE_EQ				"role.eq"
//#define TOKEN_EQ			"token.eq"
//#define TOKEN_NEQ			"token.neq"
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
#define EQ					"eq"
#define NEQ					"neq"
#define PROPERTY			"property"
#define MISHAP				"mishap"
#define CULPRIT				"culprit"
#define TITLE				"title"

Parser::Parser( LnxReader & itemf, SharedMnx grammar ) : 
	mishapDesc( (Mnx *)NULL ),
	itemf( itemf )
{
	this->init( grammar );
	this->start = grammar->attribute( START );
}

void Parser::init( SharedMnx grammar ) {
	this->loadRules( grammar );
}

LnxItem * Parser::peek() {
	return this->itemf.peek();
}

void Parser::drop() {
	this->itemf.drop();
}


void Parser::loadRules( SharedMnx g ) {
	if ( g->hasName( RULE ) ) {
		this->load1Rule( g );
	} else if ( g->hasName( GROUP ) || g->hasName( GRAMMAR ) ) {
		for ( int n = 0; n < g->size(); n++ ) {
			this->loadRules( g->child( n ) );
		}
	} else if ( g->hasName( MISHAP ) ) {
		this->mishapDesc = g;		
	} else {
		throw Mishap( "Unrecognised element in grammar" ).culprit( "Element name", g->name() );
	}
}

void Parser::load1Rule( SharedMnx g ) {
	if ( ! g->hasAttribute( STATE ) ) {
		throw Mishap( "Misformed rule" );
	}
	this->rules[ g->attribute( STATE ) ].push_back( g );
}

std::vector< Ginger::SharedMnx > & Parser::getRule( const std::string & state ) {
	return this->rules[ state ];
}


SharedMnx Parser::parse() {
	LnxItem item( *this->peek() );
	RuleParser p( this, this->start, 0, &item );
	p.parse();
	return this->builder.build();
}

RuleParser::RuleParser( 
	Parser * parent, 
	const std::string & state, 
	float precedence, 
	LnxItem * item 
) :
	parent( parent ),
	builder( parent->getBuilder() ),
	depth( 0 ),
	state( state ),
	precedence( precedence ),
	item( item )
{}

LnxItem * RuleParser::peek() {
	return this->parent->peek();
}

void RuleParser::drop() {
	this->parent->drop();
}

void RuleParser::parse() {
	depth += 1;
	//cerr << ">> " << this->state << " with prec " << this->precedence << endl;
	SharedMnx rule( this->findMatchingRule( this->state, this->item, true ) );
	this->processChildren( rule );
	//cerr << "<< " << state << endl;
	depth -= 1;
}

void RuleParser::parseFromState( const std::string & state, float prec ) {
	LnxItem * pk = this->peek();
	if ( pk == NULL ) {
		RuleParser r( this->parent, state, prec, pk );
		r.parse();
	} else {
		LnxItem item( *pk );
		RuleParser r( this->parent, state, prec, &item );
		r.parse();
	}
}
			
void RuleParser::parseFromState( const std::string & state ) {
	this->parseFromState( state, this->precedence );
}
			
void RuleParser::processChildren( SharedMnx & rule ) {
	for ( int i = 0; i < rule->size(); i++ ) {
		SharedMnx & action = rule->child( i );
		this->processAction( action );
	}
}

void RuleParser::processAction( SharedMnx & action ) {
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
	} else if ( action->hasName( PARSE ) ) {
		this->parseAction( action );
	} else if ( action->hasName( PARSE_LIST_READ ) ) {
		this->parseListAction( action, true );
	} else if ( action->hasName( PARSE_LIST_PEEK ) ) {
		this->parseListAction( action, false );
	} else if ( action->hasName( SAVE ) ) {
		this->saveAction();
	} else if ( action->hasName( RESTORE ) ) {
		this->restoreAction();
	} else if ( action->hasName( WHILE_READ ) ) {
		this->whileUntilAction( action, true, true );
	} else if ( action->hasName( WHILE_PEEK ) ) {
		this->whileUntilAction( action, true, false );
	} else if ( action->hasName( UNLESS_PEEK ) ) {
		this->ifUnlessAction( action, false, false );
	} else if ( action->hasName( UNLESS_READ ) ) {
		this->ifUnlessAction( action, false, true );
	} else if ( action->hasName( UNTIL_READ ) ) {
		this->whileUntilAction( action, false, true );
	} else if ( action->hasName( UNTIL_PEEK ) ) {
		this->whileUntilAction( action, false, false );
	} else {
		throw Mishap( "Don't know what to do with this action" ).culprit( "Action name", action->name() );
	}
}

static int asPrecedence( LnxItem * item ) {
	//	TO BE DONE.
	return 0;
}

void RuleParser::parseAction( SharedMnx & action ) {
	const string & state = action->attribute( STATE );
	if ( action->hasAttribute( PRECEDENCE ) ) {
		float f;
		istringstream ss( action->attribute( PRECEDENCE ) );
		ss >> f;
		this->parseFromState( state, f );
	} else if ( action->hasAttribute( PRECEDENCE_FROM ) ) {
		this->parseFromState( state, asPrecedence( this->item ) );
	} else {
		this->parseFromState( state );
	}
}

void RuleParser::elementAction( SharedMnx & action ) {
	this->builder.start( action->attribute( NAME ) );
	this->processChildren( action );
	this->builder.end();
}

static string asFeature( LnxItem * item, const string & from ) {
	//	To be done.
	return "";
}

void RuleParser::putAction( SharedMnx & action ) {
	if ( action->hasAttribute( PROPERTY ) ) {
		this->builder.put( 
			action->attribute( KEY ), 
			item->propertyValue( action->attribute( PROPERTY ) ) 
		);
	} else if ( action->hasAttribute( VALUE ) ) {
		this->builder.put( action->attribute( KEY ), action->attribute( VALUE ) );
	} else if ( action->hasAttribute( VALUE_FROM ) ) {
		this->builder.put( 
			action->attribute( KEY ), 
			asFeature( item, action->attribute( VALUE_FROM ) ) 
		);
	} else {
		throw Mishap( "Malformed put" ).culprit( "Action", action->toString() );
	}
}

void RuleParser::unexpectedToken( LnxItem * item ) {
	if ( item == NULL ) {
		throw Mishap( "Unexpected end of input" );
	} else {
		Mishap * mishap = new Mishap( "Unexpected token" );
		if ( !!this->parent->mishapDesc ) {
			MnxChildIterator kids( this->parent->mishapDesc );
			while ( kids.hasNext() ) {
				SharedMnx & culprit = kids.next();
				if ( 
					culprit->hasName( CULPRIT ) && 
					culprit->hasAttribute( PROPERTY ) &&
					culprit->hasAttribute( TITLE )
				) {
					mishap->culprit( 
						culprit->attribute( TITLE ), 
						item->propertyValue( culprit->attribute( PROPERTY ) ) 
					);				
				} else {
					throw Mishap( "Malformed culprit description" ).culprit( "Culprit", culprit->toString() );
				}
			}
		}
		throw *mishap;
	}	
}

void RuleParser::mustReadAction( SharedMnx & action ) { 
	LnxItem * item = this->peek();
	if ( !this->evaluateCondition( item, action, true ) ) {
		this->unexpectedToken( item );
	}
}


void RuleParser::ifUnlessAction( SharedMnx & action, const bool ifVsUnless, const bool readVsPeek ) {
	LnxItem item( *this->peek() );
	if ( ifVsUnless == this->evaluateCondition( &item, action, readVsPeek ) ) {
		this->processChildren( action );
	}	
}

void RuleParser::whileUntilAction( SharedMnx & action, const bool whileVsUntil, const bool readVsPeek ) { 
	for (;;) {
		LnxItem item( *this->peek() );
		if ( whileVsUntil != this->evaluateCondition( &item, action, readVsPeek ) ) break;
		this->processChildren( action );
	}
}

void RuleParser::parseListAction( SharedMnx & action, const bool readVsPeek ) {
	if ( action->size() != 2 ) {
		throw Mishap( "Malformed parse-list" ).culprit( "Element", action->toString() );
	}
	SharedMnx & separator = action->child( 0 );
	SharedMnx & closer = action->child( 1 );
	
	if ( this->peek() == NULL ) {
		throw Mishap( "Unexpected end of input" );
	} else {
		LnxItem item1( *this->peek() );
		if ( not this->evaluateCondition( &item1, closer, false ) ) {
			for (;;) {
				this->parseFromState( state );
				LnxItem * t = this->peek();
				if ( not this->evaluateCondition( t, separator, true ) ) break;   
			}
		}
		
		LnxItem * itemN = this->parent->peek();
		if ( not this->evaluateCondition( itemN, closer, readVsPeek ) ) {
			this->unexpectedToken( itemN );
		}
	}
}

void RuleParser::saveAction() {
	this->builder.save();
}

void RuleParser::restoreAction() {
	this->builder.restore();
}

SharedMnx RuleParser::findMatchingRule( const string & state, LnxItem * item, const bool throwVsReturnNull ) {
	vector< SharedMnx > & rv = this->parent->getRule( state );
	//cerr << "Checking " << rv.size() << " rules" << endl;
	for ( 
		vector< SharedMnx >::iterator it = rv.begin();
		it != rv.end();
		++it
	) {
		SharedMnx & g = *it;
		if ( this->evaluateCondition( item, g, true ) ) {
			//cerr << "Found matching rule" << endl;
			return g;
		}
	}
	if ( throwVsReturnNull ) {
		throw Mishap( "Parser failed" );
	} else {
		return shared< Mnx >();
	}
}

bool RuleParser::evaluateCondition( LnxItem  * item, SharedMnx & g, const bool readVsPeek ) {
	//cerr << "evaluateCondition" << endl;
	//cerr << "  " << g->toString() << endl;
	//cerr << "  " << "item = " << (long)item << endl;
	if ( item == NULL && !g->hasAttribute( DEFAULT ) ) {
		return false;
	} else if ( g->hasAttribute( EQ ) && g->hasAttribute( PROPERTY ) ) {
		//cerr << g->attribute( PROPERTY ) << " (" <<  item->propertyValue( g->attribute( PROPERTY ) ) << ") = '";
		//cerr << g->attribute( EQ ) << "'" << endl;
		const bool test = (
			item != NULL && 
			(
				g->attribute( EQ ) == 
				item->propertyValue( g->attribute( PROPERTY ) )
			)
		);
		if ( test && readVsPeek ) this->drop();
		return test;
	} else if ( g->hasAttribute( NEQ ) && g->hasAttribute( PROPERTY ) ) {
		//cerr << g->attribute( PROPERTY ) << " (" <<  item->propertyValue( g->attribute( PROPERTY ) ) << ") = '";
		//cerr << g->attribute( NEQ ) << "'" << endl;
		const bool test = (
			item == NULL || 
			(
				g->attribute( NEQ ) != 
				item->propertyValue( g->attribute( PROPERTY ) )
			)
		);
		if ( test && readVsPeek ) this->drop();
		return test;
	} else if ( g->hasAttribute( PRECEDENCE_GTE ) ) {
		cerr << " - checking precedence" << endl;
		cerr << "   1. " <<  asPrecedence( item ) << endl;
		cerr << "   2. " << this->precedence  << endl;
		cerr << "   =  " << ( asPrecedence( item ) >= this->precedence ) << endl;
		if ( item != NULL && ( asPrecedence( item ) >= this->precedence ) ) {
			if ( readVsPeek ) this->drop();
			return true;
		} else {
			return false;
		}
	} else if ( g->hasAttribute( DEFAULT ) ) {
		//cerr << "Found matching rule [default]" << endl;
		return true;
	} else {
		//cerr << "Don't know how to do find-match for this rule" << endl;
		//g->render();
		//cerr << endl;
		return false;
	}
}


} // namespace XSON2GNX {
