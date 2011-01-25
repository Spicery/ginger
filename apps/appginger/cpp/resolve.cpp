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

//  Resolve is the scope analyser.  It combines two jobs, name resolution
//	and conversion of outer-locals by a process akin to lambda lifting.
//
//    Resolveing is a non-trivial task so I am deferring that in order to
//    make progress on more basic issues.

#include <iostream>
#include <vector>
#include <list>
using namespace std;

#include "resolve.hpp"
#include "mishap.hpp"
#include "ident.hpp"
#include "package.hpp"
#include "term.hpp"

//#define DBG_RESOLVE
#ifdef DBG_RESOLVE
    #include <stdio.h>
#endif

//----------------------------------------------------------------------

enum Lookup {
    Global,
    InnerLocal,
    OuterLocal
};

//- environment --------------------------------------------------------

struct Scope { 
	std::vector< Ident > 	data;
	FnTermClass *			fn;
	
	Scope( FnTermClass * f ) : fn( f ) {
	}
};

class Env {
private:
	std::vector< Scope > idents;

public:	
	void add( Ident id ) {
		this->idents.back().data.push_back( id );
	}
	
	FnTermClass * getFunction( const int n ) const {
		return this->idents[ n ].fn;
	}
	
	void addAt( const int n, const Ident id ) {
		this->idents[ n ].data.push_back( id );
	}
	
	void setAsFinalInput( int from_index, Ident new_input ) {
		FnTermClass * f = this->idents[ from_index ].fn;
		int & nin = f->ninputs();
	
		nin += 1;
		int slot = new_input->getFinalSlot();
		cerr << "Bumping #inputs for " << f->name() << " from " << nin-1 << " to " << nin << endl;
		
		//	Now we need to swap nin and slot. Check there is a need to make
		//	changes!
		if ( nin != slot ) {	
			int N = this->idents.size();
			for ( int i = from_index; i < N; i++ ) {
				Scope & le = this->idents[ i ];
				if ( le.fn != NULL && i > from_index ) break;
				
				std::vector< Ident > & v = le.data;
				for ( std::vector< Ident >::iterator it = v.begin(); it != v.end(); ++it ) {
					it->get()->swapSlot( nin, slot );
				}
			}
		}
	}
		
	void capturingFunctions( Ident & outer, std::list< int >  & index_of_functions ) {
		for ( int i = this->idents.size() - 1; i >= 0; i-- ) {
			Scope & le = this->idents[ i ];
			FnTermClass * f = le.fn;
			if ( f ) {
				index_of_functions.push_front( i );
			}
			if ( f == outer->function() ) return;
		}		
		throw Unreachable( __FILE__, __LINE__ );
	}
	
	Ident search( const std::string & c ) {
		for ( std::vector< Scope >::reverse_iterator it = this->idents.rbegin(); it != this->idents.rend(); ++it ) {
			Scope & le = *it;
			std::vector< Ident > & v = le.data;
			for ( std::vector< Ident >::reverse_iterator jt = v.rbegin(); jt != v.rend(); ++jt ) {
				Ident ident = *jt;
				const std::string & ic = ident->getNameString();
				#ifdef DBG_RESOLVE
					fprintf( stderr, "Comparing %s with %s\n", c.c_str(), ic.c_str() );
				#endif
				if ( c == ic ) {
					return ident;
				}		
			}
		}
		return shared< IdentClass >();
	}
	
	void mark( FnTermClass * fn ) {
		this->idents.push_back( Scope( fn ) );
	}
	
	void cutToMark() {
		this->idents.pop_back();
	}

};


//-- resolve state ------------------------------------------------------

class ResolveStateClass {
public:
    Package *		package;
    int         	level;
    FnTermClass * 	function;
    Env        		env;
    bool			needs_lifting;
    
private:    
    enum Lookup lookup( IdTermClass * t, Ident *id );
	Term generalResolve( Term term );


public:    
	void addIdent( NamedTermMixin * arg, FnTermClass * fn, int & slot );
    Term resolve( Term term );
    bool needsLifting() { return this->needs_lifting; }
    
public:   
	ResolveStateClass( Package * pkg, int level ) :
		package( pkg ),
		level( level ),
		function( NULL ),
		needs_lifting( false )
	{	
	}
};

