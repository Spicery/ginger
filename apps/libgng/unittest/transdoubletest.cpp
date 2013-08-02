#include "transdouble.hpp"
#include "CppUnitLite2.h"


/**
 *  In this particular version of CppUnitLite, fixtures work by
 *  being subclassed for each test, the test-body becoming a method
 *  of the subclass (the test-method), and then the Test object instantiating 
 *  the fixture and handing off control to the test-method.
 *
 *  The overall effect is that the test-body has access to the protected
 *  and public slots and methods of the fixture class via "this->XXX"
 */

class TransDoubleFixture {

public:
    TransDouble zero;
    TransDouble one;
    TransDouble n_one;
    TransDouble p_inf;
    TransDouble n_inf;
    TransDouble nullity;
    
public:
    TransDoubleFixture() :
        zero( 0.0 ),
        one( 1.0 ),
        n_one( -1.0 ),
        p_inf( 1.0 / 0.0 ),
        n_inf( -1.0 / 0.0 ),
        nullity( 0.0 / 0.0 )
    {
    }

};

TEST_F( TransDoubleFixture, MinusZeroInDivision__TransDouble )
{
    TransDouble nzero = zero / n_one;
    TransDouble what_is_this = one / nzero;

    CHECK_EQUAL( p_inf, what_is_this );
    CHECK_EQUAL( false, ( p_inf != what_is_this ) );

    double dzero = 0.0;
    double done = 1.0;
    double dnzero = dzero / -done;
    double dpinf = done / dzero;
    double dwhat_is_this = done / dnzero;

    CHECK_EQUAL( true, dpinf != dwhat_is_this );
}

TEST_F( TransDoubleFixture, Add__TransDouble )
{
    CHECK_EQUAL( this->n_inf,   this->n_inf + this->n_inf );
    CHECK_EQUAL( this->n_inf,   this->n_inf + this->n_one );
    CHECK_EQUAL( this->n_inf,   this->n_inf + this->zero );
    CHECK_EQUAL( this->n_inf,   this->n_inf + this->one );
    CHECK_EQUAL( this->nullity, this->n_inf + this->p_inf );
    CHECK_EQUAL( this->nullity, this->n_inf + this->nullity );

    TransDouble n_two( -2 );
    CHECK_EQUAL( this->n_inf,   this->n_one + this->n_inf );
    CHECK_EQUAL( n_two,         this->n_one + this->n_one );
    CHECK_EQUAL( this->n_one,   this->n_one + this->zero );
    CHECK_EQUAL( this->zero,    this->n_one + this->one );
    CHECK_EQUAL( this->p_inf,   this->n_one + this->p_inf );
    CHECK_EQUAL( this->nullity, this->n_one + this->nullity );

    CHECK_EQUAL( this->n_inf,   this->zero + this->n_inf );
    CHECK_EQUAL( this->n_one,   this->zero + this->n_one );
    CHECK_EQUAL( this->zero,    this->zero + this->zero );
    CHECK_EQUAL( this->one,     this->zero + this->one );
    CHECK_EQUAL( this->p_inf,   this->zero + this->p_inf );
    CHECK_EQUAL( this->nullity, this->zero + this->nullity );

    TransDouble p_two( 2 );
    CHECK_EQUAL( this->n_inf,   this->one + this->n_inf );
    CHECK_EQUAL( this->zero,    this->one + this->n_one );
    CHECK_EQUAL( this->one,     this->one + this->zero );
    CHECK_EQUAL( p_two,         this->one + this->one );
    CHECK_EQUAL( this->p_inf,   this->one + this->p_inf );
    CHECK_EQUAL( this->nullity, this->one + this->nullity );

    CHECK_EQUAL( this->nullity, this->p_inf + this->n_inf );
    CHECK_EQUAL( this->p_inf,   this->p_inf + this->n_one );
    CHECK_EQUAL( this->p_inf,   this->p_inf + this->zero );
    CHECK_EQUAL( this->p_inf,   this->p_inf + this->one );
    CHECK_EQUAL( this->p_inf,   this->p_inf + this->p_inf );
    CHECK_EQUAL( this->nullity, this->p_inf + this->nullity );

    CHECK_EQUAL( this->nullity, this->nullity + this->n_inf );
    CHECK_EQUAL( this->nullity, this->nullity + this->n_one );
    CHECK_EQUAL( this->nullity, this->nullity + this->zero );
    CHECK_EQUAL( this->nullity, this->nullity + this->one );
    CHECK_EQUAL( this->nullity, this->nullity + this->p_inf );
    CHECK_EQUAL( this->nullity, this->nullity + this->nullity );
}

