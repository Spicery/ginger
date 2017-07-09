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

#include <stdexcept>
#include <string>
#include <sstream>

#include <stddef.h>
//#include <stdio.h>
#include <assert.h>

#include "gnxconstants.hpp"
#include "shared.hpp"
#include "mishap.hpp"
#include "mnx.hpp"

#include "item_factory.hpp"
#include "read_expr.hpp"
#include "item.hpp"
#include "role.hpp"
#include "sysconst.hpp"
#include "gnxconstants.hpp"
#include "mishap.hpp"

//#define DBG_COMMON2GNX

using namespace std;

#define CONSTANT_WAS_ANON "was.anon"

typedef Ginger::MnxBuilder NodeFactory;

static Node makeApp( Node lhs, Node rhs ) {
    if ( lhs->name() == Ginger::GNX_SYSFN ) {
        NodeFactory sysapp;
        sysapp.start( Ginger::GNX_SYSAPP );
        std::string name = lhs->attribute( Ginger::GNX_SYSFN_VALUE );
        sysapp.put( Ginger::GNX_SYSAPP_NAME, name );
        sysapp.add( rhs );
        sysapp.end();
        return sysapp.build();
    } else {
        NodeFactory app;
        app.start( Ginger::GNX_APP );
        app.add( lhs );
        app.add( rhs );
        app.end();
        return app.build();
    }
}

static Node makeIndex( Node lhs, Node rhs ) {
    NodeFactory index;
    index.start( Ginger::GNX_SYSAPP );
    index.put( Ginger::GNX_SYSAPP_NAME, "index" );
    index.add( rhs );
    index.add( lhs );
    index.end();
    return index.build();
}

static void pushConstant( NodeFactory & f, const char * type, const std::string & value ) {
    f.start( Ginger::GNX_CONSTANT );
    f.put( Ginger::GNX_CONSTANT_TYPE, type );
    f.put( Ginger::GNX_CONSTANT_VALUE, value );
    f.end();
}

static void pushConstant( NodeFactory & f, const char * type, const long value ) {
    f.start( Ginger::GNX_CONSTANT );
    f.put( Ginger::GNX_CONSTANT_TYPE, type );
    f.put( Ginger::GNX_CONSTANT_VALUE, value );
    f.end();
}

static Node makeConstant( const char * type, const std::string & value ) {
    NodeFactory simple;
    pushConstant( simple, type, value );
    return simple.build();
}

#ifdef NOT_CURRENTLY_USED_BUT_KEEP
static Node makeConstant( const char * type, const long value ) {
    NodeFactory simple;
    pushConstant( simple, type, value );
    return simple.build();
}
static void pushStringConstant( NodeFactory & f, const string & s ) {
    pushConstant( f, "string", s.c_str() );
}
static void pushAnyPattern( NodeFactory & f ) {
    f.start( VAR );
    f.put( Ginger::GNX_VID_PROTECTED, "true" );
    f.end();
}
#endif


static Node makeEmpty() {
    return make_shared< Ginger::Mnx >( Ginger::GNX_SEQ );
}

static void updateAsPattern( Node node, const bool val_vs_var ) {
    if ( node->hasName( Ginger::GNX_ID ) ) {
        node->name() = Ginger::GNX_VAR;
        node->putAttribute( Ginger::GNX_VID_PROTECTED, val_vs_var ? "true" : "false" );
    } else if ( node->hasName( Ginger::GNX_CONSTANT ) && node->hasAttribute( CONSTANT_WAS_ANON ) ) {
        node->name() = Ginger::GNX_VAR;
        node->removeAttribute( Ginger::GNX_CONSTANT_TYPE );
        node->removeAttribute( Ginger::GNX_CONSTANT_VALUE );
    } else if ( node->hasName( Ginger::GNX_SEQ ) ) {
        for ( Ginger::MnxChildIterator chit( node ); !!chit; ++chit ) {
            Node child = *chit;
            updateAsPattern( child, val_vs_var );
        }
    }
}

string ReadStateClass::readPkgName() {
    Item it = this->item_factory->read();
    return it->nameString();
}

const string ReadStateClass::readIdName() {
    Item it = this->item_factory->read();
    if ( it->tok_type != tokty_id ) {
        Ginger::Mishap error = CompileTimeError( "Identifier expected" ).culprit( "Found", it->nameString() );
        if ( it->tok_type == tokty_eof ) {
            UnexpectedEndOfInputError().cause( error );
        } else {
            throw error;
        }
    }
    return it->nameString();
}

Node ReadStateClass::readExprPrec( int prec ) {
    Node e = this->readOptExprPrec( prec );
    if ( not e ) {
        Item it = this->item_factory->peek();
        if ( it->item_is_eof() ) {
            throw UnexpectedEndOfInputError();
        } else if ( it->item_is_anyfix() ) {
            throw CompileTimeError( "Found reserved word" ).culprit( it->nameString() );
        } else {
            throw CompileTimeError( "Unexpected end of expression" ).culprit( it->nameString() );
        }
    }
    return e;
}

void ReadStateClass::checkToken( TokType fnc ) {
    Item it = this->item_factory->read();
    if ( it->tok_type != fnc ) {
        Ginger::Mishap err = CompileTimeError( "Unexpected token" ).culprit( "Found", it->nameString() ).culprit( "Expected", tok_type_name( fnc ) );
        if ( it->item_is_eof() ) {
            throw UnexpectedEndOfInputError().cause( err );
        }
        throw err;
    }
}

bool ReadStateClass::tryPeekToken( TokType fnc ) {
    Item it = this->item_factory->peek();
    return it->tok_type == fnc;
}

bool ReadStateClass::tryPeekCloser() {
    Item it = this->item_factory->peek();
    return it->role.IsCloser();
}

void ReadStateClass::checkPeekCloser() {
    Item it = this->item_factory->peek();
    if ( not it->role.IsCloser() ) {
        if ( it->item_is_eof() ) {
            throw UnexpectedEndOfInputError().hint( "Looking for closing keyword" );
        }
        throw CompileTimeError( "Unexpected token" ).culprit( "Found", it->nameString() );
    }
}

void ReadStateClass::checkPeekToken( TokType fnc ) {
    Item it = this->item_factory->peek();
    if ( it->tok_type != fnc ) {
        if ( it->item_is_eof() ) {
            throw UnexpectedEndOfInputError().hint( "Looking for closing keyword" ).culprit( "Expected", tok_type_name( fnc ) );
        }       
        throw CompileTimeError( "Unexpected token" ).culprit( "Found", it->nameString() ).culprit( "Expected", tok_type_name( fnc ) );
    }
}

bool ReadStateClass::tryName( const char * name ) {
    ItemFactory ifact = this->item_factory;
    Item it = ifact->peek();
    if ( it->nameString() == name ) {
        ifact->drop();
        return true;
    } else {
        return false;
    }
}

bool ReadStateClass::tryToken( TokType fnc ) {
    ItemFactory ifact = this->item_factory;
    if ( ifact->peek()->tok_type == fnc ) {
        ifact->drop();
        return true;
    } else {
        return false;
    }
}

