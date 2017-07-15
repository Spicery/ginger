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

#include "gclogger.hpp"

namespace Ginger {
using namespace std;


GCLogger::GCLogger( const bool scs ) :
	scan_call_stack( scs )
{
	gclog.open( "gc.log", ios_base::app | ios_base::out );
}

GCLogger::~GCLogger() {
	gclog.close();
}

void GCLogger::atRef( Ref current ) {	
	gclog << "      Forwarding ref = 0x" << hex << ToULong( current ) << endl;
	gclog.flush();
	gclog << "        Is obj? " << IsObj( current ) << endl;
	gclog.flush();
}

void GCLogger::afterAtRef( Ref current ) {
	gclog << "        Forwarded to ref = 0x" << hex << ToULong( current ) << endl;
	gclog.flush();
}

void GCLogger::atVariable( std::string var ) {	
	gclog << "Forwarding " << var << endl;
}

void GCLogger::startValueStack() {
	gclog << "Forwarding value stack" << endl;
}

void GCLogger::endValueStack() {
}

void GCLogger::startCallStackAndPC() {
	gclog << "Forwarding call stack and pc" << endl;
}
		
void GCLogger::endCallStackAndPC() {
}

void GCLogger::startDictionary() {
	gclog << "Forward dictionary" << endl;
}

void GCLogger::endDictionary() {
}

void GCLogger::startDynamicRoots() {
	gclog << "Forward dynamic roots" << endl;
}

void GCLogger::endDynamicRoots() {
}

void GCLogger::startGarbageCollection() {
	gclog << "### " << ( scan_call_stack ? "" : "Quiescent " ) << "GC" << endl;	
}

void GCLogger::endGarbageCollection() {
	gclog << "Finished GC (" << ( scan_call_stack ? "" : "Quiescent " ) << ")" << endl;	
}

void GCLogger::pickedObjectToCopy( Ref * obj ) {
	gclog << "Picked " << obj << endl;
	gclog.flush();
}

void GCLogger::startContents( Ref * obj_K ) {
	gclog << "  Advance contents for " << keyName( *obj_K ) << endl;
}

void GCLogger::endContents() {
	gclog << "  Finished forwarding contents" << endl;
	gclog.flush();
}

void GCLogger::startFnObj() {
	gclog << "  (Function object)" << endl;
}

void GCLogger::endFnObj() {
	gclog << "  (Function object)" << endl;
}

void GCLogger::startInstruction( FnObjCrawl & fnobjcrawl ) {
	gclog << "    At instruction " << fnobjcrawl.getName() << endl;
	gclog << "       position    " << fnobjcrawl.getPosn() << endl;
}

void GCLogger::endInstruction( FnObjCrawl & fnobjcrawl ) {
	gclog << "    Forwarded " << fnobjcrawl.getName() << endl;
}

void GCLogger::startVector( Ref * obj_K ) {
	gclog << "  (Vector)" << endl;
}
				
void GCLogger::endVector( Ref * obj_K ) {
}
				
void GCLogger::startRecord( Ref * obj_K ) {
	gclog << "  (Record)" << endl;
}
				
void GCLogger::endRecord( Ref * obj_K ) {
}
		
void GCLogger::startInstance( Ref * obj_K ) {
	gclog << "  (Instance)" << endl;
}
				
void GCLogger::endInstance( Ref * obj_K ) {
}
		
void GCLogger::atString( Ref * obj_K ) {
	gclog << "  (String)" << endl;
}

} // namespace Ginger