TEST_F( TransDoubleFixture, Subtract__TransDouble )
{
    CHECK_EQUAL( this->nullity,   this->n_inf - this->n_inf );
    CHECK_EQUAL( this->n_inf,   this->n_inf - this->n_one );
    CHECK_EQUAL( this->n_inf,   this->n_inf - this->zero );
    CHECK_EQUAL( this->n_inf,   this->n_inf - this->one );
    CHECK_EQUAL( this->n_inf,   this->n_inf - this->p_inf );
    CHECK_EQUAL( this->nullity, this->n_inf - this->nullity );

    TransDouble n_two( -2 );
    CHECK_EQUAL( this->p_inf,   this->n_one - this->n_inf );
    CHECK_EQUAL( this->zero,    this->n_one - this->n_one );
    CHECK_EQUAL( this->n_one,   this->n_one - this->zero );
    CHECK_EQUAL( n_two,         this->n_one - this->one );
    CHECK_EQUAL( this->n_inf,   this->n_one - this->p_inf );
    CHECK_EQUAL( this->nullity, this->n_one - this->nullity );

    CHECK_EQUAL( this->p_inf,   this->zero - this->n_inf );
    CHECK_EQUAL( this->one,     this->zero - this->n_one );
    CHECK_EQUAL( this->zero,    this->zero - this->zero );
    CHECK_EQUAL( this->n_one,   this->zero - this->one );
    CHECK_EQUAL( this->n_inf,   this->zero - this->p_inf );
    CHECK_EQUAL( this->nullity, this->zero - this->nullity );

    TransDouble p_two( 2 );
    CHECK_EQUAL( this->p_inf,   this->one - this->n_inf );
    CHECK_EQUAL( p_two,         this->one - this->n_one );
    CHECK_EQUAL( this->one,     this->one - this->zero );
    CHECK_EQUAL( this->zero,    this->one - this->one );
    CHECK_EQUAL( this->n_inf,   this->one - this->p_inf );
    CHECK_EQUAL( this->nullity, this->one - this->nullity );

    CHECK_EQUAL( this->p_inf,   this->p_inf - this->n_inf );
    CHECK_EQUAL( this->p_inf,   this->p_inf - this->n_one );
    CHECK_EQUAL( this->p_inf,   this->p_inf - this->zero );
    CHECK_EQUAL( this->p_inf,   this->p_inf - this->one );
    CHECK_EQUAL( this->nullity, this->p_inf - this->p_inf );
    CHECK_EQUAL( this->nullity, this->p_inf - this->nullity );

    CHECK_EQUAL( this->nullity, this->nullity - this->n_inf );
    CHECK_EQUAL( this->nullity, this->nullity - this->n_one );
    CHECK_EQUAL( this->nullity, this->nullity - this->zero );
    CHECK_EQUAL( this->nullity, this->nullity - this->one );
    CHECK_EQUAL( this->nullity, this->nullity - this->p_inf );
    CHECK_EQUAL( this->nullity, this->nullity - this->nullity );
}



