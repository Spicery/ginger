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

#ifndef GNG_WELL_KNOWN_PATHS_HPP
#define GNG_WELL_KNOWN_PATHS_HPP

namespace Ginger {

#define LICENSE_FILE	INSTALL_LIB "/COPYING"

#define FETCHGNX        INSTALL_TOOL "/fetchgnx"
#define SIMPLIFYGNX		INSTALL_TOOL "/simplifygnx"

#define GNGREADLINE     INSTALL_TOOL "/gngreadline"
#define FILE2GNX        INSTALL_TOOL "/file2gnx"
#define COMMON2GNX      INSTALL_TOOL "/common2gnx"
#define LISP2GNX		INSTALL_TOOL "/lisp2gnx"
#define GSON2GNX		INSTALL_TOOL "/gson2gnx"
#define CSTYLE2GNX      INSTALL_TOOL "/cstyle2gnx"
#define GNX2GNX         "/bin/cat" 

#define TAIL			"/usr/bin/tail"

} // namespace Ginger 

#endif