Node ReadStateClass::readCompoundCore() {
    if ( this->cstyle_mode ) {
        NodeFactory stmnts;
        stmnts.start( Ginger::GNX_SEQ );
        while ( this->tryPeekToken( tokty_semi ) || not this->tryPeekCloser() ) {
            Node n = this->readSingleStmnt();
            stmnts.add( n );
        }
        stmnts.end();
        return stmnts.build();
    } else {
        return this->readStmnts();
    }
}

Node ReadStateClass::readCompoundCoreCheck( TokType closer ) {
    Node n = this->readCompoundCore();
    this->checkToken( closer );
    return n;
}

Node ReadStateClass::readCompoundStmnts() {
    if ( this->cstyle_mode ) {
        this->checkToken( tokty_obrace );
        return this->readCompoundCoreCheck( tokty_cbrace );
    } else {
        return this->readStmnts();
    }
}

Node ReadStateClass::readSingleStmnt( const bool top_level ) {
    if ( this->cstyle_mode ) {
        Item it = this->item_factory->read();
        //cerr << "SINGLE " << tok_type_name( it->tok_type ) << endl;
        switch ( it->tok_type ) {
            case tokty_obrace: 
                return this->readCompoundCoreCheck( tokty_cbrace );
            case tokty_function: 
                if ( this->cstyle_mode && this->item_factory->peek()->tok_type == tokty_oparen ) {
                    return this->readLambda();
                } else {
                    return this->readDefinition();
                }
            case tokty_if: 
                return this->readIf( tokty_if, tokty_endif );
            case tokty_for: 
                return this->readFor();
            case tokty_switch: 
                return this->readSwitch();
            case tokty_recordclass:
                return this->readRecordClass();
            case tokty_dsemi:
            case tokty_semi: 
                return makeEmpty();
            default: 
                this->item_factory->unread();
                // --- Fall-through --- //
        }
    }
    
    //  Fall thru!
    Node n = this->readOptEmptyExpr();
    if ( this->tryToken( tokty_dsemi ) ) {
        NodeFactory f;
        f.start( Ginger::GNX_ERASE );
        f.add( n );
        f.end();
        return f.build();
    } else if ( this->tryToken( tokty_semi ) ) {
        return n;
    } else {
        if ( this->item_factory->peek()->role.IsCloser() && not top_level ) {
            return n;   
        } else {
            if ( this->item_factory->peek()->item_is_eof() ) {
                throw UnexpectedEndOfInputError().hint( "Missing semi-colon?" );
            }
            throw CompileTimeError( "Missing semi-colon?" ).culprit( "Token", this->item_factory->peek()->nameString() );
        }
    }
}

Node ReadStateClass::readStmnts() {
    if ( this->cstyle_mode ) {
        return this->readSingleStmnt();
    } else {
        return this->readOptEmptyExprPrec( prec_max );
    }
}


Node ReadStateClass::readStmntsCheck( TokType fnc ) {
    Node t = this->readStmnts();
    this->checkToken( fnc );
    return t;
}
    
Node ReadStateClass::readExpr() {
    return this->readExprPrec( prec_semi );
}

Node ReadStateClass::readQuery() {
    return this->readQueryPrec( prec_semi );
}


Node ReadStateClass::readQueryPrec( const int prec ) {
    Node e = this->readExprPrec( prec );
    const std::string name = e->name();
    if ( 
        name == Ginger::GNX_BIND    ||
        name == Ginger::GNX_IN      ||
        name == Ginger::GNX_FROM    ||
        name == Ginger::GNX_WHERE   ||
        name == Ginger::GNX_WHILE   ||
        name == Ginger::GNX_ZIP     ||
        name == Ginger::GNX_CROSS   ||
        name == Ginger::GNX_OK      ||
        name == Ginger::GNX_FINALLY ||
        name == Ginger::GNX_DO
    ) {
        return e;
    } else {
        NodeFactory where;
        where.start( Ginger::GNX_WHERE );
        where.start( Ginger::GNX_OK );
        where.end();
        where.add( e );
        where.end();
        return where.build();
    }
}

Node ReadStateClass::readOptExpr() {
    return this->readOptExprPrec( prec_semi );
}

Node ReadStateClass::readOptEmptyExprPrec( int prec ) {
    Node n = this->readOptExprPrec( prec);
    if ( not n ) {
        return makeEmpty();
    } else {
        return n;
    }
}

Node ReadStateClass::readOptEmptyExpr() {
    return this->readOptEmptyExprPrec( prec_semi );
}

Node ReadStateClass::readOptEmptyExprCheck( TokType fnc ) {
    Node n = this->readOptEmptyExpr();
    this->checkToken( fnc );
    return n;
}

Node ReadStateClass::readExprCheck( TokType fnc ) {
    Node t = this->readExpr();
    this->checkToken( fnc );
    return t;
}