static void lookupAndAddGlobal( Package * current, VarTermClass * t ) {
	const std::string & epkg = t->encPkg();
	const std::string & dpkg = t->defPkg();
	const std::string & c = t->name();
	//const FacetSet * facets = t->facets();
	switch ( t->refType() ) {
		case UNQUALIFIED_REF_TYPE: {
			Package * p = epkg != "" ? current->getPackage( epkg ) : current;
			t->ident() = p->fetchDefinitionIdent( c ); //, facets );
			break;
		}
		case ABSOLUTE_REF_TYPE: {
			Package * p = current->getPackage( dpkg );
			t->ident() = p->fetchDefinitionIdent( c ); //, facets );
			break;
		}
		case ALIAS_REF_TYPE: {
			throw Mishap( "Declaring global variable in aliased context" );
			/*Import * imp = current->getAlias( t->alias() );
			const FacetSet * m = imp->matchingTags();
		
			//cout << "ALIAS DECLARATION" << endl;
			//cout << "Import facet : " << *m << endl;
			//cout << "Declaration facets: " << *facets << endl;
		
			if ( imp != NULL && facets->isntEmptyIntersection( m ) ) {
				Ident id = imp->package()->fetchDefinitionIdent( c, facets );
				t->ident() = id;
			} else if ( imp != NULL ) {
				throw Mishap( "Declaration in package referenced by alias of an import would not be exported" );
			} else {
				throw Mishap( "No such alias" );
			}
			break;*/
		}
	}
}

static Ident lookupGlobal( Package * current, IdTermClass * t ) {
	const std::string & epkg = t->encPkg();
	const std::string & dpkg = t->defPkg();
	const std::string & c = t->name();
	//cout << "Looking up global identifier: " << c << endl;
	//cout << "Ref type                    : " << t->refType() << endl;
	//cout << "Def pkg                     : " << dpkg << endl;
	switch ( t->refType() ) {
		case UNQUALIFIED_REF_TYPE: {
			Package * p = epkg != "" ? current->getPackage( epkg ) : current;
			return p->fetchUnqualifiedIdent( c );
		}
		case ABSOLUTE_REF_TYPE: {
			Package * p = current->getPackage( dpkg );
			return p->fetchAbsoluteIdent( c );
		}
		case ALIAS_REF_TYPE: {
			Package * p = epkg != "" ? current->getPackage( epkg ) : current;
			return p->fetchQualifiedIdent( t->alias(), c );
		}
		default:
			throw Unreachable( __FILE__, __LINE__ );
	}
}


enum Lookup ResolveStateClass::lookup( IdTermClass * t, Ident *id ) {
	const std::string & c = t->name();
	Ident ident = this->env.search( c ); 
	if ( ident ) {
		*id = ident;
		#ifdef DBG_RESOLVE
			fprintf( stderr, "< lookup\n" );
		#endif
		Lookup result = ident->level >= this->level ? InnerLocal : OuterLocal;
		if ( result == OuterLocal ) {
			ident->setOuter();
		}
		return result;
	} else {
		*id = lookupGlobal( this->package, t );
		if ( *id == 0 ) throw Mishap( "Undeclared variable" ).culprit( "Variable", c );
		#ifdef DBG_RESOLVE
			fprintf( stderr, "< lookup\n" );
		#endif
		return Global;
	}
}

Term ResolveStateClass::generalResolve( Term term ) {
    int i;
    int A = term_count( term );
    for ( i = 0; i < A; i++ ) {
        Term *r = term_index_ref( term, i );
        #ifdef DBG_RESOLVE
            fprintf( stderr, "Resolveing arg with functor %s\n", functor_name( term_functor( *r ) ) );
        #endif
        *r = this->resolve( *r );
    }
    return term;
}

void ResolveStateClass::addIdent( NamedTermMixin * arg, FnTermClass * fn, int & slot ) {
	//cerr << "Argument " << arg->nameString() << " in " << fn->name() << endl;
	const std::string & c = arg->name();
	#ifdef DBG_RESOLVE
		fprintf( stderr, "Processing %s\n", c );
	#endif
	Ident id = identNewLocal( c, fn );
	this->env.add( id );
	id->level = this->level;
	arg->ident() = id;
}

