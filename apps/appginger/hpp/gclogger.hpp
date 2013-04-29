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

#include <fstream>

#include "gctracker.hpp"
#include "common.hpp"

namespace Ginger {

class GCLogger  : public GCTracker {
private:
	bool 					scan_call_stack;
	std::ofstream			gclog;

public:
	void atRef( Ref current );
	void afterAtRef( Ref current );
	void atVariable( std::string var );
	void startValueStack();
	void endValueStack();
	void startCallStackAndPC();
	void endCallStackAndPC();
	void startDictionary();
	void endDictionary();
	void startRegisters();
	void endRegisters();
	void startGarbageCollection();
	void endGarbageCollection();
	void pickedObjectToCopy( Ref * obj );
	void startContents( Ref * obj_K );
	void endContents();
	void startFnObj();
	void endFnObj();
	void startInstruction( FnObjCrawl & fnobjcrawl );
	void endInstruction( FnObjCrawl & fnobjcrawl );
	void startVector( Ref * obj_K );
	void endVector( Ref * obj_K );
	void startRecord( Ref * obj_K );
	void endRecord( Ref * obj_K );
	void startInstance( Ref * obj_K );
	void endInstance( Ref * obj_K );
	void atString( Ref * obj_K );
	
public:
	GCLogger( const bool scs );
	~GCLogger();
};

} // namespace Ginger