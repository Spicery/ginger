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

#include <iostream>
#include <sstream>
#include <gmp.h>

#include "mishap.hpp"
#include "rational.hpp"
#include "numbers.hpp"
#include "bigint.hpp"

namespace Ginger {
using namespace std;

/// @todo Can this leak?
RationalExternal::RationalExternal( const std::string & _value ) :
    value( _value )
{
    //cerr << "Make big int external from string" << endl;
    //cerr << "= " << this->value << endl;
}

RationalExternal::RationalExternal( const char * _value ) :
    value( _value )
{
    //cerr << "Make big int external from char *" << endl;
    //cerr << "= " << this->value << endl;
}

RationalExternal::RationalExternal( const mpq_class & _value ) :
    value( _value )
{
    //cerr << "Make big int external from mpz_class ref" << endl;
    //cerr << "= " << this->value << endl;
}

RationalExternal::RationalExternal( const BigIntExternal & _value ) :
    value( _value.value )
{
}

RationalExternal::~RationalExternal() {
    //cerr << "Deleting big int external: " << this->value << endl;
    //cerr << "= " << this->value << endl;
}

BigIntExternal RationalExternal::numerator() const {
    return BigIntExternal( this->value.get_num() );
}

BigIntExternal RationalExternal::denominator() const {
    return BigIntExternal( this->value.get_den() );
}

bool RationalExternal::isIntegral() const {
    return this->value.get_den() == 1;
}

void RationalExternal::print( std::ostream & out ) {
    out << this->value;
}


Ref * RationalExternal::sysApply( Ref * pc, MachineClass * vm ) {
    stringstream str;
    str << this->value;
    throw Mishap( "Trying to apply a number" ).culprit( "Number", str.str() );
    return pc;
}
 
void RationalExternal::addBy( const long n ) {
    this->value += n;
}

void RationalExternal::addBy( const RationalExternal & n ) {
    this->value += n.value;
}

void RationalExternal::addBy( const BigIntExternal & n ) {
    this->value += n.value;
}

void RationalExternal::subBy( const long n ) {
    this->value -= n;
}

void RationalExternal::subBy( const RationalExternal & n ) {
    this->value -= n.value;
}

void RationalExternal::subBy( const BigIntExternal & n ) {
    this->value -= n.value;
}

void RationalExternal::mulBy( const long n ) {
    this->value *= n;
}

void RationalExternal::mulBy( const RationalExternal & n ) {
    this->value *= n.value;
}

RationalExternal RationalExternal::mul( const long n ) {
    return RationalExternal( this->value * n );
}

void RationalExternal::divBy( const long n ) {
    this->value /= n;
}

/**
 * ALERT: this definition needs RationalExternal to be a friend of BigIntExternal.
 */
void RationalExternal::divBy( const BigIntExternal & n ) {
    this->value /= n.value;
}

void RationalExternal::divBy( const RationalExternal & n ) {
    this->value /= n.value;
}

RationalExternal RationalExternal::div( const long n ) {
    return RationalExternal( this->value / n );
}

RationalExternal RationalExternal::add( const long n ) {
    return RationalExternal( this->value + n );
}

RationalExternal RationalExternal::add( const RationalExternal & n ) {
    return RationalExternal( this->value + n.value );
}

RationalExternal RationalExternal::sub( const long n ) {
    return RationalExternal( this->value - n );
}

RationalExternal RationalExternal::subFrom( const long n ) {
    return RationalExternal( n - this->value );
}

RationalExternal RationalExternal::sub( const RationalExternal & n ) {
    return RationalExternal( this->value - n.value );
}

bool RationalExternal::isInSmallRange() const {
    return Numbers::MIN_SMALL <= this->value && this->value <= Numbers::MAX_SMALL;
}

gngdouble_t RationalExternal::toFloat() const {
    return this->value.get_d();
}

RationalExternal RationalExternal::neg() const {
    return RationalExternal( -this->value );
}

bool RationalExternal::lt( const RationalExternal & n ) const {
    return this->value < n.value;
}

bool RationalExternal::eq( const RationalExternal & n ) const {
    return this->value == n.value;
}

bool RationalExternal::isZero() const {
    return this->value == 0;
}

bool RationalExternal::isPositive() const {
    return this->value > 0;
}

bool RationalExternal::isNegative() const {
    return this->value < 0;
}


} // namespace.