Node ReadStateClass::postfixProcessing( Node lhs, Item item, int prec ) {
    Role role = item->role;
    TokType fnc = item->tok_type;
    if ( role.IsBinary() ) {
        const bool direction = tok_type_as_direction( fnc );
        if ( role.IsSys() ) {
            NodeFactory a;
            a.start( Ginger::GNX_SYSAPP );
            a.put( Ginger::GNX_SYSAPP_NAME, tok_type_as_sysapp( fnc ) );
            Node rhs = this->readExprPrec( prec );
            a.add( direction ? lhs : rhs );
            a.add( !direction ? lhs : rhs );
            a.end();
            return a.build();
        } else if ( role.IsForm() ) {
            NodeFactory a;
            a.start( tok_type_as_tag( fnc ) );
            Node rhs = this->readExprPrec( prec );
            a.add( direction ? lhs : rhs );
            a.add( !direction ? lhs : rhs );
            a.end();
            return a.build();           
        } else {
            throw CompileTimeError( "Internal error - postfixProcessing" );
        }
    } else if ( role.IsUnary() ) {
        if ( role.IsSys() ) {
            NodeFactory a;
            a.start( Ginger::GNX_SYSAPP );
            a.put( Ginger::GNX_SYSAPP_NAME, tok_type_as_sysapp( fnc ) );
            a.add( lhs );
            a.end();
            return a.build();
        } else if ( role.IsForm() ) {
            NodeFactory a;
            a.start( tok_type_as_tag( fnc ) );
            Node rhs = this->readExprPrec( prec );
            a.add( lhs );
            a.end();
            return a.build();           
        } else {
            throw CompileTimeError( "Internal error (postfixProcessing): None of these cases defined yet" );            
        }
    } else {
        switch ( fnc ) {
            case tokty_bindrev:
            case tokty_bind: {
                Node rhs = this->readExprPrec( prec );
                updateAsPattern( fnc == tokty_bind ? lhs : rhs, true );
                NodeFactory bind;
                bind.start( Ginger::GNX_BIND );
                bind.add( fnc == tokty_bind ? lhs : rhs );
                bind.add( fnc != tokty_bind ? lhs : rhs );
                bind.end();
                return bind.build();
            }
            case tokty_cross:
            case tokty_zip: {
                NodeFactory factory;
                factory.start( fnc == tokty_cross ? Ginger::GNX_CROSS : Ginger::GNX_ZIP );
                factory.add( lhs );
                Node rhs = this->readQueryPrec( prec );
                factory.add( rhs );
                factory.end();
                return factory.build();             
            }
            case tokty_finally: {
                NodeFactory finally;
                finally.start( Ginger::GNX_FINALLY );
                finally.add( lhs );
                Node rhs = this->readExprPrec( prec );
                finally.add( rhs );
                finally.end();
                return finally.build();
            }
            case tokty_until:
            case tokty_while: {
                NodeFactory whileuntil;
                whileuntil.start( Ginger::GNX_WHILE );
                whileuntil.add( lhs );
                Node rhs = this->readExprPrec( prec );
                if ( fnc == tokty_until ) {
                    whileuntil.start( Ginger::GNX_SYSAPP );
                    whileuntil.put( Ginger::GNX_SYSAPP_NAME, "not" );
                }
                whileuntil.add( rhs );
                if ( fnc == tokty_until ) {
                    whileuntil.end();
                }
                if ( this->tryToken( tokty_then ) ) {
                    Node then = this->readExprPrec( prec_then );
                    whileuntil.add( then );
                } else {
                    whileuntil.start( Ginger::GNX_SEQ );
                    whileuntil.end();
                }
                whileuntil.end();
                return whileuntil.build();              
            }
            case tokty_where: {
                NodeFactory where;
                where.start( Ginger::GNX_WHERE );
                where.add( lhs );
                Node rhs = this->readExprPrec( prec );
                where.add( rhs );
                where.end();
                return where.build();
            }
            case tokty_do: {
                NodeFactory node;
                node.start( Ginger::GNX_DO );
                node.add( lhs );
                Node do_stmnts = this->readCompoundStmnts();
                if ( not this->tryToken( tokty_enddo ) ) {
                    this->checkPeekCloser();
                }
                node.add( do_stmnts );
                node.end();
                return node.build();
            }
            case tokty_from: {              
                updateAsPattern( lhs, true );
                NodeFactory node;
                node.start( Ginger::GNX_FROM );
                Node from_expr = this->readExprPrec( prec );
                node.add( lhs );
                node.add( from_expr );

                const bool has_by_part = this->tryToken( tokty_by );
                
                if ( has_by_part ) {
                    Node by_expr = this->readExprPrec( prec );
                    node.add( by_expr );
                }

                const bool has_to_part = this->tryToken( tokty_to );

                if ( has_to_part && not has_by_part ) {
                    //  We should insert a default.
                    //  TODO: refactor (extract) adding a constant int.
                    pushConstant( node, "int", "1" );
                }
                
                if ( has_to_part ) {
                    Node to_expr = this->readExprPrec( prec );
                    node.add( to_expr );
                }
                
                node.end();
                return node.build();
            }
            case tokty_in: {                
                updateAsPattern( lhs, true );
                Node in_expr = this->readExprPrec( prec );
                NodeFactory node;
                node.start( Ginger::GNX_IN );
                node.add( lhs );
                node.add( in_expr );
                node.end();
                return node.build();
            }
            case tokty_dsemi:
            case tokty_semi: {
                Node rhs = this->readOptExprPrec( prec );
                bool hasnt_rhs = not( rhs );
                if ( fnc == tokty_semi && hasnt_rhs ) {
                    return lhs;
                } else {
                    NodeFactory s;
                    s.start( fnc == tokty_semi ? Ginger::GNX_SEQ : Ginger::GNX_ERASE );
                    s.add( lhs );
                    if ( not hasnt_rhs ) s.add( rhs );
                    s.end();
                    return s.build();
                }
            }
            case tokty_obracket: {
                //  Indexing operator.
                Node rhs = this->readStmntsCheck( tokty_cbracket );
                return makeIndex( lhs, rhs );
            }
            case tokty_oparen: {    
                Node rhs = this->readOptEmptyExprCheck( tokty_cparen );
                return makeApp( lhs, rhs );
            }
            case tokty_at:
            case tokty_dot: {
                Node func = this->readExprPrec( prec_tight );
                Node rhs = this->readOptExprPrec( prec );           
                NodeFactory seq;
                seq.start( Ginger::GNX_SEQ );
                seq.add( lhs );
                if ( not not rhs ) { seq.add( rhs ); }
                seq.end();
                return makeApp( func, seq.build() );
            }
            case tokty_double: {
                NodeFactory add;
                //  TODO: Bug??
                add.start( "add" );
                add.add( lhs );
                pushConstant( add, "double", item->nameString() );
                add.end();
                Node t = add.build();
                //cerr << "DEBUG arity " << term_arity( t ) << endl;
                return t;
            }
            case tokty_int: {
                NodeFactory add;
                //  TODO: Bug??
                add.start( "add" );
                add.add( lhs );
                pushConstant( add, "int", item->nameString() );
                add.end();
                Node t = add.build();
                //cerr << "DEBUG arity " << term_arity( t ) << endl;
                return t;
            }
            /*
                case tokty_explode: {
                NodeFactory expl;
                expl.start( SYSAPP );
                expl.put( SYSAPP_NAME, "explode" );
                expl.add( lhs );
                expl.end();
                Node t = expl.build();
                return t;
            }
            */
            default: {
                throw SystemError( "This keyword not handled" ).culprit( "Keyword", item->nameString() );
            }
        }
    }
    throw UnreachableError();
}
    

static void predicate( TokType sense, NodeFactory & ifunless, Node pred ) {
    if ( sense == tokty_if ) {
        ifunless.add( pred );
    } else {
        //  TODO: This is a bug?
        ifunless.start( "sysval" );
        ifunless.put( "name", "not" );
        ifunless.add( pred );
        ifunless.end();
    }
}

Node ReadStateClass::readIf( TokType sense, TokType closer ) {
    if ( this->cstyle_mode ) this->checkToken( tokty_oparen );
    Node pred = this->readExpr();
    this->checkToken( this->cstyle_mode ? tokty_cparen : tokty_then );
    Node then_part = this->readStmnts();
    if ( this->tryToken( tokty_else ) ) {   
        NodeFactory ifunless;
        ifunless.start( Ginger::GNX_IF );
        predicate( sense, ifunless, pred );
        ifunless.add( then_part );
        Node x = this->readStmnts();
        if ( not this->cstyle_mode ) this->checkToken( closer );
        ifunless.add( x );
        ifunless.end();
        return ifunless.build();
    } else if ( this->cstyle_mode || this->tryToken( closer ) ) {
        NodeFactory ifunless;
        ifunless.start( Ginger::GNX_IF );
        predicate( sense, ifunless, pred );
        ifunless.add( then_part );
        ifunless.end();
        return ifunless.build();
    } else {
        TokType new_sense;
        if ( this->tryToken( tokty_elseif ) ) {
            new_sense = tokty_if;
        } else {
            this->checkToken( tokty_elseunless );
            new_sense = tokty_unless;
        }
        NodeFactory ifunless;
        ifunless.start( Ginger::GNX_IF );
        predicate( sense, ifunless, pred );
        ifunless.add( then_part );
        Node x = this->readIf( new_sense, closer );
        ifunless.add( x );
        ifunless.end();
        return ifunless.build();
    }
}

