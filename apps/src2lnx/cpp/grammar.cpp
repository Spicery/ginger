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

//	Ginger libs
#include "mishap.hpp"

//	Local libs
#include "grammar.hpp"
#include "match.hpp"

namespace SRC2LNX_NS {
using namespace Ginger;
using namespace std;

#define ANY_MATCHER 		"any"
#define CATEGORY			"category"
#define DEFAULT 			"default"
#define DEHEX 				"dehex"
#define DESTINATION			"destination"
#define EMIT 				"emit"
#define EOF_MATCHER 		"eof"
#define FALSE				"false"
#define FROM_PROPERTY 		"from.property"
#define GOTO 				"go"
#define IN_CATEGORY			"in.category"
#define INCLUDE_MATCHER 	"include"
#define IS_ALNUM_MATCHER 	"isalnum"
#define IS_ALPHA_MATCHER 	"isalpha"
#define IS_CNTRL_MATCHER 	"iscntrl"
#define IS_DIGIT_MATCHER 	"isdigit"
#define IS_GRAPH_MATCHER 	"isgraph"
#define IS_LOWER_MATCHER 	"islower"
#define IS_PRINT_MATCHER 	"isprint"
#define IS_PUNCT_MATCHER 	"ispunct"
#define IS_SPACE_MATCHER 	"isspace"
#define IS_UPPER_MATCHER 	"isupper"
#define KEY					"key"
#define LINE_NUMBER			"line.number"
#define MAP		 			"map"
#define NAME 				"name"
#define NODE 				"node"
#define OUTPUT 				"output"
#define PROPERTY 			"property"
#define PUSH 				"push"
#define PUSHBACK 			"pushback"
#define RESET 				"reset"
#define SOURCE				"source"
#define START 				"start"
#define TITLE 				"title"
#define TO 					"to"
#define VALUE				"value"


void Grammar::start() {
	this->state->emitHead();
}

bool Grammar::processChar( const char ch ) {
	Node * current = this->nodes[ this->state->nodeIndex() ];
	vector< Arc * > & arcs = current->arcs();
	for ( 
		vector< Arc * >::iterator it = arcs.begin();
		it != arcs.end();
		++it
	) {	
		Arc * current_arc = *it;
		Match * match = current_arc->match();
		if ( match->matches( ch ) ) {
			//cerr << "Found a matching option! Hurruh!" << endl;
			current_arc->runActions( ch );
			return false;
		}
	}
	throw Mishap( "Unexpected character" ).culprit( "Character", ch ).culprit( "While processing", current->title() );
}

void Grammar::processEof() {
	Node * current = this->nodes[ this->state->nodeIndex() ];
	if ( current->eofArc() != NULL ) {
		current->eofArc()->runActions( (char)0 );
	}
}

void Grammar::finish() {
	this->state->emitTail();
	//cout << "</item.stream>" << endl;
}

void Grammar::initEmptyNodes(
	State * state, 
	SharedMnx description,
	vector< SharedMnx > & node_desc,
	vector< SharedMnx > & mapping_desc,
	map< string, string > & named_categories
) {
	state->lineno_needed = false;
	MnxChildIterator it( description );
	while ( it.hasNext() ) {
		SharedMnx d( it.next() );
		if ( d->hasName( PROPERTY ) ) {
			if ( d->hasAttribute( DEFAULT ) ) {
				state->addProperty( d->attribute( NAME ), d->attribute( DEFAULT ), !d->hasAttribute( OUTPUT, FALSE ) );
			} else if ( d->hasAttribute( LINE_NUMBER ) ) {
				state->lineno_needed = true;
				state->lineno_property = d->attribute( NAME );
			} else {
				throw Mishap( "Unrecognised property description in LinX grammar" ).culprit( "Element", d->toString() );
			}
		} else if ( d->hasName( MAP ) ) {
			if ( d->hasAttribute( SOURCE ) && d->hasAttribute( DESTINATION ) ) {
				mapping_desc.push_back( d );
			} else {
				throw Mishap( "Mapping missing source or destination in LinX grammar" ).culprit( "Element", d->toString() );
			}
		} else if ( d->hasName( NODE ) ) {
			Node * node = new Node( d->attribute( NAME ) );
			this->node_index[ d->attribute( NAME ) ] = this->node_count++;
			this->nodes.push_back( node );
			if ( d->hasAttribute( TITLE ) ) {
				node->title() = d->attribute( TITLE );
			}
			node_desc.push_back( d );
		} else if ( d->hasName( CATEGORY ) ) {
			if ( d->hasAttribute( NAME ) && d->hasAttribute( INCLUDE_MATCHER ) ) {
				named_categories[ d->attribute( NAME ) ] = d->attribute( INCLUDE_MATCHER );
			} else {
				throw Mishap( "Category missing name or include attribute in LinX grammar" ).culprit( "Element", d->toString() );				
			}
		} else {
			throw Mishap( "Unrecognised element name in LinX grammar" ).culprit( "Name", d->name() );
		}
	}
}

void Grammar::initStartNode( SharedMnx description ) {
	state->nodeIndex() = description->hasAttribute( START ) ? this->node_index[ description->attribute( START ) ] : 0;
}

void Grammar::initMatch( 
	Node * node, 
	SharedMnx arcd, 
	Arc * arc, 
	map< string, string > & named_categories 
) {
	if ( arcd->hasAttribute( EOF_MATCHER ) ) {
		node->eofArc() = arc;
	} else {
		node->addArc( arc );
		if ( arcd->hasAttribute( ANY_MATCHER ) ) {
			arc->match() = new AnyMatch();
		} else if ( arcd->hasAttribute( INCLUDE_MATCHER ) ) {
			arc->match() = new IncludeMatch( arcd->attribute( INCLUDE_MATCHER ) );
		} else if ( arcd->hasAttribute( IN_CATEGORY ) ) {
			map< string, string >::iterator it = named_categories.find( arcd->attribute( IN_CATEGORY ) );
			if ( it != named_categories.end() ) {
				arc->match() = new IncludeMatch( it->second );
			} else {
				throw Mishap( "Unrecognised category" ).culprit( "Name", arcd->attribute( IN_CATEGORY ) );
			}
		} else if ( arcd->hasAttribute( IS_SPACE_MATCHER ) ) {
			arc->match() = new IsSpaceMatch();
		} else if ( arcd->hasAttribute( IS_ALPHA_MATCHER ) ) {
			arc->match() = new IsAlphaMatch();
		} else if ( arcd->hasAttribute( IS_DIGIT_MATCHER ) ) {
			arc->match() = new IsDigitMatch();
		} else if ( arcd->hasAttribute( IS_ALNUM_MATCHER ) ) {
			arc->match() = new IsAlnumMatch();
		} else if ( arcd->hasAttribute( IS_PUNCT_MATCHER ) ) {
			arc->match() = new IsPunctMatch();
		} else if ( arcd->hasAttribute( IS_GRAPH_MATCHER ) ) {
			arc->match() = new IsGraphMatch();
		} else if ( arcd->hasAttribute( IS_CNTRL_MATCHER ) ) {
			arc->match() = new IsCntrlMatch();
		} else if ( arcd->hasAttribute( IS_LOWER_MATCHER ) ) {
			arc->match() = new IsLowerMatch();
		} else if ( arcd->hasAttribute( IS_UPPER_MATCHER ) ) {
			arc->match() = new IsUpperMatch();
		} else if ( arcd->hasAttribute( IS_PRINT_MATCHER ) ) {
			arc->match() = new IsPrintMatch();
		} else 	{
			Mishap mishap( "Unrecognised arc-type" );
			MnxEntryIterator it( arcd );
			while ( it.hasNext() ) {
				MnxEntry & p = it.next();
				mishap.culprit( "Attribute", p.first );
			}
			throw mishap;
		}
	}
}

void Grammar::initAction(
	SharedMnx actiond,
	Arc * arc
) {
	if ( actiond->hasName( PUSH ) ) {
		if ( actiond->hasAttribute( PROPERTY ) ) {	
			if ( actiond->hasAttribute( VALUE ) ) {
				arc->addAction( 
					new ValuePushAction( 
						this->state, 
						actiond->attribute( PROPERTY ), 
						actiond->attribute( VALUE ) 
					)
				);
			} else if ( actiond->hasAttribute( FROM_PROPERTY ) ) {
				arc->addAction( 
					new FromPropertyPushAction( 
						this->state, 
						actiond->attribute( PROPERTY ), 
						actiond->attribute( FROM_PROPERTY ) 
					)
				);
			} else {
				arc->addAction( new CucharPushAction( this->state, actiond->attribute( PROPERTY ) ) );
			}
		} else {
			cerr << "Bad node" << endl;
			throw;
		}
	} else if ( actiond->hasName( PUSHBACK ) ) {
		arc->addAction( new PushBackAction( this->state ) );
	} else if ( actiond->hasName( EMIT ) ) {
		arc->addAction( new EmitAction( this->state ) );
	} else if ( actiond->hasName( GOTO ) ) {
		if ( actiond->hasAttribute( TO ) ) {
			int n = this->node_index[ actiond->attribute( TO ) ];
			arc->addAction( new GoAction( this->state, n ) );
		} else {
			cerr << "Bad node" << endl;
			throw;
		}
	} else if ( actiond->hasName( RESET ) ) {
		if ( actiond->hasAttribute( PROPERTY ) ) {
			arc->addAction( new ResetAction( this->state, actiond->attribute( PROPERTY ) ) );
		} else {
			cerr << "Bad node" << endl;
			throw;			
		}
	} else if ( actiond->hasName( DEHEX ) ) {
		if ( actiond->hasAttribute( PROPERTY ) ) {
			arc->addAction( new DehexAction( this->state, actiond->attribute( PROPERTY ) ) );
		} else {
			cerr << "Bad node" << endl;
			throw;			
		}
	} else {
		cerr << "Unrecognised action " << actiond->name() << endl;
	}	
}

void Grammar::initArcs(
	vector< SharedMnx > & node_desc,
	map< string, string > & named_categories
) {
	for ( unsigned int i = 0; i < node_desc.size(); i++ ) {
		SharedMnx d( node_desc[ i ] );
		Node * node = this->nodes[ i ];
		
		MnxChildIterator arcs( d );
		while ( arcs.hasNext() ) {
			SharedMnx arcd( arcs.next() );
			//cerr << "ARC " << arc->name() << endl;
			Arc * arc = new Arc();
			this->initMatch( node, arcd, arc, named_categories );
	
			MnxChildIterator actions( arcd );
			while ( actions.hasNext() ) {
				SharedMnx actiond( actions.next() );
				this->initAction( actiond, arc );
			}
		}		
	}
}

void Grammar::initMappings( vector< SharedMnx > & mapping_desc ) {
	for ( unsigned int i = 0; i < mapping_desc.size(); i++ ) {
		SharedMnx d( mapping_desc[ i ] );
		//	We have checked it has source and destination attributes.
		int src = this->state->propIndex( d->attribute( SOURCE ) );
		int dst = this->state->propIndex( d->attribute( DESTINATION ) );
		
		Mapping * mapping = new Mapping( src, dst );
		this->state->mappings.push_back( mapping );
		
		MnxChildIterator it( d );
		while ( it.hasNext() ) {
			SharedMnx & putd = it.next();
			if ( putd->hasAttribute( KEY ) && putd->hasAttribute( VALUE ) ) {
				mapping->put( putd->attribute( KEY ), putd->attribute( VALUE ) );
			} else {
				throw Mishap( "Malformed 'put' in LinX grammar" ).culprit( "Element", putd->toString() );
			}
		}
	}	
}

Grammar::Grammar( State * state, SharedMnx description ) :
	state( state ),
	node_count( 0 )
{
	vector< SharedMnx > node_desc;
	vector< SharedMnx > mapping_desc;
	map< string, string > named_categories;
	
	//	Create the initial nodes as empty.
	this->initEmptyNodes( state, description, node_desc, mapping_desc, named_categories );

	//	Setup the starting node in the global state.
	this->initStartNode( description );
	
	//	Fill in the mappings.
	this->initMappings( mapping_desc );
	
	//	Fill in the arcs.
	this->initArcs( node_desc, named_categories );

}

} // namespace
