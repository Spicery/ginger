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

#ifndef GNG_GENERATOR_HPP
#define GNG_GENERATOR_HPP

namespace Ginger {

template < class T > 
class Generator {
private:
	typename T::iterator start;
	typename T::iterator finish;
public:
	Generator( T & v ) :
		start( v.begin() ),
		finish( v.end() )
	{}
public:
	typename T::value_type & operator *() { return *this->start; }
	bool operator !() { return this->start == this->finish; }
	Generator & operator ++() { ++this->start; return *this; }
};

} // namespace Ginger

#endif
