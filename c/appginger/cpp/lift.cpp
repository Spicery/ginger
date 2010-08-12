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

#include "lift.hpp"
#include "mishap.hpp"
#include "ident.hpp"
#include "package.hpp"
#include "term.hpp"

//#define DBG_LIFT
#ifdef DBG_LIFT
    #include <stdio.h>
#endif

//----------------------------------------------------------------------

enum Lookup {
    Global,
    InnerLocal,
    OuterLocal
};

//- environment --------------------------------------------------------

class Env {
public:
	std::vector< Ident > data;
	
public:
	void add( Ident id ) {
		this->data.push_back( id );
	}
	
	bool isNull() {
		return this->data.empty();
	}
	
	Ident & top() {
		return this->data.back();
	}
};


//-- lift state --------------------------------------------------------

class LiftStateClass {
public:
    Package *		package;
    int         	level;
    FnTermClass * 	function;
    Env        		env;
    
private:    
    enum Lookup lookup( NamedRefType r, const std::string & pkg, const std::string & c, Ident *id );
	Term general_lift( Term term );


public:    
    Term lift( Term term );
    
public:   
	LiftStateClass( Package * pkg, int level ) :
		package( pkg ),
		level( level ),
		function( NULL )
	{	
	}
};

enum Lookup LiftStateClass::lookup( NamedRefType r, const std::string & pkg, const std::string & c, Ident *id ) {
	for ( std::vector< Ident >::iterator it = this->env.data.begin(); it != this->env.data.end(); ++it ) {
		Ident ident = *it;
        const std::string & ic = ident->getNameString();
        #ifdef DBG_LIFT
            fprintf( stderr, "Comparing %s with %s\n", c.c_str(), ic.c_str() );
        #endif
        if ( c == ic ) {
            *id = ident;
            #ifdef DBG_LIFT
                fprintf( stderr, "< lookup\n" );
            #endif
            return ident->level >= this->level ? InnerLocal : OuterLocal;
        }		
	}
	Package * p;
	if ( r == LOCAL_REF_TYPE ) {
		p = this->package;
	} else if ( r == ABSOLUTE_REF_TYPE ) {
		p = this->package->getPackage( pkg );
	} else if ( r == QUALIFIED_REF_TYPE ) {
		throw ToBeDone();
	} else {
		throw Unreachable();
	}
    *id = p->lookup( c );
    if ( *id == 0 ) throw Mishap( "Undeclared variable" ).culprit( "Variable", c );
    #ifdef DBG_LIFT
        fprintf( stderr, "< lookup\n" );
    #endif
    return Global;
}

