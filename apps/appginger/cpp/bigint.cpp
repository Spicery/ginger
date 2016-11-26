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

//  Until GMP is updated, we must include the cstddef header in the CPP file
//  as the first include. See https://gcc.gnu.org/gcc-4.9/porting_to.html
/*  Header <cstddef> changes

The <cstddef> header was updated for C++11 support and this breaks some libraries which misuse macros meant for internal use by GCC only. For instance with GMP versions up to 5.1.3, you may see:

/usr/include/c++/4.9.0/cstddef:51:11: error: ‘::max_align_t’ has not been declared
   using ::max_align_t;
*/
#include <cstddef>

#include <iostream>
#include <sstream>
#include <gmp.h>

#include "mishap.hpp"
#include "bigint.hpp"
#include "numbers.hpp"

namespace Ginger {
using namespace std;

/// @todo Can this leak?
BigIntExternal::BigIntExternal( const std::string & _value ) :
    value( _value )
{
    //cerr << "Make big int external from string" << endl;
    //cerr << "= " << this->value << endl;
}

BigIntExternal::BigIntExternal( const char * _value ) :
    value( _value )
{
    //cerr << "Make big int external from char *" << endl;
    //cerr << "= " << this->value << endl;
}

BigIntExternal::BigIntExternal( const mpz_class & _value ) :
    value( _value )
{
    //cerr << "Make big int external from mpz_class ref" << endl;
    //cerr << "= " << this->value << endl;
}

BigIntExternal::~BigIntExternal() {
    //cerr << "Deleting big int external: " << this->value << endl;
    //cerr << "= " << this->value << endl;
}


void BigIntExternal::print( std::ostream & out ) {
    out << this->value;
}


Ref * BigIntExternal::sysApply( Ref * pc, MachineClass * vm ) {
    stringstream str;
    str << this->value;
    throw Mishap( "Trying to apply a number" ).culprit( "Number", str.str() );
    return pc;
}
 
void BigIntExternal::addBy( const long n ) {
    this->value += n;
}

void BigIntExternal::subBy( const long n ) {
    this->value -= n;
}

void BigIntExternal::mulBy( const long n ) {
    this->value *= n;
}

void BigIntExternal::mulBy( const BigIntExternal & n ) {
    this->value *= n.value;
}

void BigIntExternal::flooredQuotientBy( const long _d ) {
    mpz_t d;
    mpz_init_set_si( d, _d );
    mpz_t q;
    mpz_init( q );
    mpz_fdiv_q( q, this->value.get_mpz_t(), d );
    this->value = static_cast< mpz_class >( q );
}

void BigIntExternal::truncatedQuotientBy( const long _d ) {
    mpz_t d;
    mpz_init_set_si( d, _d );
    mpz_t q;
    mpz_init( q );
    mpz_tdiv_q( q, this->value.get_mpz_t(), d );
    this->value = static_cast< mpz_class >( q );
}

void BigIntExternal::flooredQuotientBy( const BigIntExternal & d ) {
    mpz_t q;
    mpz_init( q );
    mpz_fdiv_q( q, this->value.get_mpz_t(), d.value.get_mpz_t() );
    this->value = static_cast< mpz_class >( q );
}

void BigIntExternal::truncatedQuotientBy( const BigIntExternal & d ) {
    mpz_t q;
    mpz_init( q );
    mpz_tdiv_q( q, this->value.get_mpz_t(), d.value.get_mpz_t() );
    this->value = static_cast< mpz_class >( q );
}

void BigIntExternal::flooredRemainderBy( const long _d ) {
    mpz_t d;
    mpz_init_set_si( d, _d );
    mpz_t q;
    mpz_init( q );
    mpz_fdiv_r( q, this->value.get_mpz_t(), d );
    this->value = static_cast< mpz_class >( q );
}

void BigIntExternal::truncatedRemainderBy( const long _d ) {
    mpz_t d;
    mpz_init_set_si( d, _d );
    mpz_t q;
    mpz_init( q );
    mpz_tdiv_r( q, this->value.get_mpz_t(), d );
    this->value = static_cast< mpz_class >( q );
}

void BigIntExternal::flooredRemainderBy( const BigIntExternal & d ) {
    mpz_t q;
    mpz_init( q );
    mpz_fdiv_r( q, this->value.get_mpz_t(), d.value.get_mpz_t() );
    this->value = static_cast< mpz_class >( q );
}

void BigIntExternal::truncatedRemainderBy( const BigIntExternal & d ) {
    mpz_t q;
    mpz_init( q );
    mpz_tdiv_r( q, this->value.get_mpz_t(), d.value.get_mpz_t() );
    this->value = static_cast< mpz_class >( q );
}

/* 
    DISABLED in favour of MOD =^= floored remainder.
void BigIntExternal::remBy( const long _d ) {
    mpz_t d;
    mpz_init_set_si( d, _d );
    mpz_t q;
    mpz_init( q );
    mpz_fdiv_r( q, this->value.get_mpz_t(), d );
    this->value = static_cast< mpz_class >( q );
}

void BigIntExternal::remBy( const BigIntExternal & d ) {
    mpz_t q;
    mpz_init( q );
    mpz_fdiv_r( q, this->value.get_mpz_t(), d.value.get_mpz_t() );
    this->value = static_cast< mpz_class >( q );
}
*/

BigIntExternal BigIntExternal::mul( const long n ) {
    return BigIntExternal( this->value * n );
}

BigIntExternal BigIntExternal::add( const long n ) {
    return BigIntExternal( this->value + n );
}

BigIntExternal BigIntExternal::add( const BigIntExternal & n ) {
    return BigIntExternal( this->value + n.value );
}

BigIntExternal BigIntExternal::sub( const long n ) {
    return BigIntExternal( this->value - n );
}

BigIntExternal BigIntExternal::subFrom( const long n ) {
    return BigIntExternal( n - this->value );
}

BigIntExternal BigIntExternal::sub( const BigIntExternal & n ) {
    return BigIntExternal( this->value - n.value );
}

bool BigIntExternal::isInSmallRange() const {
    //cerr << "Numbers::MIN_SMALL = " << Numbers::MIN_SMALL << endl;
    //cerr << "Numbers::MAX_SMALL = " << Numbers::MAX_SMALL << endl;
    return Numbers::MIN_SMALL <= this->value && this->value <= Numbers::MAX_SMALL;
}

Ref BigIntExternal::toSmall() const {
    return LongToSmall( this->value.get_si() );
}

gngdouble_t BigIntExternal::toFloat() const {
    return this->value.get_d();
}

BigIntExternal BigIntExternal::neg() const {
    return BigIntExternal( -this->value );
}

BigIntExternal BigIntExternal::abs() const {
    return BigIntExternal( this->value >= 0 ? this->value : -this->value );
}

bool BigIntExternal::lt( const BigIntExternal & n ) const {
    return this->value < n.value;
}

bool BigIntExternal::eq( const BigIntExternal & n ) const {
    return this->value == n.value;
}

bool BigIntExternal::isZero() const {
    return this->value == 0;
}

bool BigIntExternal::isPositive() const {
    return this->value > 0;
}

bool BigIntExternal::isNegative() const {
    return this->value < 0;
}


} // namespace.