TEST_F( TransDoubleFixture, Multiply__TransDouble )
{
    CHECK_EQUAL( this->p_inf,   this->n_inf * this->n_inf );
    CHECK_EQUAL( this->p_inf,   this->n_inf * this->n_one );
    CHECK_EQUAL( this->nullity, this->n_inf * this->zero );
    CHECK_EQUAL( this->n_inf,   this->n_inf * this->one );
    CHECK_EQUAL( this->n_inf,   this->n_inf * this->p_inf );
    CHECK_EQUAL( this->nullity, this->n_inf * this->nullity );

    CHECK_EQUAL( this->p_inf,   this->n_one * this->n_inf );
    CHECK_EQUAL( this->one,     this->n_one * this->n_one );
    CHECK_EQUAL( this->zero,    this->n_one * this->zero );
    CHECK_EQUAL( this->n_one,   this->n_one * this->one );
    CHECK_EQUAL( this->n_inf,   this->n_one * this->p_inf );
    CHECK_EQUAL( this->nullity, this->n_one * this->nullity );

    CHECK_EQUAL( this->nullity, this->zero * this->n_inf );
    CHECK_EQUAL( this->zero,    this->zero * this->n_one );
    CHECK_EQUAL( this->zero,    this->zero * this->zero );
    CHECK_EQUAL( this->zero,    this->zero * this->one );
    CHECK_EQUAL( this->nullity, this->zero * this->p_inf );
    CHECK_EQUAL( this->nullity, this->zero * this->nullity );

    CHECK_EQUAL( this->n_inf,   this->one * this->n_inf );
    CHECK_EQUAL( this->n_one,   this->one * this->n_one );
    CHECK_EQUAL( this->zero,    this->one * this->zero );
    CHECK_EQUAL( this->one,     this->one * this->one );
    CHECK_EQUAL( this->p_inf,   this->one * this->p_inf );
    CHECK_EQUAL( this->nullity, this->one * this->nullity );

    CHECK_EQUAL( this->n_inf,   this->p_inf * this->n_inf );
    CHECK_EQUAL( this->n_inf,   this->p_inf * this->n_one );
    CHECK_EQUAL( this->nullity, this->p_inf * this->zero );
    CHECK_EQUAL( this->p_inf,   this->p_inf * this->one );
    CHECK_EQUAL( this->p_inf,   this->p_inf * this->p_inf );
    CHECK_EQUAL( this->nullity, this->p_inf * this->nullity );

    CHECK_EQUAL( this->nullity, this->nullity * this->n_inf );
    CHECK_EQUAL( this->nullity, this->nullity * this->n_one );
    CHECK_EQUAL( this->nullity, this->nullity * this->zero );
    CHECK_EQUAL( this->nullity, this->nullity * this->one );
    CHECK_EQUAL( this->nullity, this->nullity * this->p_inf );
    CHECK_EQUAL( this->nullity, this->nullity * this->nullity );
}



TEST_F( TransDoubleFixture, Divide__TransDouble )
{
    CHECK_EQUAL( this->nullity, this->n_inf / this->n_inf );
    CHECK_EQUAL( this->p_inf,   this->n_inf / this->n_one );
    CHECK_EQUAL( this->n_inf,   this->n_inf / this->zero );
    CHECK_EQUAL( this->n_inf,   this->n_inf / this->one );
    CHECK_EQUAL( this->nullity, this->n_inf / this->p_inf );
    CHECK_EQUAL( this->nullity, this->n_inf / this->nullity );

    CHECK_EQUAL( this->zero,    this->n_one / this->n_inf );
    CHECK_EQUAL( this->one,     this->n_one / this->n_one );
    CHECK_EQUAL( this->n_inf,   this->n_one / this->zero );
    CHECK_EQUAL( this->n_one,   this->n_one / this->one );
    CHECK_EQUAL( this->zero,    this->n_one / this->p_inf );
    CHECK_EQUAL( this->nullity, this->n_one / this->nullity );

    CHECK_EQUAL( this->zero,    this->zero / this->n_inf );
    CHECK_EQUAL( this->zero,    this->zero / this->n_one );
    CHECK_EQUAL( this->nullity, this->zero / this->zero );
    CHECK_EQUAL( this->zero,    this->zero / this->one );
    CHECK_EQUAL( this->zero,    this->zero / this->p_inf );
    CHECK_EQUAL( this->nullity, this->zero / this->nullity );

    CHECK_EQUAL( this->zero,    this->one / this->n_inf );
    CHECK_EQUAL( this->n_one,   this->one / this->n_one );
    CHECK_EQUAL( this->p_inf,   this->one / this->zero );
    CHECK_EQUAL( this->one,     this->one / this->one );
    CHECK_EQUAL( this->zero,    this->one / this->p_inf );
    CHECK_EQUAL( this->nullity, this->one / this->nullity );

    CHECK_EQUAL( this->nullity, this->p_inf / this->n_inf );
    CHECK_EQUAL( this->n_inf,   this->p_inf / this->n_one );
    CHECK_EQUAL( this->p_inf,   this->p_inf / this->zero );
    CHECK_EQUAL( this->p_inf,   this->p_inf / this->one );
    CHECK_EQUAL( this->nullity, this->p_inf / this->p_inf );
    CHECK_EQUAL( this->nullity, this->p_inf / this->nullity );

    CHECK_EQUAL( this->nullity, this->nullity / this->n_inf );
    CHECK_EQUAL( this->nullity, this->nullity / this->n_one );
    CHECK_EQUAL( this->nullity, this->nullity / this->zero );
    CHECK_EQUAL( this->nullity, this->nullity / this->one );
    CHECK_EQUAL( this->nullity, this->nullity / this->p_inf );
    CHECK_EQUAL( this->nullity, this->nullity / this->nullity );
}

