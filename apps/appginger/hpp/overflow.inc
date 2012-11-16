namespace SignedOverflow {
#define DBG_SIGNED_OVERFLOW



    typedef unsigned long gngulong_t;
    typedef long gnglong_t;

    const static size_t SHIFT = ( sizeof( gngulong_t ) * 4 );
    const static size_t SHIFT1 = SHIFT - 1;
    const static size_t ABS_MIN_NEG_INT = ( 1UL << SHIFT1 );
    const static size_t MASK = ( ( 1UL << SHIFT ) - 1 );

    //  Precondition: a <= b.
    inline static int moCheckLoHiUInt64( gngulong_t a, gngulong_t b, bool neg_result ) {
        #ifdef DBG_SIGNED_OVERFLOW
            std::cout << "moCheckLoHiUInt64: ";
            std::cout << a << ", " << b;
            std::cout << std::hex << "(" << a << ", " << b << ")" << std::dec;
            std::cout << std::endl;
        #endif
        if ( not( b >> SHIFT ) ) {
            return false;   //  Performance optimisation for small numbers.
        } else if ( a > MASK ) { 
            #ifdef DBG_SIGNED_OVERFLOW
                std::cout << "overflowed! " << a << ">" << MASK << std::endl;
            #endif
            return true;
        } else {
            gngulong_t bl = (b & MASK) * a;
            gngulong_t bh = (b >> SHIFT) * a + (bl >> SHIFT);
            const bool overflow = ( ( bh >> SHIFT1 ) != 0 ) && not( neg_result && ( bl == 0 ) && ( bh == ABS_MIN_NEG_INT ) );
            #ifdef DBG_SIGNED_OVERFLOW
                std::cout << "bl = " << bl << std::endl;
                std::cout << "bh = " << bh << std::endl;
                std::cout << "overflow = " << overflow << std::endl;
                if ( ( bh >> SHIFT1 ) != 0 ) {
                    if ( bl != 0 ) {
                        std::cout << "SIGNED WILL OVERFLOW" << std::endl;
                    } else {
                        if ( bh != ABS_MIN_NEG_INT ) {
                            std::cout << "SIGNED WILL OVERFLOW (bh = " << bh << "," << ABS_MIN_NEG_INT << ")" << std::endl;
                        } else {
                            std::cout << "Max negative" << std::endl;
                        }               
                    }
                }
            #endif
            return overflow;
        }
    }

    inline static bool moCheckUInt64( gngulong_t a, gngulong_t b, bool neg_result ) {
        if ( a > b ) {
            return moCheckLoHiUInt64( b, a, neg_result );
        } else {
            return moCheckLoHiUInt64( a, b, neg_result );
        }
    }

    //  Signed integer multiply overflopw check.
    inline static bool mulOverflowCheck( gnglong_t a, gnglong_t b ) {
        if ( a > 0 ) {
            if ( b > 0 ) {
                return moCheckUInt64( a, b, false );
            } else if ( b < 0 ) {
                return moCheckUInt64( a, -b, true );
            } else {
                return false;
            }
        } else if ( a < 0 ) {
            if ( b > 0 ) {
                return moCheckUInt64( -a, b, true );
            } else if ( b < 0 ) {
                return moCheckUInt64( -a, -b, false );
            } else {
                return false;
            }
        } else {
            return false;
        }
    }
}