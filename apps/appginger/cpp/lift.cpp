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

//  Lift is the scope analyser.  It combines two jobs, name resolution
//	and conversion of outer-locals by a process akin to lambda lifting.
//
//    Lifting is a non-trivial task so I am deferring that in order to
//    make progress on more basic issues.

#include "debug.hpp"

#include <iostream>
#include <vector>
#include <list>
using namespace std;

#include "lift.hpp"
#include "mishap.hpp"
#include "ident.hpp"
#include "package.hpp"
#include "term.hpp"

//#define DBG_LIFT
#ifdef DBG_LIFT
    #include <stdio.h>
#endif


//-- lift state --------------------------------------------------------

class LiftStateClass {
public:
    vector< FnTermClass * > ancestors;
    vector< list< Ident > > substitutes;
    
private:    
	void capturingFunctions( Ident & outer, list< int > & index_of_functions );
    Ident lookup( const string & name );
	Term generalLift( Term term );

public:    
    Term lift( Term term );
    
public:   
	LiftStateClass() {}
};


void LiftStateClass::capturingFunctions( Ident & outer, std::list< int >  & index_of_functions ) {
	for ( int i = this->ancestors.size() - 1; i >= 0; i-- ) {
		FnTermClass * f = this->ancestors[ i ];
		index_of_functions.push_front( i );
		if ( f == outer->function() ) return;
	}		
	throw Unreachable( __FILE__, __LINE__ );
}


Ident LiftStateClass::lookup( const std::string & name ) {
	#ifdef DBG_LIFTING
		cerr << "lookup ... ";
	#endif
	list< Ident > & idlist = this->substitutes.back();
	for ( list< Ident >::iterator it = idlist.begin(); it != idlist.end(); ++it ) {
		#ifdef DBG_LIFTING
			cerr << " check... ";
		#endif
		if ( (*it)->getNameString() == name ) return *it;
	}
	#ifdef DBG_LIFTING
		cerr << endl;
	#endif
	return shared< IdentClass >();
}


Term LiftStateClass::generalLift( Term term ) {
    int i;
    int A = term_count( term );
    for ( i = 0; i < A; i++ ) {
        Term *r = term_index_ref( term, i );
        #ifdef DBG_LIFT
            fprintf( stderr, "Lifting arg with functor %s\n", functor_name( term_functor( *r ) ) );
        #endif
        *r = this->lift( *r );
    }
    return term;
}

Term LiftStateClass::lift( Term term ) {
    Functor fnc = term_functor( term );
    #ifdef DBG_LIFT
        fprintf( stderr, "Lifting term with functor %s\n", functor_name( fnc ) );
    #endif
    switch ( fnc ) {
        case fnc_fn : {
            FnTermClass * fn = dynamic_cast< FnTermClass * >( term.get() );
            this->ancestors.push_back( fn );
            this->substitutes.push_back( std::list< Ident >() );

			//	Now we lift the body.
			Term *body = term_index_ref( term, 1 );
			*body = this->lift( *body );

			this->ancestors.pop_back();            
            this->substitutes.pop_back();
            return term;
        }
        case fnc_id: {
         	IdTermClass * t = dynamic_cast< IdTermClass * >( term.get() );
         	if ( t->isOuterReference() ) {
         		Ident subst_id = this->lookup( t->nameString() );
         		if ( subst_id ) {
         			#ifdef DBG_LIFTING
						cerr << "Found previous use " << subst_id->getNameString();
						cerr << "@";
						cerr << subst_id->function() ->name();
						cerr << endl;
					#endif
         			t->ident() = subst_id; 
         		} else {
					std::list< int > index_of_functions;
					this->capturingFunctions( t->ident(), index_of_functions );
					
					Ident prev_id;
					for ( std::list< int >::iterator it = index_of_functions.begin(); it != index_of_functions.end(); ++it ) {
						int index = *it;
						if ( !prev_id ) {
							prev_id = t->ident();
						} else {
							FnTermClass * this_fn = this->ancestors[ index ];
	
							// Stuff is going to be put here.
							#ifdef DBG_LIFTING
								cerr << "Outer detected for " << this_fn->name() << endl;
								cerr << "  Index level " << index << endl;
								cerr << "  Outer ident is " << (prev_id)->getNameString() << "@" << (prev_id)->function()->name() << endl;							
							#endif
							Ident new_input = identNewLocal( t->nameString(), this_fn );
							if ( prev_id->isShared() ) { new_input->setShared(); }
							#ifdef DBG_LIFTING
								cerr << "  Inner ident is " << new_input->getFinalSlot() << endl;
							#endif
							
							//	Now set the new_input as the last parameter and
							//	add the outer to the function-term.
							this_fn->setAsFinalInput( new_input );		
							this_fn->addOuter( prev_id );
							this->substitutes[ index ].push_back( new_input );
							
							prev_id = new_input;
						}
					}
					
					//	At this point prev_id is bound to the correct IdTermClass
					t->ident() = prev_id;
				}
				return term;
            }
            return term;
        }
        default: {
            if ( term_count( term ) == 0 ) {
                return term;
            } else {
                return this->generalLift( term );
            }
        }
    }
    throw Unreachable( __FILE__, __LINE__ );
}

Term liftTerm( Package * pkg, Term term ) {
    LiftStateClass state;
    return state.lift( term );
}