Term LiftStateClass::general_lift( Term term ) {
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
    	case fnc_package: {
    		Package * previous = this->package;
    		this->package = this->package->getPackage( term_package_url( term ) );
    		Term t = this->general_lift( term );
    		this->package = previous;
    		return t;
    	}
    	case fnc_import: {
    		ImportTermClass * t = dynamic_cast< ImportTermClass * >( term.get() );
			Package * from_pkg = this->package->getPackage( t->from );
			this->package->import( 
				Import( 
					t->facet,
					from_pkg,
					t->alias,
					t->prot,
					//t->into,
					t->intos
				)
			);
    		return term;
    	}
        case fnc_define: {
            Term synthetic =
                term_new_basic2(
                    fnc_var,
                    term_index( term, 0 ),
                    term_new_fn(
                        term_index( term, 1 ),
                        term_index( term, 2 )
                    )
                );
            #ifdef DBG_LIFT
                fprintf( stderr, "SYNTHETIC\n " ) ;
                term_print( synthetic );
            #endif
            return this->lift( synthetic );
        }
        case fnc_fn : {
            //    fn( args( id( _ ), ... ), Body )
            this->level += 1;
            FnTermClass * old_function = this->function;
            
            FnTermClass * fn = dynamic_cast< FnTermClass * >( term.get() );
            this->function = fn;
            int & slot = fn->nlocals();
            Term args = fn->child( 0 );
            int a = term_count( args );
            
            //int slot = 0;
            #ifdef DBG_LIFT
                fprintf( stderr, "Arity = %d\n", a );
            #endif

            for ( int i = 0; i < a; i++ ) {
            	NamedTermMixin * arg = dynamic_cast< NamedTermMixin * >( term_index( args, i ).get() );
                //Term arg = term_index( args, i );
                const std::string & c = arg->name(); //term_named_string( arg );
                #ifdef DBG_LIFT
                    fprintf( stderr, "Processing %s\n", c );
                #endif
                Ident id = ident_new_local( c );
                this->env.add( id );
                //printf( "Function now has %d slots\n", fn->nlocals() );
                id->slot = slot++;
                //printf( "Function now has %d slots\n", fn->nlocals() );
                id->level = this->level;
                arg->ident() = id;
            }
            #ifdef DBG_LIFT
                fprintf( stderr, "Processed arguments\n" );
                term_print( term );
            #endif
            {
                Term *r = term_index_ref( term, 1 );
                *r = this->lift( *r );
            }
            #ifdef DBG_LIFT
                fprintf( stderr, "Returning term\n" );
            #endif
    		fn->ninputs() = a;
            //*term_fn_nlocals_ref( term ) = ( this->env.isNull() ? 0 : this->env.top()->slot + 1 );
            //*term_fn_ninputs_ref( term ) = a;
            
            this->level -= 1;
            this->function = old_function;
            return term;
        }
        case fnc_var : {
        	VarTermClass * t = dynamic_cast< VarTermClass * >( term.get() );
            const std::string & c = term->name();	//	term_named_string( term );
            if ( this->level == 0 ) {
                //    printf( "GLOBAL VAR %s\n", c );
                //	Lookup or add is really only applicable to development mode.
                //	When we implement a distinction between runtime and devtime
                //	we'll have to plug this.
                //const Facet * facet = t->facet();
                const FacetSet * facets = t->facets();
                Ident id = this->package->lookup_or_add( c, /*facet,*/ facets );
                t->ident() = id;
            } else {
            	FnTermClass * fn = this->function;
            	int & slot = fn->nlocals();
                //    printf( "LOCAL VAR %s\n", c );
                Ident id = ident_new_local( c );
                this->env.add( id );
                id->slot = slot++;
                id->level = this->level;
                t->ident() = id;
                //printf( "Function now has %d slots\n", fn->nlocals() );
            }
 			return term;			        	
        }
         case fnc_id: {
         	IdTermClass * t = dynamic_cast< IdTermClass * >( term.get() );
            Ident id;
            switch ( this->lookup( t->refType(), t->pkg(), t->name(), &id ) ) {
                case Global:
                case InnerLocal: {
                    //    fprintf( stderr, "Found ident at %x\n", (unt)( id ) );
                    t->ident() = id;
                    //    fprintf( stderr, "Stuffed term with ident at %x\n", (unt)( term_id_ident( term ) ) );
                    return term;
                }
                case OuterLocal: {
                    to_be_done( "LIFT: outer locals" );
                }
            }
            throw;	//	Unreachable.
        }
       case fnc_add: {
            //Pool scratch = state.pools->scratch;
            Term arg0 = term_index( term, 0 );
            Term arg1 = term_index( term, 1 );
            Functor fnc0 = term_functor( arg0 );
            Functor fnc1 = term_functor( arg1 );
            if ( fnc0 == fnc_int ) {
                return this->lift( term_new_basic2( fnc_incr_by, arg1, arg0 ) );
            } else if ( fnc1 == fnc_int ) {
                return this->lift( term_new_basic2( fnc_incr_by, arg0, arg1 ) );
            }
            return this->general_lift( term );
        }
        case fnc_sub: {
            Term arg0 = term_index( term, 0 );
            Term arg1 = term_index( term, 1 );
            Functor fnc1 = term_functor( arg1 );
            if ( fnc1 == fnc_int ) {
                return this->lift( term_new_basic2( fnc_decr_by, arg0, arg1 ) );
            }
            return this->general_lift( term );
        }
        default: {
            if ( term_count( term ) == 0 ) {
                return term;
            } else {
                return this->general_lift( term );
            }
        }
    }
    throw;	//	Unreachable.
}

Term lift_term( Package * pkg, Term term ) {
    LiftStateClass state( pkg, 0 );
    return state.lift( term );
}
