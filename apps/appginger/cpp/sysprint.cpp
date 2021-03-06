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

#include <string>
#include <sstream>
#include <iostream>


#include "sysprint.hpp"

#include "refprint.hpp"
#include "machine.hpp"
#include "sys.hpp"  
#include "cell.hpp"
  


//#define DBG_SYSPRINT

namespace Ginger {
using namespace std;

Ref * sysRefPrint( Ref * pc, class MachineClass * vm ) {
	int column = 0;
	RefPrint printer( std::cout, column );
	for ( int i = vm->count - 1; i >= 0; i-- ) {
		Ref r = vm->fastSubscr( i );
		printer.refPrint( r );		
	}
	vm->fastDrop( vm->count );
	return pc;
}
SysInfo infoRefPrint( 
	FullName( "print" ), 
	Ginger::Arity( 0, true ), 
	Ginger::Arity( 0 ), 
	sysRefPrint, 
	"Prints any values in reading format" 
);

Ref * sysRefPrintln( Ref * pc, class MachineClass * vm ) {
	pc = sysRefPrint( pc, vm );
	std::cout << std::endl;
	return pc;
}
SysInfo infoRefPrintln( 
	FullName( "println" ), 
	Ginger::Arity( 0, true ), 
	Ginger::Arity( 0 ), 
	sysRefPrintln, 
	"Prints any values in reading format and then adds a new line" 
);


// - showMePrint ---------------------------------------------------------------


Ref * sysRefShow( Ref * pc, class MachineClass * vm ) {
	int column = 0;
	RefPrint printer( std::cout, column, RefPrint::SHOW );
	for ( int i = vm->count - 1; i >= 0; i-- ) {
		Ref r = vm->fastSubscr( i );
		printer.refPrint( r );		
		if ( i != 0 ) std::cout << ",";
	}
	vm->fastDrop( vm->count );
	return pc;
}
SysInfo infoSysPrint( 
	FullName( "sysPrint" ), 
	Ginger::Arity( 1, true ), 
	Ginger::Arity( 0 ), 
	sysRefShow, 
	"Prints any values in summary format"
);

// - showMe --------------------------------------------------------------------


Ref * sysRefShowln( Ref * pc, class MachineClass * vm ) {
	pc = sysRefShow( pc, vm );
	std::cout << std::endl;
	return pc;
}
SysInfo infoSysPrintln( 
	FullName( "showMe" ), 
	Ginger::Arity( 1, true ), 
	Ginger::Arity( 0 ), 
	sysRefShowln, 
	"Prints any values in summary format and then adds a new line" 
);
RegisterSysAltName altSysPrintln( infoSysPrintln, "ginger.library", "sysPrintln" );

//------------------------------------------------------------------------------

void refPrint( Ref r ) {
	int column = 0;
	RefPrint printer( std::cout, column );
	printer.refPrint( r );
}

void refShow( const Ref r ) {
	int column = 0;
	RefPrint printer( std::cout, column, RefPrint::SHOW );
	printer.refPrint( r );
}

void refPrint( std::ostream & out, const Ref r ) {
	int column = 0;
	RefPrint printer( out, column );
	printer.refPrint( r );
}

void refShow( std::ostream & out, const Ref r ) {
	int column = 0;
	RefPrint printer( out, column, RefPrint::SHOW );
	printer.refPrint( r );
}

void refPtrPrint( Ref * r ) {
	refPrint( Ptr4ToRef( r ) );
}

void refPtrPrint( std::ostream & out, Ref * r ) {
	refPrint( out, Ptr4ToRef( r ) );
}

std::string refToPrintString( Ref ref ) {
	std::ostringstream s;
	int column = 0;
	RefPrint printer( s, column, RefPrint::PRINT );
	printer.refPrint( ref );
	return s.str();
}

std::string refToShowString( Ref ref ) {
	std::ostringstream s;
	int column = 0;
	RefPrint printer( s, column, RefPrint::SHOW );
	printer.refPrint( ref );
	return s.str();
}


// - showMeRuntimeInfo ---------------------------------------------------------

Ref * sysShowMeRuntimeInfo( Ref * pc, class MachineClass * vm ) {
	vm->getAppContext().showMeRuntimeInfo();
	return pc;
}
SysInfo infoShowMeRuntimeInfo(
	FullName( "showMeRuntimeInfo" ),
	Ginger::Arity( 0 ), 
	Ginger::Arity( 0 ), 
	sysShowMeRuntimeInfo, 
	"Prints the runtime info to stdout"
);
RegisterSysAltName altShowMeRuntimeInfo( infoShowMeRuntimeInfo, "ginger.library", "gingerInfo" );

// - printf & printfln ---------------------------------------------------------
//	
//	%p = print
//	%s = sysprint
//


void gngPrintf( ostream & out, Ref * pc, class MachineClass * vm ) {
	if ( vm->count < 1 ) {
		throw Ginger::Mishap( "Too few arguments" );
	}
	
	//	The control string is the first argument.
	std::string control( Cell( vm->fastPeek( vm->count - 1 ) ).asHeapObject().asStringObject().getString() );

	int column = 0;
	RefPrint printer( out, column );
	
	//	Iterate over the control string.
	int index = vm->count - 2;
	for ( 
		size_t i = 0;
		i < control.size();
		i++
	) {
		const char ch = control[ i ];
		if ( ch != '%' ) {
			out << ch;
		} else if ( i + 1 < control.size() ) {
			if ( index < 0 ) {
				throw Ginger::Mishap( "Not enough arguments for format string" ).culprit( "Format string", control );
			}
			const char next = control[ ++i ];
			switch ( next ) {
				case 's': 
				case 'p': {
					Cell( vm->fastPeek( index-- ) ).print( out, next == 's' );
					break;
				}
				case '%' : {
					out << ch;
					break;
				}
				default: {
					cerr << "Invalid character after % in format string: " << ch << endl;
					break;
				}
			}
		} else {
			out << ch;
		}
	}
}

Ref * sysPrintf( Ref * pc, class MachineClass * vm ) {
	gngPrintf( std::cout, pc, vm );
	vm->fastDrop( vm->count );
	return pc;
}
SysInfo infoPrintf( 
	FullName( "printf" ), 
	Ginger::Arity( 1, true ), 
	Ginger::Arity( 0 ), 
	sysPrintf, 
	"Formatted printing" 
);

Ref * sysPrintfln( Ref * pc, class MachineClass * vm ) {
	pc = sysPrintf( pc, vm );
	std::cout << std::endl;
	return pc;
}
SysInfo infoPrintfln( 
	FullName( "printfln" ), 
	Ginger::Arity( 1, true ), 
	Ginger::Arity( 0 ), 
	sysPrintfln, 
	"Formatted printing and then adds a new line" 
);

Ref * sysStringf( Ref * pc, class MachineClass * vm ) {
	std::ostringstream str;
	gngPrintf( str, pc, vm );
	vm->fastDrop( vm->count );
	vm->fastPush( vm->heap().copyString( str.str().c_str() ) );	//	May cause garbage collection!
	return pc;
}
SysInfo infoStringf( 
	FullName( "stringf" ), 
	Ginger::Arity( 1, true ), 
	Ginger::Arity( 0 ), 
	sysStringf, 
	"Returns a formatted string" 
);

// - stringPrint ---------------------------------------------------------------

static void gngStringPrint( ostream & out, Ref * pc, class MachineClass * vm ) {
	int column;
	RefPrint printer( out, column );
	//	Iterate over the value stack.
	for ( int index = vm->count - 1; index >= 0; index -= 1 ) {
		printer.refPrint( vm->fastPeek( index ) );
	}
}

Ref * sysStringPrint( Ref * pc, class MachineClass * vm ) {
	std::ostringstream out;
	gngStringPrint( out, pc, vm );
	vm->fastDrop( vm->count );
	if ( vm->count == 0 ) {
		vm->checkStackRoom( 1 );
	}
	vm->fastPush( vm->heap().copyString( pc, out.str().c_str() ) );
	return pc;
}
SysInfo infoStringPrint( 
	FullName( "stringPrint" ), 
	Ginger::Arity( 0, true ), 
	Ginger::Arity( 1 ), 
	sysStringPrint, 
	"Concatenates the print strings of an arbitrary number of items" 
);

} // namespace Ginger
