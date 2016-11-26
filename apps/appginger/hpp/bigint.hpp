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

#ifndef BIGINTS_HPP
#define BIGINTS_HPP

#include <memory>
#include <fstream>
#include <string>

#include <gmpxx.h>

#include "external.hpp"


namespace Ginger {

class BigIntExternal : public External {
friend class RationalExternal;
private:
    mpz_class value;

public:
    BigIntExternal( const std::string & _value );
    BigIntExternal( const char * _value );
    BigIntExternal( const mpz_class & _value );
    virtual ~BigIntExternal();

public:
    void print( std::ostream & out );
    Ref * sysApply( Ref * pc, MachineClass * vm );

public:
    //  Specific to big ints.
    void flooredQuotientBy( const long n );
    void flooredQuotientBy( const BigIntExternal & n );
    void flooredRemainderBy( const long n );
    void flooredRemainderBy( const BigIntExternal & n );
    void truncatedQuotientBy( const long n );
    void truncatedQuotientBy( const BigIntExternal & n );
    void truncatedRemainderBy( const long n );
    void truncatedRemainderBy( const BigIntExternal & n );
    Ref toSmall() const;

public:
    bool isInSmallRange() const;
    void addBy( const long n );
    void subBy( const long n );
    void mulBy( const long n );
    void mulBy( const BigIntExternal & n );
    BigIntExternal mul( const long n );
    BigIntExternal add( const long n );
    BigIntExternal add( const BigIntExternal & n );
    BigIntExternal sub( const long n );
    BigIntExternal subFrom( const long n );
    BigIntExternal sub( const BigIntExternal & n );
    gngdouble_t toFloat() const;
    BigIntExternal neg() const;
    bool lt( const BigIntExternal & n ) const;
    bool eq( const BigIntExternal & n ) const;
    bool isZero() const;
    bool isPositive() const;
    bool isNegative() const;
    BigIntExternal abs() const;
};

} // namespace Ginger

#endif
