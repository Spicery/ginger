/******************************************************************************\
	Copyright (c) 2010 Stephen Leach. AppGinger is distributed under the terms 
	of the GNU General Public License. This file is part of Ginger.

    Ginger is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Ginger is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Ginger.  If not, see <http://www.gnu.org/licenses/>.
\******************************************************************************/

#include <set>
#include <map>
#include <string>
#include <iostream>

#include "key.hpp"
#include "mishap.hpp"
#include "misclayout.hpp"
#include "stringlayout.hpp"
#include "syssymbol.hpp"

namespace Ginger {
using namespace std;

/*
	Serial numbers must lie in the range 0-MAX_SERIAL where
	MAX_SERIAL = 2^( sizeof( unsigned long ) - TAGGG ) - 1
*/
#define MAX_SERIAL_WIDTH ( sizeof( unsigned long ) - TAGGG )
#define MAX_SERIAL ( (1 << MAX_SERIAL_WIDTH ) - 1 )

#define SymbolSerial( r ) 	( ToULong(r) >> LENGTH_OFFSET )

//	Converts std::strings to serial numbers.
typedef std::map< const std::string, unsigned long > SYMTAB;
typedef std::map< unsigned long, const std::string > REVSYMTAB;
typedef std::set< unsigned long > ACTIVE;

static unsigned long next_serial = 0;
static SYMTAB symtab;
static REVSYMTAB revsymtab;
static ACTIVE active;

static bool available( unsigned long serial ) {
	return revsymtab.find( serial ) == revsymtab.end();
}

static unsigned long nextSerial() {
	bool resetted = false;
	for (;;) {
		if ( available( next_serial ) ) return next_serial++ ;
		if ( next_serial > MAX_SERIAL ) {
			if ( resetted ) {
				throw Ginger::Mishap( "Symbol table is full" );
			} else { 
				next_serial = 0;
				resetted = true;
			}
		}
	}
}

void preGCSymbolTable( const bool gctrace ) {
	active.clear();
	if ( gctrace ) {
		cerr << "### Before GC: Size of symbol table " << symtab.size() << endl;
	}
}

void postGCSymbolTable( const bool gctrace ) {
	for ( 
		REVSYMTAB::iterator it = revsymtab.begin();
		it != revsymtab.end();
	) {
		if ( active.find( it->first ) == active.end() ) {
			symtab.erase( it->second );
			revsymtab.erase( it++ );
		} else {
			++it;
		}
	}
	active.clear();
	if ( gctrace ) {
		cerr << "### After GC: Size of symbol table " << symtab.size() << endl;
	}
}

void gcTouchSymbol( Ref r ) {
	active.insert( SymbolSerial( r ) );
}

Ref refMakeSymbol( const std::string & s ) {
	unsigned serial;
	SYMTAB::const_iterator it = symtab.find( s );
	if ( it == symtab.end() ) {
		serial = nextSerial();
		symtab.insert( std::pair< const std::string, unsigned int >( s, serial ) );
		revsymtab.insert( std::pair< unsigned int, const std::string >( serial, s ) );
	} else {
		serial = it->second;
	}
	return IntToRef( ( serial << TAGGG ) | SYMBOL_TAGGG );
}

/*
	On the top of the stack should be a VM string.
*/
Ref * sysMakeSymbol( Ref *pc, MachineClass * vm ) {
	if ( vm->count == 1 ) {
		Ref r = vm->fastPeek();
		if ( IsString( r ) ) {
			Ref * str_K = RefToPtr4( r );
			const std::string s( ToChars( str_K + 1 ), lengthOfStringLayout( str_K ) );
			vm->fastPeek() = refMakeSymbol( s );
			return pc;
		} else {
			throw Ginger::Mishap( "String needed for newSymbol" );
		}
	} else {
		throw Ginger::Mishap( "Wrong number of arguments for newSymbol" );
	}
}

extern const std::string & symbolToStdString( Ref r ) {
	REVSYMTAB::iterator it = revsymtab.find( SymbolSerial( r ) );
	if ( it != revsymtab.end() ) {
		return it->second;
	} else {
		throw SystemError( "Corrupt symbol table" );
	}
}

} // namepsace Ginger
