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

#ifndef GC_TRACKER_HPP
#define GC_TRACKER_HPP

#include "common.hpp"
#include "fnobjcrawl.hpp"

namespace Ginger {

class GCTracker {
public:
	virtual long changed() { return 0; }
	virtual void reset() {}
	virtual void copy1() {}
	virtual void prune1() {}
	virtual void retain1() {}
	virtual long assocsPruned() { return 0; }
	virtual long assocsRetained() { return 0; }

public:
	virtual void atRef( Ref current ) {}
	virtual void afterAtRef( Ref current ) {}
	virtual void atVariable( std::string var ) {}
	virtual void startValueStack() {}
	virtual void endValueStack() {}
	virtual void startCallStackAndPC() {}
	virtual void endCallStackAndPC() {}
	virtual void startDictionary() {}
	virtual void endDictionary() {}
	virtual void startDynamicRoots() {}
	virtual void endDynamicRoots() {}
	virtual void startGarbageCollection() {}
	virtual void endGarbageCollection() {}
	virtual void pickedObjectToCopy( Ref * obj ) {}
	virtual void startContents( Ref * obj_K ) {}
	virtual void endContents() {}
	virtual void startFnObj() {}
	virtual void endFnObj() {}
	virtual void startInstruction( FnObjCrawl & fnobjcrawl ) {}
	virtual void endInstruction( FnObjCrawl & fnobjcrawl ) {}
	virtual void startVector( Ref * obj_K ) {}
	virtual void endVector( Ref * obj_K ) {}
	virtual void startRecord( Ref * obj_K ) {}
	virtual void endRecord( Ref * obj_K ) {}
	virtual void startInstance( Ref * obj_K ) {}
	virtual void endInstance( Ref * obj_K ) {}
	virtual void atString( Ref * obj_K ) {}
	virtual void atWRecord( Ref * obj_K ) {}
	
public:
	virtual ~GCTracker() {}
};

} // namespace Ginger

#endif
