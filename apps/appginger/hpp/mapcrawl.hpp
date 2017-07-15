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

#ifndef MAP_CRAWL_HPP
#define MAP_CRAWL_HPP

#include "common.hpp"

namespace Ginger {

class MapCrawl {
private:
    long size_of_data;
    int index_of_data;
    Ref * data;
    Ref bucket;

public:
    MapCrawl( Ref * map_K );

public:
    Ref * nextBucket();
    bool hasBeenCalled() const;

};

} // namespace Ginger

#endif