Node ReadStateClass::readSyscall() {
    ItemFactory ifact = this->item_factory; 
    Item it = ifact->read();
    if ( it->tok_type == tokty_id ) {
        return makeConstant( Ginger::GNX_SYSFN, it->nameString() );
    } else {
        if ( it->item_is_eof() ) {
            throw UnexpectedEndOfInputError().hint( "After >-> (syscall) arrow" );
        }
        throw CompileTimeError( "Invalid token after >-> (syscall) arrow" ).culprit( it->nameString() );
    }
}

Node ReadStateClass::readFor() {
    if ( this->cstyle_mode ) {
        this->checkToken( tokty_oparen );
        Node query = this->readQuery();
        this->checkToken( tokty_cparen );
        Node body = this->readStmnts();
        NodeFactory for_node;
        for_node.start( Ginger::GNX_FOR );
        for_node.start( Ginger::GNX_DO );
        for_node.add( query );
        for_node.add( body );
        for_node.end(); // DO
        for_node.end();
        return for_node.build();
    } else {
        Node query = this->readQuery();
        NodeFactory for_node;
        for_node.start( Ginger::GNX_FOR );
        for_node.add( query );
        this->checkToken( tokty_endfor );
        for_node.end();
        return for_node.build();
    }
}

static void readImportQualifiers( ReadStateClass & r, bool & pervasive, bool & qualified ) {
    pervasive = true;
    qualified = false;
    for (;;) {
        if ( r.tryName( "pervasive" ) ) {
            pervasive = true;
        } else if ( r.tryName( "nonpervasive" ) ) {
            pervasive = false;
        } else if ( r.tryName( "qualified" ) ) {
            qualified = true;
        } else if ( r.tryName( "unqualified" ) ) {
            qualified = false;
        } else {
            break;
        }
    }
}


static void readTags( ReadStateClass & r, NodeFactory & imp, const char * prefix, const bool add_default ) {
    if ( r.tryToken( tokty_oparen ) ) {
        ItemFactory ifact = r.item_factory;
        if ( ifact->peek()->tok_type != tokty_cparen ) {
            for ( int i = 0; true; i++ ) {
                Item item = ifact->read();
                ostringstream s;
                s << prefix << i;
                imp.put( s.str(), item->nameString() );
                
                item = ifact->read();
                if ( item->tok_type != tokty_comma ) {
                    if ( item->tok_type != tokty_cparen ) {
                        if ( item->item_is_eof() ) {
                            throw UnexpectedEndOfInputError().hint( "Looking for close parenthesis i.e. ')'" );
                        }
                        throw CompileTimeError( "Expecting close parenthesis" );
                    }
                    break;
                }
            }   
        }
    } else if ( add_default ) {
        imp.put( prefix, "public" );
    }
}

static void readImportMatch( ReadStateClass & r, NodeFactory & imp ) {
    readTags( r, imp, "match", true );
}

static void readImportInto( ReadStateClass & r, NodeFactory & imp ) {
    readTags( r, imp, "into", false );
}

static Node makeCharSequence( Item item ) {
    int n = item->nameString().size();
    if ( n == 0 ) {
        return makeEmpty();
    } else {
        NodeFactory charseq;
        charseq.start( Ginger::GNX_SEQ );
        const std::string & s = item->nameString();
        std::string::const_iterator iter = s.begin();
        for ( iter = s.begin(); iter != s.end(); ++iter ) {
            charseq.start( Ginger::GNX_CONSTANT );
            charseq.put( Ginger::GNX_CONSTANT_TYPE, "char" );
            charseq.put( Ginger::GNX_CONSTANT_VALUE, std::string() + *iter );   //   WRONG
            charseq.end();
        }
        charseq.end();
        return charseq.build();
    }
}

static Node makeSymbol( const std::string & name ) {
    NodeFactory sym;
    sym.start( "constant" );
    sym.put( "type", "symbol" );
    sym.put( "value", name );
    sym.end();
    return sym.build();
}

static Node makeAssert1( Node node ) {
    if ( node->hasName( Ginger::GNX_CONSTANT ) ) {
        return node;
    } else {
        NodeFactory assert1;
        assert1.start( "assert" );
        assert1.put( "n", 1 );
        assert1.add( node );
        assert1.end();
        return assert1.build();
    }
}

static Node maybeMakeAssert1( const bool only1, Node node ) {
    if ( only1 ) {
        return makeAssert1( node );
    } else {
        return node;
    }
}

/// @param only1 True if one and only one result permitted.
Node ReadStateClass::readAtomicExpr( const bool only1 ) {
    ItemFactory ifact = this->item_factory;
    Item item = ifact->read();
    TokType fnc = item->tok_type;
    Role role = item->role;
    if ( role.IsLiteral() ) {
        return makeConstant( tok_type_as_type( fnc ), item->nameString() ); 
    } else if ( item->item_is_name() ) {
        return makeSymbol( item->nameString() );
    } else if ( fnc == tokty_charseq ) {
        //  TODO: Could give a better error message here.
        return maybeMakeAssert1( only1, makeCharSequence( item ) );
    } else if ( fnc == tokty_oparen ) {
        return maybeMakeAssert1( only1, this->readExprCheck( tokty_cparen ) );
    } else {
        if ( item->item_is_eof() ) {
            throw UnexpectedEndOfInputError();
        }
        throw CompileTimeError( "Unexpected token while reading attribute in element" ).culprit( "Token", item->nameString() );
    }
}

class Canonise {
private:
    const bool name_needed;
    const bool assume_no_name;
    Node raw;
public:
    Canonise( bool _name_needed, bool _nameless, Node ap ) : 
        name_needed( _name_needed ),
        assume_no_name( not _name_needed and _nameless ),
        raw( ap ) 
    {}

public:
    /// build is guaranteed to return a (left) spine of APP nodes
    /// that terminates in a VAR nodes. The terminating VAR node may
    /// be anonymous.
    Node build() {
        #ifdef DBG_COMMON2GNX
            cerr << "Canonise" << endl;
            cerr << "  Before: ";
            this->raw->render( cerr );
            cerr << endl;
        #endif

        Node answer = this->canonise( 0, this->raw );

        #ifdef DBG_COMMON2GNX
            cerr << "  After: ";
            answer->render( cerr );
            cerr << endl;
        #endif

        return answer;
    }

private:
    void squash( NodeFactory acc, Node rhs ) {
        const std::string 
        name = rhs->name();
        if ( name == Ginger::GNX_SEQ ) {
            int n = rhs->size();
            for ( int i = 0; i < n; i++ ) {
                squash( acc, rhs->getChild( i ) );
            }
        } else {
            acc.add( rhs );
        }
    }

    Node anon( Node fn ) {
        NodeFactory b;
        b.start( Ginger::GNX_APP );
        b.start( Ginger::GNX_VAR ); // But no name - anonymous.
        b.end();
        b.start( Ginger::GNX_SEQ );
        squash( b, fn );
        b.end();
        b.end();
        return b.build();   
    }

