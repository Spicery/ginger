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

#ifndef SHARED_HPP
#define SHARED_HPP

//  The point of this file was to accommodate the fact that shared_ptr
//  was introduced in TR1 but not widely available. As a consequence we
//  had to use the Boost library for a while. In Oct 2012 testing showed
//  that TR1 was available on all our major platforms and so we could
//  continue exclusively with TR1.

//  #define BOOST
#ifdef BOOST
	#include <boost/shared_ptr.hpp>
	#define shared boost::shared_ptr
#else
	#include <memory>
	#define shared std::shared_ptr
#endif


#endif

