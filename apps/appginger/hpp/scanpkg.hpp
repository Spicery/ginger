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

#ifndef SCAN_PKG_HPP
#define SCAN_PKG_HPP

#include "package.hpp"
#include <string>

class ScanPkg {
private:
	std::string var;	//	debug
public:
	std::string variable() { return this->var; }	//	debug
	

private:
	Package * pkg;
	std::map< std::string, Ident >::iterator it;

public:
	Ref * next();
	
public:
	ScanPkg( Package * d );
};

#endif
