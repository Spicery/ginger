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

#define NAME "name"
#define DEFAULT "default"
#define PROPERTY "property"
#define NODE "node"
#define START "start"
#define ANY_MATCHER "any"
#define PUSH "push"
#define VALUE "value"
#define FROM_PROPERTY "from.property"
#define EMIT "emit"
#define GOTO "go"
#define TO "to"
#define RESET "reset"
#define DEHEX "dehex"
#define EOF_MATCHER "eof"
#define INCLUDE_MATCHER "include"
#define IS_ALPHA_MATCHER "isalpha"
#define IS_DIGIT_MATCHER "isdigit"
#define IS_ALNUM_MATCHER "isalnum"
#define IS_PUNCT_MATCHER "ispunct"
#define IS_GRAPH_MATCHER "isgraph"
#define IS_CNTRL_MATCHER "iscntrl"
#define IS_LOWER_MATCHER "islower"
#define IS_UPPER_MATCHER "isupper"
#define IS_SPACE_MATCHER "isspace"
#define IS_PRINT_MATCHER "isprint"


void Grammar::start() {
	cout << "<item.stream";
	for ( int n = 0; n < this->state->count(); n++ ) {
		string & name = this->state->name( n );
		string & def = this->state->def( n );
		cout << " " << name << "=\"";
		gnxRenderText( cout, def );
		cout << "\"";
	}
	cout << ">" << endl;
}

void Grammar::processChar( char & ch ) {
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
			return;
		}
	}
}

void Grammar::processEof() {
	Node * current = this->nodes[ this->state->nodeIndex() ];
	if ( current->eofArc() != NULL ) {
		current->eofArc()->runActions( (char)0 );
	}
}

void Grammar::finish() {
	cout << "</item.stream>" << endl;
}

void Grammar::initEmptyNodes(
	State * state, 
	SharedGnx description,
	vector< SharedGnx > & node_desc
) {
	GnxChildIterator it( description );
	while ( it.hasNext() ) {
		SharedGnx d( it.next() );
		if ( d->hasName( PROPERTY ) ) {
			//cerr << "Name = " << d->name() << endl;
			state->addProperty( d->attribute( NAME ), d->attribute( DEFAULT ) );
		} else if ( d->hasName( NODE ) ) {
			Node * node = new Node( d->attribute( NAME ) );
			this->node_index[ d->attribute( NAME ) ] = this->node_count++;
			this->nodes.push_back( node );
			node_desc.push_back( d );
		} else {
			throw Mishap( "Unrecognised element name in LinX grammar" ).culprit( "Name", d->name() );
		}
	}
}

void Grammar::initStartNode( SharedGnx description ) {
	state->nodeIndex() = description->hasAttribute( START ) ? this->node_index[ description->attribute( START ) ] : 0;
}

void Grammar::initMatch( Node * node, SharedGnx arcd, Arc * arc ) {
	if ( arcd->hasAttribute( EOF_MATCHER ) ) {
		node->eofArc() = arc;
	} else {
		node->addArc( arc );
		if ( arcd->hasAttribute( ANY_MATCHER ) ) {
			arc->match() = new AnyMatch();
		} else if ( arcd->hasAttribute( INCLUDE_MATCHER ) ) {
			arc->match() = new IncludeMatch( arcd->attribute( INCLUDE_MATCHER ) );
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
			GnxEntryIterator it( arcd );
			while ( it.hasNext() ) {
				GnxEntry & p = it.next();
				mishap.culprit( "Attribute", p.first );
			}
			throw mishap;
		}
	}
}

void Grammar::initAction(
	SharedGnx actiond,
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
	vector< SharedGnx > & node_desc
) {
	for ( unsigned int i = 0; i < node_desc.size(); i++ ) {
		SharedGnx d( node_desc[ i ] );
		Node * node = this->nodes[ i ];
		
		GnxChildIterator arcs( d );
		while ( arcs.hasNext() ) {
			SharedGnx arcd( arcs.next() );
			//cerr << "ARC " << arc->name() << endl;
			Arc * arc = new Arc();
			this->initMatch( node, arcd, arc );
	
			GnxChildIterator actions( arcd );
			while ( actions.hasNext() ) {
				SharedGnx actiond( actions.next() );
				this->initAction( actiond, arc );
			}
		}		
	}
}


Grammar::Grammar( State * state, SharedGnx description ) :
	state( state ),
	node_count( 0 )
{
	vector< SharedGnx > node_desc;
	
	//	Create the initial nodes as empty.
	this->initEmptyNodes( state, description, node_desc );

	//	Setup the starting node in the global state.
	this->initStartNode( description );
	
	//	Fill in the arcs.
	this->initArcs( node_desc );

}

} // namespace
