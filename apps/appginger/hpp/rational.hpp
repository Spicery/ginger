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

#ifndef GINGER_RATIONAL_HPP
#define GINGER_RATIONAL_HPP

#include <memory>
#include <fstream>
#include <string>

#include <gmpxx.h>

#include "external.hpp"
#include "bigint.hpp"


namespace Ginger {

class RationalExternal : public External {
private:
    mpq_class value;

public:
    RationalExternal( const std::string & _value );
    RationalExternal( const char * _value );
    RationalExternal( const mpq_class & _value );
    RationalExternal( const BigIntExternal & _value );
    virtual ~RationalExternal();

public:
    void print( std::ostream & out );
    Ref * sysApply( Ref * pc, MachineClass * vm );

public:
    //  Not shared with big ints (yet).
    void divBy( const long n );
    void divBy( const BigIntExternal & n );
    void divBy( const RationalExternal & n );

    void addBy( const long n );
    void addBy( const BigIntExternal & n );
    void addBy( const RationalExternal & n );

    void subBy( const long n );
    void subBy( const BigIntExternal & n );
    void subBy( const RationalExternal & n );

    RationalExternal div( const long n );
    BigIntExternal numerator() const;
    BigIntExternal denominator() const;
    bool isIntegral() const;

public:
    bool isInSmallRange() const;

    
    void mulBy( const long n );
    void mulBy( const RationalExternal & n );
    
    void flooredQuotientBy( const long n );
    void flooredQuotientBy( const RationalExternal & n );
    void flooredRemainderBy( const long n );
    void flooredRemainderBy( const RationalExternal & n );
    
    RationalExternal mul( const long n );
    
    RationalExternal add( const long n );
    RationalExternal add( const RationalExternal & n );
    
    RationalExternal sub( const long n );
    RationalExternal subFrom( const long n );
    RationalExternal sub( const RationalExternal & n );
    
    gngdouble_t toFloat() const;
    RationalExternal neg() const;
    bool lt( const RationalExternal & n ) const;
    bool eq( const RationalExternal & n ) const;
};

} // namespace Ginger

#endif