    Node canonise( const int level, Node fn ) {
        const bool is_var = fn->hasName( Ginger::GNX_VAR );
        if ( is_var ) {
            if ( level == 0 ) {
                if ( this->name_needed ) this->badHeader();
                return this->anon( fn );
            } else if ( this->assume_no_name ) {
                return this->anon( fn );
            } else {
                return fn;
            }
        } else if ( fn->hasName( Ginger::GNX_SEQ ) && not this->name_needed ) {
            return this->anon( fn );
        } else if ( fn->hasName( Ginger::GNX_APP ) && fn->size() == 2 ) {
            NodeFactory b;
            b.start( Ginger::GNX_APP );
            Node n = this->canonise( level + 1, fn->getChild( 0 ) );
            b.add( n );
            b.start( Ginger::GNX_SEQ );
            squash( b, fn->getChild( 1 ) );
            b.end();
            b.end();
            return b.build();                   
        } else {
            throw this->badHeader();
        }
    }

    Ginger::Mishap badHeader() {
        Ginger::Mishap mishap( "Invalid function header", Ginger::Mishap::CompileTimeCategory );
        mishap.culprit( "Name", this->raw->name() );
        if ( this->raw->name() == Ginger::GNX_SYSAPP ) {
            mishap.culprit( "Reason", "Trying to redefine a system function" );
        }
        return mishap;
    }
};

class Uncurry {
private:
    Node lhs;
    Node rhs;
public:
    Uncurry( Node _lhs, Node _rhs ) : lhs( _lhs ), rhs( _rhs ) {}
public:
    void uncurry() {
        #ifdef DBG_COMMON2GNX
            cerr << "At start of uncurrying" << endl;
            cerr << "  LHS: ";
            this->lhs->render( cerr );
            cerr << endl;
            cerr << "  RHS: ";
            this->rhs->render( cerr );
            cerr << endl;
        #endif

        while ( this->isCurryd() ) {
            NodeFactory b;
            b.start( Ginger::GNX_FN );
            Node arg = this->lhs->getChild( 1 );
            if ( arg->hasName( Ginger::GNX_SEQ ) ) {
                b.add( arg );
            } else {
                b.start( Ginger::GNX_SEQ );
                b.add( arg );
                b.end();
            }
            b.add( this->rhs );
            b.end();
            this->rhs = b.build();
            this->lhs = this->lhs->getChild( 0 );
            #ifdef DBG_COMMON2GNX
                cerr << "Uncurried" << endl;
                cerr << "  LHS: ";
                this->lhs->render( cerr );
                cerr << endl;
                cerr << "  RHS: ";
                this->rhs->render( cerr );
                cerr << endl;
            #endif
        }
    }

    Node getFun() const {
        return this->lhs->getChild( 0 );
    }

    Node getArgs() const {
        Node args = this->lhs->getChild( 1 );
        if ( args->hasName( Ginger::GNX_SEQ ) ) return args;
        NodeFactory b;
        b.start( Ginger::GNX_SEQ );
        b.add( args );
        b.end();
        return b.build();
    }

    Node getBody() const {
        return this->rhs;
    }
private:
    bool isCurryd() const {
        return isApp( this->lhs ) && isApp( this->lhs->getChild( 0 ) );
    }
    static bool isApp( const Node & n ) {
        return n->hasName( Ginger::GNX_APP ) && n->size() == 2;
    }
};


Node ReadStateClass::readCanonicalLambdaLHS( const bool name_needed ) {

    const bool starts_with_oparen = this->item_factory->peek()->tok_type == tokty_oparen;

    ReadStateClass pattern( *this );
    pattern.setPatternMode();
    #ifdef DBG_COMMON2GNX
        cerr << "About to read pattern" << endl;
    #endif
    Node ap = pattern.readExprPrec( prec_arrow );
    #ifdef DBG_COMMON2GNX
        cerr << "Pattern read" << endl;
    #endif
    Canonise canonise( name_needed, starts_with_oparen, ap );
    #ifdef DBG_COMMON2GNX
        cerr << "About to canonicalise" << endl;
    #endif

    return canonise.build();
}

Node ReadStateClass::readLambda() {
    #ifdef DBG_COMMON2GNX
        cerr << "LAMBDA" << endl;
    #endif

    Node ap = this->readCanonicalLambdaLHS( false );
    #ifdef DBG_COMMON2GNX
        cerr << "Now check =>> is next token" << endl;
    #endif
    if ( not this->cstyle_mode ) this->checkToken( tokty_fnarrow );
    Node body = this->readCompoundStmnts();
    if ( not this->cstyle_mode ) this->checkToken( tokty_endfn );   

    Uncurry components( ap, body );
    components.uncurry();

    Node fun = components.getFun();
    
    NodeFactory fn;
    fn.start( Ginger::GNX_FN );
    if ( fun && fun->hasAttribute( "name" ) ) {
        fn.put( Ginger::GNX_FN_NAME, fun->attribute( "name" ) );
    }
    {
        Node a = components.getArgs();
        fn.add( a );
        Node b = components.getBody();
        fn.add( b );
    }
    fn.end();
    Node lambda = fn.build();           
    #ifdef DBG_COMMON2GNX
        cerr << "Built lambda: ";
        lambda->render( cerr );
        cerr << endl;
    #endif
    return lambda;
}

Node ReadStateClass::readDefinition() {
    Node ap = this->readCanonicalLambdaLHS( true );

    if ( not this->cstyle_mode ) this->checkToken( tokty_fnarrow );
    Node body0 = this->readCompoundStmnts();
    if ( not this->cstyle_mode ) this->checkToken( tokty_enddefine );
    if ( this->cstyle_mode ) {
        this->is_postfix_allowed = false;
    }

    Uncurry components( ap, body0 );
    components.uncurry();

    Node fn = components.getFun();
    if ( not fn->hasAttribute( "name" ) ) {
        throw CompileTimeError( "Cannot determine function name" );
    }
    Node args = components.getArgs();
    Node body = components.getBody();

    const std::string name( fn->attribute( Ginger::GNX_VID_NAME ) );
    NodeFactory def;
    def.start( Ginger::GNX_BIND );
    def.start( Ginger::GNX_VAR );
    def.put( Ginger::GNX_VID_NAME, name );
    def.put( Ginger::GNX_VID_PROTECTED, "true" );
    def.end();
    def.start( Ginger::GNX_FN );
    def.put( Ginger::GNX_FN_NAME, name );
    def.add( args );
    def.add( body );
    def.end();
    def.end();
    return def.build();
}

Node ReadStateClass::readTry( const bool try_vs_transaction ) {
    NodeFactory ftry;
    ftry.start( try_vs_transaction ? "try" : "transaction" );
    Node app = this->readExpr();
    ftry.add( app );

    while ( this->tryToken( tokty_catch ) ) {

        if ( this->tryPeekToken( tokty_oparen ) ) {
            ftry.start( "catch.return" );
        } else {
            ftry.start( "catch.then" );
            ftry.put( "event", this->readIdName() );            
        }       
        
        ReadStateClass pattern( *this );
        pattern.setPatternMode();
        Node catch_patt = pattern.readExpr();
        ftry.add( catch_patt );
        
        this->checkToken( tokty_then );
        Node e = this->readExpr();
        ftry.add( e );

        ftry.end();
    }

    if ( this->tryToken( tokty_else ) ) {
        ftry.start( "catch.else" );
        Node else_stmnts = this->readStmnts();
        ftry.add( else_stmnts );
        ftry.end();
    }

    this->checkToken( try_vs_transaction? tokty_endtry : tokty_endtransaction );
    ftry.end();
    return ftry.build();
}

