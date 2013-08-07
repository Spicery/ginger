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

};

std::ostream& operator<<(std::ostream& os, const TransDouble& obj);
std::istream& operator>>(std::istream& os, TransDouble& obj);

#endif // TRANSDOUBLE_HPP