TEST_F( TransDoubleFixture, LessThan__TransDouble )
{
    CHECK( not( this->n_inf < this->n_inf ) );
    CHECK( this->n_inf < this->n_one );
    CHECK( this->n_inf < this->zero );
    CHECK( this->n_inf < this->one );
    CHECK( this->n_inf < this->p_inf );
    CHECK( not( this->n_inf < this->nullity ) );

    CHECK( not( this->n_one < this->n_inf ) );
    CHECK( not( this->n_one < this->n_one ) );
    CHECK( this->n_one < this->zero );
    CHECK( this->n_one < this->one );
    CHECK( this->n_one < this->p_inf );
    CHECK( not( this->n_one < this->nullity ) );

    CHECK( not( this->zero < this->n_inf ) );
    CHECK( not( this->zero < this->n_one ) );
    CHECK( not( this->zero < this->zero ) );
    CHECK( this->zero < this->one );
    CHECK( this->zero < this->p_inf );
    CHECK( not( this->zero < this->nullity ) );

    CHECK( not( this->one < this->n_inf ) );
    CHECK( not( this->one < this->n_one ) );
    CHECK( not( this->one < this->zero ) );
    CHECK( not( this->one < this->one ) );
    CHECK( this->one < this->p_inf );
    CHECK( not( this->one < this->nullity ) );

    CHECK( not( this->p_inf < this->n_inf ) );
    CHECK( not( this->p_inf < this->n_one ) );
    CHECK( not( this->p_inf < this->zero ) );
    CHECK( not( this->p_inf < this->one ) );
    CHECK( not( this->p_inf < this->p_inf ) );
    CHECK( not( this->p_inf < this->nullity ) );

    CHECK( not( this->nullity < this->n_inf ) );
    CHECK( not( this->nullity < this->n_one ) );
    CHECK( not( this->nullity < this->zero ) );
    CHECK( not( this->nullity < this->one ) );
    CHECK( not( this->nullity < this->p_inf ) );
    CHECK( not( this->nullity < this->nullity ) );

}

TEST_F( TransDoubleFixture, GreaterThan__TransDouble )
{
    CHECK( not( this->n_inf > this->n_inf ) );
    CHECK( not( this->n_inf > this->n_one ) );
    CHECK( not( this->n_inf > this->zero ) );
    CHECK( not( this->n_inf > this->one ) );
    CHECK( not( this->n_inf > this->p_inf ) );
    CHECK( not( this->n_inf > this->nullity ) );

    CHECK( this->n_one > this->n_inf );
    CHECK( not( this->n_one > this->n_one ) );
    CHECK( not( this->n_one > this->zero ) );
    CHECK( not( this->n_one > this->one ) );
    CHECK( not( this->n_one > this->p_inf ) );
    CHECK( not( this->n_one > this->nullity ) );

    CHECK( this->zero > this->n_inf );
    CHECK( this->zero > this->n_one );
    CHECK( not( this->zero > this->zero ) );
    CHECK( not( this->zero > this->one ) );
    CHECK( not( this->zero > this->p_inf ) );
    CHECK( not( this->zero > this->nullity ) );

    CHECK( ( this->one > this->n_inf ) );
    CHECK( ( this->one > this->n_one ) );
    CHECK( ( this->one > this->zero ) );
    CHECK( not( this->one > this->one ) );
    CHECK( not( this->one > this->p_inf ) );
    CHECK( not( this->one > this->nullity ) );

    CHECK( ( this->p_inf > this->n_inf ) );
    CHECK( ( this->p_inf > this->n_one ) );
    CHECK( ( this->p_inf > this->zero ) );
    CHECK( ( this->p_inf > this->one ) );
    CHECK( not( this->p_inf > this->p_inf ) );
    CHECK( not( this->p_inf > this->nullity ) );

    CHECK( not( this->nullity > this->n_inf ) );
    CHECK( not( this->nullity > this->n_one ) );
    CHECK( not( this->nullity > this->zero ) );
    CHECK( not( this->nullity > this->one ) );
    CHECK( not( this->nullity > this->p_inf ) );
    CHECK( not( this->nullity > this->nullity ) );

}