Node ReadStateClass::readVarVal( TokType fnc ) {
    NodeFactory bind;
    bind.start( Ginger::GNX_BIND );
    
    Node lhs = this->readExprPrec( prec_assign );
    updateAsPattern( lhs, fnc == tokty_val );

    bind.add( lhs );
    this->checkToken( this->cstyle_mode ? tokty_equal : tokty_bind );
    Node x = this->readExpr();
    bind.add( x );
    bind.end();
    return bind.build();
}

Node ReadStateClass::readElement() {
    NodeFactory element;
    element.start( Ginger::GNX_SEQ );
    for (;;) {
        element.start( Ginger::GNX_SYSAPP );
        element.put( Ginger::GNX_SYSAPP_NAME, "newElement" );
        element.start( Ginger::GNX_SYSAPP );
        element.put( Ginger::GNX_SYSAPP_NAME, "newAttrMap" );

        // Cache the element name for the close tag. Anything other than
        // a literal name will need the close-anything tag.
        std::string element_name;
        Item item = this->item_factory->peek();
        if ( item->tok_type == tokty_id ) {
            element_name = item->nameString();
        }

        Node name = this->readAtomicExpr();
        element.add( name );    

        bool closed = false;
        for (;;) {
            if ( this->tryToken( tokty_gt ) ) break;
            if ( this->tryToken( tokty_slashgt ) ) { closed = true; break; }

            Node key_or_keyvalue = this->readAtomicExpr( false );

            if ( this->tryToken( tokty_equal ) ) {
                Node a1 = makeAssert1( key_or_keyvalue );
                element.add( a1 );
                Node value = this->readAtomicExpr();
                element.add( value );
            } else {
                element.start( Ginger::GNX_SYSAPP );
                element.put( Ginger::GNX_SYSAPP_NAME, "explodeMapsAndMaplets" );
                element.add( key_or_keyvalue );
                element.end();
            }
        }
        element.end();  // newAttrMap.
        if ( not closed ) {
            //  Read the children.
            bool ended = false;
            for (;;) {
                if ( this->tryToken( tokty_ltslash ) ) break;
                if ( this->tryToken( tokty_endelement ) ) { ended = true; break; }
                Node child = this->readExpr();
                element.add( child );
            }
            if ( not ended ) {
                const string ename( this->readIdName() );
                if ( ename != element_name ) {
                    throw CompileTimeError( "Element close tag does not match open tag" ).culprit( "Open tag", element_name ).culprit( "Close tag", ename );
                }
                this->checkToken( tokty_gt );
            }
        }
        element.end();  //  newElement.

        //  Uniquely, at this point in the language, no semi-colon is
        //  needed if the next item is ANOTHER element.
        if ( not this->tryToken( tokty_lt ) ) break;
    }           
    element.end();
    Node answer = element.build();
    return answer->size() == 1 ? answer->getChild( 0 ) : answer;
}

Node ReadStateClass::prefixProcessing() {
    ItemFactory ifact = this->item_factory;
    const int start = ifact->lineNumber();
    Node node = this->prefixProcessingCore();
    if ( node && this->span_mode ) {
        const int end = ifact->lineNumber();
        stringstream span;
        span << start;
        if ( start != end ) {
            span << ";" << end;
        }
        string spanstr( span.str() );
        string spanspan( Ginger::GNX_SPAN );
        node->putAttribute( spanspan, spanstr );
    }
    return node;
}

Node ReadStateClass::readListOrVector( bool vector_vs_list, TokType closer ) {
    NodeFactory list;
    list.start( vector_vs_list ? Ginger::GNX_VECTOR : Ginger::GNX_LIST );
    Node stmnts = this->readCompoundCore();
    if ( not vector_vs_list && this->tryToken( tokty_bar ) ) {
        list.add( stmnts );
        list.end();
        Node L = list.build();
        NodeFactory append;
        append.start( Ginger::GNX_LIST_APPEND );
        append.add( L );
        Node x = this->readCompoundCoreCheck( closer );
        append.add( x );
        append.end();
        return append.build();
    } else {
        this->checkToken( closer );
        list.add( stmnts );
        list.end();
        return list.build();
    }
}

Node ReadStateClass::readMap( TokType closer ) {
    NodeFactory list;
    list.start( Ginger::GNX_SYSAPP );
    list.put( Ginger::GNX_SYSAPP_NAME, "newMap" );
    Node x = this->readStmntsCheck( closer );
    list.add( x );
    list.end();
    return list.build();
}

Node ReadStateClass::readSwitchStmnts() {
    NodeFactory st;
    st.start( Ginger::GNX_SEQ );
    while (
        not ( 
            this->tryPeekToken( tokty_cbrace ) ||
            this->tryPeekToken( tokty_case ) ||
            this->tryPeekToken( tokty_default )
        ) 
    ) {
        Node x = this->readSingleStmnt();
        st.add( x );
    }
    st.end();
    return st.build();
}

Node ReadStateClass::readSwitch() {
    NodeFactory sw;
    sw.start( Ginger::GNX_SWITCH );

    bool else_seen = false;
    Node swelse;

    if ( this->cstyle_mode ) {
        
        this->checkToken( tokty_oparen );
        Node swvalue = this->readExprCheck( tokty_cparen );
        sw.add( swvalue );
    
        this->checkToken( tokty_obrace );
    
        for (;;) {
            if ( this->tryToken( tokty_case ) ) {
                Node e = this->readExpr();
                sw.add( e );
                this->checkToken( tokty_colon );
                Node b = this->readSwitchStmnts();
                sw.add( b );
            } else if ( this->tryToken( tokty_default ) ) {
                if ( else_seen ) {
                    throw CompileTimeError( "Switch with two default parts" );
                }
                else_seen = true;
                this->checkToken( tokty_colon );
                swelse = this->readSwitchStmnts();
            } else {
                break;
            }
        }
        this->checkToken( tokty_cbrace );

    } else {
        Node swvalue = this->readExpr();
        sw.add( swvalue );

        for (;;) {
            if ( this->tryToken( tokty_case ) ) {
                Node e = this->readExprCheck( tokty_then );
                sw.add( e );
                Node b = this->readStmnts();
                sw.add( b );
            } else if ( this->tryToken( tokty_else ) ) {
                if ( else_seen ) {
                    throw CompileTimeError( "Switch with two else parts" );
                }
                else_seen = true;
                swelse = this->readOptExpr();
            } else {
                break;
            }
        }
        this->checkToken( tokty_endswitch );
    }

    if ( else_seen ) {
        sw.add( swelse );
    }

    sw.end();
    return sw.build();
}

Node ReadStateClass::readThrow() {
    NodeFactory panic;
    panic.start( "throw" );

    panic.put( "event", this->readIdName() );
    panic.put(
        "level",
        this->tryToken( tokty_bang ) ? "rollback" :
        this->tryToken( tokty_dbang ) ? "failover" :
        this->tryToken( tokty_panic ) ? "panic" :
        "escape"
    );
    if ( this->tryPeekToken( tokty_oparen ) ) {
        Node e = this->readExpr();
        panic.add( e );
    } else {
        panic.start( Ginger::GNX_SEQ );
        panic.end();
    }

    panic.end();
    return panic.build();
}

