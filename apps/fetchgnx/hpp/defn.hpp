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

#ifndef DEFN_HPP
#define DEFN_HPP

enum Mode {
	UNQUALIFIED,
	QUALIFIED,
	ABSOLUTE
};

struct Defn {
	Mode					mode;
	std::string 			pkg;
	std::string				alias;
	std::string				var;

	Defn(
		Mode					mode,
		const std::string &		pkg,
		const std::string &		alias,
		const std::string &		var
	) :
		mode( mode ),
		pkg( pkg ),
		alias( alias ),
		var( var )
	{}
};

#endif
