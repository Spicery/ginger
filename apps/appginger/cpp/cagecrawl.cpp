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

#include "cagecrawl.hpp"

#include "mishap.hpp"
#include "key.hpp"
#include "misclayout.hpp"

namespace Ginger {

Ref * CageCrawl::next() {
	if ( this->current < this->cage->top ) {
		Ref * key = findObjectKey( this->current );
		unsigned long len = lengthAfterObjectKey( key );
		this->current = key + len + 1;
		return key;
	} else {
		return 0;
	}
}

} // namespace Ginger