Node ReadStateClass::readId( const std::string name ) {
    SysConst * sysc = lookupSysConst( name );
    if ( sysc != NULL ) {
        NodeFactory constant;
        constant.start( Ginger::GNX_CONSTANT );
        constant.put( Ginger::GNX_CONSTANT_TYPE, sysc->tag );
        constant.put( Ginger::GNX_CONSTANT_VALUE, sysc->value );
        constant.end();
        return constant.build();
    } else {
        NodeFactory id;
        id.start( this->pattern_mode ? Ginger::GNX_VAR : Ginger::GNX_ID );
        id.put( Ginger::GNX_VID_NAME, name );
        id.end();
        return id.build();
    }
}

Node ReadStateClass::readAnon( const std::string name ) {
    NodeFactory anon;
    if ( this->pattern_mode ) {
        anon.start( Ginger::GNX_VAR );
    } else {
        anon.start( Ginger::GNX_CONSTANT );
        anon.put( Ginger::GNX_CONSTANT_TYPE, "absent" );
        anon.put( Ginger::GNX_CONSTANT_VALUE, "absent" );
        anon.put( CONSTANT_WAS_ANON, name );
    }
    anon.put( Ginger::GNX_COMMENT, name );
    anon.end();
    return anon.build();
}

Node ReadStateClass::readEnvVar() {
    this->checkToken( tokty_obrace );
    NodeFactory envvar;
    envvar.start( Ginger::GNX_SYSAPP );
    envvar.put( "name", "sysGetEnv" );
    envvar.start( Ginger::GNX_CONSTANT );
    envvar.put( Ginger::GNX_CONSTANT_TYPE, "string" );
    envvar.put( Ginger::GNX_CONSTANT_VALUE, this->item_factory->read()->nameString() );
    envvar.end();       
    this->checkToken( tokty_cbrace );
    envvar.end();
    return envvar.build();
}

Node ReadStateClass::readDHat() {
    NodeFactory maplet;
    maplet.start( Ginger::GNX_SYSAPP );
    maplet.put( "name", "newMaplet" );
    const string name( this->readIdName() );
    maplet.start( "constant" );
    maplet.put( "type", "symbol" );
    maplet.put( "value", name );
    maplet.end();
    maplet.start( "id" );
    maplet.put( "name", name );
    maplet.end(); 
    maplet.end();
    return maplet.build();
}

Node ReadStateClass::readPackage() {
    NodeFactory pkg;
    pkg.start( "package" );
    string url = this->readPkgName();
    pkg.put( "url", url );
    this->checkToken( tokty_semi );
    Node body = this->readStmntsCheck( tokty_endpackage );
    pkg.add( body );
    pkg.end();
    return pkg.build();
}

Node ReadStateClass::readImport() {
    NodeFactory imp;
    imp.start( "import" );
    bool pervasive, qualified;
    
    readImportQualifiers( *this, pervasive, qualified );
    imp.put( "pervasive", pervasive ? "true" : "false" );
    imp.put( "qualified", qualified ? "true" : "false" );
    
    readImportMatch( *this, imp );
    
    this->checkToken( tokty_from );
    string url = this->readPkgName();
    imp.put( "from", url );

    if ( this->tryName( "alias" ) ) {
        imp.put( "alias", this->readIdName() );
    }
    
    if ( this->tryName( "into" ) ) {
        readImportInto( *this, imp );
    }
    
    imp.end();
    return imp.build();
}

Node ReadStateClass::readReturn() {
    NodeFactory ret;
    ret.start( Ginger::GNX_ASSERT );
    ret.put( Ginger::GNX_ASSERT_TAILCALL, "true" );
    Node n = this->readExpr();
    ret.add( n );
    ret.end();
    return ret.build();
}

/*
Transforms 
    recordclass <RNAME>
        slot <NAME1>;
        slot <NAME2>;
        ...
        slot <NAME_N>;
    endrecordclass;
into
    <seq>
        <bind>
            <var name="${RNAME}"/>
            <sysapp value="newRecordClass">
                <constant type="string" value="${RNAME}"/>
                <constant type="int" value="${N}"/>
            </sysapp>
        </bind>
        <bind>
            <var name="new${RNAME}"/>
            <sysapp value="classConstructor"/>
                <id name="${RNAME}"/>
            </sysapp>
        </bind>
        <!-- one for each slot -->
        <bind>
            <var name="${NAME_1}"/>
            <sysapp value="classAccessor"/>
                <id name="${RNAME}"/>
                <constant type="int" value="1"/>
            </sysapp>
        </bind>
        ....
        <bind>
            <var name="${NAME_N}"/>
            <sysapp value="classAccessor"/>
                <id name="${RNAME}"/>
                <constant type="int" value="${N}"/>
            </sysapp>
        </bind>
    </seq>
*/
Node ReadStateClass::readRecordClass() {
    
    vector< string > slot_names;
    const string class_name( this->readIdName() );
    if ( this->cstyle_mode ) this->checkToken( tokty_obrace );
    while ( this->tryToken( tokty_slot ) ) {
        slot_names.push_back( this->readIdName() );
        this->checkToken( tokty_semi );
    }
    this->checkToken( this->cstyle_mode ? tokty_cbrace : tokty_endrecordclass );

    NodeFactory f;
    f.start( Ginger::GNX_SEQ );

    //  Datakey.
    f.start( Ginger::GNX_BIND );
    f.start( Ginger::GNX_VAR ); 
    f.put( Ginger::GNX_VID_NAME, class_name );
    f.end(); // VAR
    f.start( Ginger::GNX_SYSAPP );
    f.put( Ginger::GNX_SYSAPP_NAME, "newRecordClass" );
    pushConstant( f, "string", class_name );        //  CONSTANT
    pushConstant( f, "int", slot_names.size() );    // CONSTANT
    f.end();    //  SYSAPP
    f.end(); // BIND

    //  Constructor
    f.start( Ginger::GNX_BIND );
    f.start( Ginger::GNX_VAR ); 
    {
        stringstream s;
        s << "new";
        s << class_name;
        f.put( Ginger::GNX_VID_NAME, s.str() );
    }
    f.end(); // VAR
    f.start( Ginger::GNX_SYSAPP );
    f.put( Ginger::GNX_SYSAPP_NAME, "newClassConstructor" );
    f.start( Ginger::GNX_ID );
    f.put( Ginger::GNX_VID_NAME, class_name );
    f.end();    //  ID
    f.end();    //  SYSAPP
    f.end(); // BIND

    //  Exploder
    f.start( Ginger::GNX_BIND );
    f.start( Ginger::GNX_VAR );
    {
        stringstream s;
        s << "explode";
        s << class_name;
        f.put( Ginger::GNX_VID_NAME, s.str() );     
    }
    f.end();        //  VAR
    f.start( Ginger::GNX_SYSAPP );
    f.put( Ginger::GNX_SYSAPP_NAME, "newClassExploder" );
    f.start( Ginger::GNX_ID );
    f.put( Ginger::GNX_VID_NAME, class_name );
    f.end();        //  ID
    f.end();        //  SYAPP
    f.end();        //  BIND

    //  Recogniser
    f.start( Ginger::GNX_BIND );
    f.start( Ginger::GNX_VAR ); 
    {
        stringstream s;
        s << "is";
        s << class_name;
        f.put( Ginger::GNX_VID_NAME, s.str() );
    }
    f.end(); // VAR
    f.start( Ginger::GNX_SYSAPP );
    f.put( Ginger::GNX_SYSAPP_NAME, "newClassRecogniser" );
    f.start( Ginger::GNX_ID );
    f.put( Ginger::GNX_VID_NAME, class_name );
    f.end();    //  ID
    f.end();    //  SYSAPP
    f.end(); // BIND

    for ( size_t i = 0; i < slot_names.size(); i++ ) {
        const string & ith_name( slot_names[ i ] );

        f.start( Ginger::GNX_BIND );
        f.start( Ginger::GNX_VAR ); 
        f.put( Ginger::GNX_VID_NAME, ith_name );
        f.end(); // VAR
        f.start( Ginger::GNX_SYSAPP );
        f.put( Ginger::GNX_SYSAPP_NAME, "newClassAccessor" );
        f.start( Ginger::GNX_ID );
        f.put( Ginger::GNX_VID_NAME, class_name );
        f.end();    //  ID
        pushConstant( f, "int", i + 1 ); // CONSTANT
        f.end();    //  SYSAPP
        f.end(); // BIND

    }

    f.end(); // SEQ
    return f.build();
}

