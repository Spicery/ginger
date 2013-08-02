#ifndef TRANSDOUBLE_HPP
#define TRANSDOUBLE_HPP

#include <cmath>
#include <ostream>

class TransDouble {
private:
    double data;

public:
    TransDouble( const double d ) : data( d ) {}

    double asDouble() const {
        return this->data;
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
            return this->data < t.data;
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

};

std::ostream& operator<<(std::ostream& os, const TransDouble& obj);

#endif // TRANSDOUBLE_HPP
