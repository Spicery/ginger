#ifndef TRANSDOUBLE_HPP
#define TRANSDOUBLE_HPP

#include <cmath>
#include <ostream>
#include <istream>

class TransDouble {
private:
    double data;

public:
    TransDouble() : data( 0.0 ) {}
    TransDouble( const double d ) : data( d ) {}

    double asDouble() const {
        return this->data;
    }

    TransDouble & operator =( const double & d ) {
        this->data = d;
        return *this;
    }

    TransDouble operator +( const TransDouble& t ) const {
        return TransDouble( this->data + t.data );
    }

    TransDouble & operator +=( const TransDouble& t ) {
        this->data += t.data;
        return *this;
    }

    TransDouble operator *( const TransDouble& t ) const {
        return TransDouble( this->data * t.data );
    }

    TransDouble & operator *=( const TransDouble& t ) {
        this->data *= t.data;
        return *this;
    }

    TransDouble operator -() const {
        return TransDouble( -this->data );
    }

    TransDouble operator -( const TransDouble& t ) const {
        return TransDouble( this->data - t.data );
    }

    TransDouble & operator -=( const TransDouble& t ) {
        this->data -= t.data;
        return *this;
    }

    TransDouble operator /( const TransDouble& t ) const {
        return TransDouble( this->data / ( t.data == 0.0 ? 0.0 : t.data ) );
    }

    TransDouble & operator /=( const TransDouble& t ) {
        this->data = this->data / ( t.data == 0.0 ? 0.0 : t.data );
        return *this;
    }

    bool operator ==( const TransDouble& t ) const {
        if ( std::isnan( this->data ) ) {
            return std::isnan( t.data );
        } else {
            return this->data == t.data;
        }
    }

    bool operator !=( const TransDouble& t ) const {
        if ( std::isnan( this->data ) ) {
            return not std::isnan( t.data );
        } else {
            return this->data != t.data;
        }
    }

    bool operator <( const TransDouble& t ) const {
        return this->data < t.data;
    }

    bool operator <=( const TransDouble& t ) const {
        if ( std::isnan( this->data ) ) {
            return std::isnan( t.data );
        } else {
            return this->data <= t.data;
        }
    }

    bool operator >( const TransDouble& t ) const {
        return this->data > t.data;
    }

    bool operator >= ( const TransDouble& t ) const {
        if ( std::isnan( this->data ) ) {
            return std::isnan( t.data );
        } else {
            return this->data >= t.data;
        }
    }

    bool isLtGt( const TransDouble& t ) const {
        return this->data < t.data || this->data > t.data;
    }

    bool isLtEGt( const TransDouble& t ) const {
        return (
            this->data < t.data || 
            this->data > t.data || 
            this->data == t.data ||
            ( std::isnan( this->data ) && std::isnan( t.data ) )
        );
    }

    TransDouble max( const TransDouble& t ) const {
        if ( std::isnan( this->data ) ) {
            return t;
        } else {
            return this->data < t.data ? t.data : this->data;
        }
    }

    TransDouble min( const TransDouble& t ) const {
        if ( std::isnan( this->data ) ) {
            return t;
        } else {
            return this->data > t.data ? t.data : this->data;
        }
    }

    bool isThisMax( const TransDouble& t ) const {
        if ( std::isnan( this->data ) ) {
            return false;
        } else {
            return not( this->data < t.data );
        }
    }

    bool isThisMin( const TransDouble& t ) const {
        if ( std::isnan( this->data ) ) {
            return false;
        } else {
            return not( this->data > t.data );
        }
    }

    bool isFinite() const {
        return std::isfinite( this->data );
    }

    bool isInfinite() const {
        return std::isinf( this->data );
    }

    bool isNullity() const {
        return std::isnan( this->data );
    }

    bool isPositiveInfinity() const {
        return this->data > 0 && std::isinf( this->data );
    }

    bool isNegativeInfinity() const {
        return this->data < 0 && std::isinf( this->data );
    }

    char quadrachotomy( const char * lzgn = "LZGN" ) const {
        return(
            this->data > 0.0 ? lzgn[ 2 ] :
            this->data < 0.0 ? lzgn[ 0 ] :
            this->data == 0.0 ? lzgn[ 1 ] :
            lzgn[ 3 ]
        );
    }

    TransDouble sin() const {
        return TransDouble( std::sin( this->data ) );
    }

    TransDouble cos() const {
        return TransDouble( std::cos( this->data ) );
    }

    TransDouble tan() const {
        return TransDouble( std::tan( this->data ) );
    }

    TransDouble exp() const {
        return TransDouble( std::exp( this->data ) );
    }

    TransDouble exp2() const {
        return TransDouble( std::exp2( this->data ) );
    }

    TransDouble log() const {
        return TransDouble( std::log( this->data ) );
    }

    TransDouble log2() const {
        return TransDouble( std::log2( this->data ) );
    }

    TransDouble log10() const {
        return TransDouble( std::log10( this->data ) );
    }

    TransDouble sqrt() const {
        return TransDouble( std::sqrt( this->data ) );
    }

    TransDouble cbrt() const {
        return TransDouble( std::cbrt( this->data ) );
    }

    TransDouble hypot( const TransDouble& t ) const {
        return std::hypot( this->data, t.data );
    }

};

std::ostream& operator<<(std::ostream& os, const TransDouble& obj);
std::istream& operator>>(std::istream& os, TransDouble& obj);

#endif // TRANSDOUBLE_HPP
