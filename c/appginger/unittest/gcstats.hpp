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

#ifndef GC_STATS_HPP
#define GC_STATS_HPP

#include "gctracker.hpp"

class GCStats : public GCTracker {
private:
	long copies_made_in_total;
	long copies_made_since_reset;
	long assocs_pruned;
	long assocs_retained;
	
public:
	long changed();	
	void reset();
	
public:	//	Observers.
	void copy1();
	void prune1();
	void retain1();
	long assocsPruned();
	long assocsRetained();
	
public:
	GCStats();
	virtual ~GCStats();
};

#endif
