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

#ifndef FN_OBJ_CRAWL
#define FN_OBJ_CRAWL

#include "common.hpp"
#include "instruction_set.hpp"
#include "machine.hpp"

/* Find the object pointers in the function object */
class FnObjCrawl {
private:
	Ref * pc;
	Ref * obj_N;
	const InstructionSet & ins;
	const char * types;

private:
	const char * sig;
	const char * name;

public:
	const char *getName() { return this->name; }
	ptrdiff_t getPosn() { return this->types - this->sig; }
	

public:
	Ref * next();
	
public:
	FnObjCrawl( MachineClass * vm, Ref * obj_K );
};

#endif