Node ReadStateClass::prefixProcessingCore() {
    ItemFactory ifact = this->item_factory;
    Item item = ifact->read();
    
    #ifdef DBG_COMMON2GNX
        cerr << "First item was " << tok_type_name( item->tok_type ) << endl;
    #endif
    
    TokType fnc = item->tok_type;
    Role role = item->role;

    //cout << "PREFIX PROCESSING: " << item->nameString() << endl;
    //cout << "  tokty = " << tok_type_name( fnc ) << endl;

    if ( role.IsLiteral() ) {
        NodeFactory simple;
        simple.start( "constant" );
        simple.put( "type", tok_type_as_type( fnc ) );
        simple.put( "value", item->nameString() );
        simple.end();
        return simple.build();
    } else if ( role.IsUnary() ) {
        if ( role.IsForm() ) {
            NodeFactory unary;
            unary.start( tok_type_as_tag( fnc ) );
            Node x = this->readExprPrec( item->precedence );
            unary.add( x );
            unary.end();
            return unary.build();
        } else if ( role.IsSys() ) {
            NodeFactory sf;
            sf.start( Ginger::GNX_SYSAPP );
            sf.put( Ginger::GNX_SYSAPP_NAME, tok_type_as_sysapp( fnc ) );
            Node x = this->readExprPrec( item->precedence );
            sf.add( x );
            sf.end();
            return sf.build();
        } else {
            throw CompileTimeError( "Internal error - unreachable code reached" );
        }
    }

    switch ( fnc ) {
        case tokty_sub: {
            Node e = this->readExprPrec( prec_negate );
            NodeFactory neg;
            neg.start( Ginger::GNX_SYSAPP );
            neg.put( Ginger::GNX_SYSAPP_NAME, "negate" );
            neg.add( e );
            neg.end();
            return neg.build();
        }
        case tokty_add: {
            Node e = this->readExprPrec( prec_negate );
            NodeFactory pos;
            pos.start( Ginger::GNX_SYSAPP );
            pos.put( Ginger::GNX_SYSAPP_NAME, "positivate" );
            pos.add( e );
            pos.end();
            return pos.build();         
        }
        case tokty_id: 
            return this->readId( item->nameString() );
        case tokty_anon:
            return this->readAnon( item->nameString() );
        // changed for ${VAR} case study
        case tokty_envvar: 
            return this->readEnvVar();
        case tokty_return: 
            return this->readReturn();
        case tokty_throw: 
            return this->readThrow();
        case tokty_try: 
        case tokty_transaction: 
            return this->readTry( fnc == tokty_try );
        case tokty_charseq: 
            return makeCharSequence( item );
        case tokty_val:
        case tokty_var : 
            return this->readVarVal( fnc );
        case tokty_oparen: {
            if ( this->cstyle_mode ) {
                return this->readOptEmptyExprCheck( tokty_cparen );
            } else {
                return this->readStmntsCheck( tokty_cparen );
            }
        }
        case tokty_obracket:
            return this->readListOrVector( true, tokty_cbracket );
        case tokty_obrace: 
            return this->readMap( tokty_cbrace );
        case tokty_fat_obracket:
            return this->readListOrVector( false, tokty_fat_cbracket );
        case tokty_fat_ocbracket: {
            Node list( new Ginger::Mnx( Ginger::GNX_LIST ) );
            return list;
        }
        case tokty_unless:
            return this->readIf( tokty_unless, tokty_endunless );
        case tokty_if: {
            if ( this->cstyle_mode ) break;
            return this->readIf( tokty_if, tokty_endif );
        }
        case tokty_syscall:
            return this->readSyscall();
        case tokty_for: {
            if ( this->cstyle_mode ) break;
            return this->readFor();
        }
        case tokty_function: {
            #ifdef DBG_COMMON2GNX
                cerr << "FUNCTION" << endl;
            #endif
            if ( this->item_factory->peek()->tok_type == tokty_oparen ) {
                return this->readLambda();
            } else {
                break;
            }
        }
        case tokty_define:
            //cerr << "DEFINE" << endl;
            return this->readDefinition();
        case tokty_fn: 
            return this->readLambda();
        case tokty_lt:
            return this->readElement();
        case tokty_package: 
            return this->readPackage();
        case tokty_import: 
            return this->readImport();
        case tokty_dhat:
            return this->readDHat();
        case tokty_switch: {
            if ( this->cstyle_mode ) break;
            return this->readSwitch();
        }
        case tokty_recordclass:
            return this->readRecordClass();
        default: 
            {}
    }
    ifact->unread();
    return Node();
}

bool ReadStateClass::isAtEndOfInput() {
    ItemFactory ifact = this->item_factory;
    Item it = ifact->peek();
    return it->tok_type == tokty_eof;
}

Node ReadStateClass::readOptExprPrec( int prec ) {
    ItemFactory ifact = this->item_factory;
    Node e = this->prefixProcessing();
    if ( not e ) return Node();
    //cerr << "starting postfix checking ... " << this->isPostfixAllowed() << endl;
    while ( this->isPostfixAllowed() ) {
        int q;
        //cerr << "peeking" << endl;
        Item it = ifact->peek();
        if ( it->item_is_signed_num() ) {
            NodeFactory t;
            t.start( Ginger::GNX_SYSAPP );
            t.put( "name", "+" );
            t.add( e );         
            if ( it->tok_type == tokty_int ) {
                t.start( "constant" );
                t.put( "type", "int" );
                t.put( "value", it->nameString() );
                t.end();
            } else {
                throw CompileTimeError( "Only integers supported so far" ).culprit( "Item", it->nameString() );
            }
            t.end();
            e = t.build();
            ifact->drop();
        } else if ( it->item_is_postfix() ) {
            q = it->precedence;
            if ( q >= prec ) break;
            ifact->drop();
            e = this->postfixProcessing( e, it, q );
        } else {
            break;
        }
    }
    return e;
}

