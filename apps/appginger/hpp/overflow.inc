namespace SignedOverflow {

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
            std::cout << hex << "(" << a << ", " << b << ")" << dec;
            std::cout << std::endl;
        #endif
        if ( not( b >> SHIFT ) ) {
            return false;   //  Performance optimisation for small numbers.
        } else if ( a > MASK ) { 
            #ifdef DBG_SIGNED_OVERFLOW
                cout << "overflowed! " << a << ">" << MASK << endl;
            #endif
            return true;
        } else {
            gngulong_t bl = (b & MASK) * a;
            gngulong_t bh = (b >> SHIFT) * a + (bl >> SHIFT);
            #ifdef DBG_SIGNED_OVERFLOW
                cout << "bl = " << bl << endl;
                cout << "bh = " << bh << endl;
                if ( ( bh >> SHIFT1 ) != 0 ) {
                    if ( bl != 0 ) {
                        cout << "SIGNED WILL OVERFLOW" << endl;
                    } else {
                        if ( bh != ABS_MIN_NEG_INT ) {
                            cout << "SIGNED WILL OVERFLOW (bh = " << bh << "," << ABS_MIN_NEG_INT << ")" << endl;
                        } else {
                            cout << "Max negative" << endl;
                        }               
                    }
                }
            #endif
            return ( ( bh >> SHIFT1 ) != 0 ) && not( neg_result && ( bl == 0 ) && ( bh == ABS_MIN_NEG_INT ) );
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