Term ResolveStateClass::resolve( Term term ) {
    Functor fnc = term_functor( term );
    #ifdef DBG_RESOLVE
        fprintf( stderr, "Resolving term with functor %s\n", functor_name( fnc ) );
    #endif
    switch ( fnc ) {
    	case fnc_package: {
    		Package * previous = this->package;
    		this->package = this->package->getPackage( term_package_url( term ) );
    		Term t = this->generalResolve( term );
    		this->package = previous;
    		return t;
    	}
    	case fnc_import: {
    		/*ImportTermClass * t = dynamic_cast< ImportTermClass * >( term.get() );
			Package * from_pkg = this->package->getPackage( t->from );
			this->package->import( 
				Import( 
					t->matchTags(),
					from_pkg,
					t->alias,
					t->prot,
					t->intos
				)
			);
    		return term;*/
    		throw SystemError( "import not directly supported any more" );
    	}
        case fnc_fn : {
            //    fn( seq( id( _ ), ... ), Body )
            this->level += 1;
            FnTermClass * old_function = this->function;

            FnTermClass * fn = dynamic_cast< FnTermClass * >( term.get() );
         	this->env.mark( fn );
            
            this->function = fn;
            int & slot = fn->nlocals();
            Term args = fn->child( 0 );
            
            // There are two cases that we handle at present.
            //	A single variable var(x) or 
            //	a sequence of variables seq( var(x1), var(x2)... )
            
            switch ( args->functor() ) {
				case fnc_var: {
					fn->ninputs() = 1;
					NamedTermMixin * arg = dynamic_cast< NamedTermMixin * >( args.get() );
					this->addIdent( arg, fn, slot );
					break;
				}
				case fnc_seq: {
					int A = term_count( args );
					
					#ifdef DBG_RESOLVE
						fprintf( stderr, "Arity = %d\n", A );
					#endif
		
					fn->ninputs() = A;
					for ( int i = 0; i < A; i++ ) {
						NamedTermMixin * arg = dynamic_cast< NamedTermMixin * >( term_index( args, i ).get() );
						this->addIdent( arg, fn, slot );
					}
					break;
				}
				default: throw ToBeDone();          
            }
            
            #ifdef DBG_RESOLVE
                fprintf( stderr, "Processed arguments\n" );
                term_print( term );
            #endif
            {
            	//	Now we resolve the body.
                Term *r = term_index_ref( term, 1 );
                *r = this->resolve( *r );
            }
            #ifdef DBG_RESOLVE
                fprintf( stderr, "Returning term\n" );
            #endif
            
            this->level -= 1;
            this->function = old_function;
            this->env.cutToMark();
            return term;
        }
        case fnc_block: {
        	this->env.mark( NULL );
        	Term t = term_new_basic0( fnc_seq );
			int A = term_count( term );
			for ( int i = 0; i < A; i++ ) {
				term_add( t, this->resolve( *term_index_ref( term, i ) ) );
			}
			this->env.cutToMark();
			return t;        	
        }
        case fnc_var : {
        	VarTermClass * t = dynamic_cast< VarTermClass * >( term.get() );
            const std::string & c = term->name();
            if ( this->level == 0 ) {
                //    printf( "GLOBAL VAR %s\n", c );
                //	Lookup/add is really only applicable to development mode.
                //	When we implement a distinction between runtime and devtime
                //	we'll have to plug this.
                lookupAndAddGlobal( this->package, t );
            } else {
            	FnTermClass * fn = this->function;
                //    printf( "LOCAL VAR %s\n", c );
                Ident id = identNewLocal( c, fn );
                this->env.add( id );
                id->level = this->level;
                t->ident() = id;
            }
 			return term;			        	
        }
        case fnc_id: {
         	IdTermClass * t = dynamic_cast< IdTermClass * >( term.get() );
            Ident id;
            Lookup flag = this->lookup( t, &id );
            t->ident() = id;
            if ( flag == OuterLocal ) {
                t->setOuterReference();
                this->needs_lifting = true;
            }
            return term;
        }
        case fnc_add: {
            Term arg0 = term_index( term, 0 );
            Term arg1 = term_index( term, 1 );
            Functor fnc0 = term_functor( arg0 );
            Functor fnc1 = term_functor( arg1 );
            if ( fnc0 == fnc_int ) {
                return this->resolve( term_new_basic2( fnc_incr_by, arg1, arg0 ) );
            } else if ( fnc1 == fnc_int ) {
                return this->resolve( term_new_basic2( fnc_incr_by, arg0, arg1 ) );
            }
            return this->generalResolve( term );
        }
        case fnc_sub: {
            Term arg0 = term_index( term, 0 );
            Term arg1 = term_index( term, 1 );
            Functor fnc1 = term_functor( arg1 );
            if ( fnc1 == fnc_int ) {
                return this->resolve( term_new_basic2( fnc_decr_by, arg0, arg1 ) );
            }
            return this->generalResolve( term );
        }
        case fnc_assign: {
        	this->generalResolve( term );
        	NamedTermMixin * t = dynamic_cast< NamedTermMixin * >( term->child(1).get() );
        	t->ident()->setAssigned();
        }
        default: {
            if ( term_count( term ) == 0 ) {
                return term;
            } else {
                return this->generalResolve( term );
            }
        }
    }
    throw Unreachable( __FILE__, __LINE__ );
}

Term resolveTerm( Package * pkg, Term term, bool & needs_lifting ) {
    ResolveStateClass state( pkg, 0 );
    Term t = state.resolve( term );
    needs_lifting = state.needsLifting();
    return t;
}

