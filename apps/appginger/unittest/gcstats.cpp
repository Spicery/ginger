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

#include "gcstats.hpp"

long GCStats::changed() {
	return this->copies_made_since_reset;
}

void GCStats::reset() {
	this->copies_made_since_reset = 0;
}

void GCStats::copy1() {
	this->copies_made_in_total += 1;
	this->copies_made_since_reset += 1;
}

void GCStats::prune1() {
	this->assocs_pruned += 1;
}

void GCStats::retain1() {
	this->assocs_retained += 1;
}

long GCStats::assocsPruned() {
	return this->assocs_pruned;
}

long GCStats::assocsRetained() {
	return this->assocs_retained;
}

GCStats::GCStats() :
	copies_made_in_total( 0 ),
	copies_made_since_reset( 0 ),
	assocs_pruned( 0 )
{
}

GCStats::~GCStats() {}
