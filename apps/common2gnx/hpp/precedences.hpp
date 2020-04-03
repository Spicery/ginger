#ifndef PRECEDENCES_HPP
#define PRECEDENCES_HPP

enum {
    prec_tight          =    100,
    prec_dhat           =    150,

    prec_apply_tight    =    200,   //  A key break point.

    prec_explode        =    250,
    prec_trap           =    250,   /// @todo I suspect this is wrong.

    prec_pow            =    300,   //  Start of arithmetic
    prec_div            =    400,
    prec_negate         =    500,
    prec_arith_mul      =    500,
    prec_quo            =    500,
    prec_mod            =    500,
    prec_arith_sub      =    600,
    prec_arith_add      =    700,   //  End of arithmetic.

    prec_append         =    800,
    prec_apply_loose    =    900,

    prec_identical      =   1000,   //  A key break point

    prec_equal          =   1100,   //  Start of comparisons.
    prec_lt             =   1200,
    prec_gt             =   1300,
    prec_lte            =   1400,
    prec_gte            =   1500,
    prec_not            =   1600,
    prec_bang           =   1650,
    prec_abs_and        =   1700,
    prec_and            =   1700,
    prec_abs_or         =   1800,
    prec_or             =   1800,
    prec_also           =   1800,
    prec_isnt_absent    =   1900,   //  End of comparisons.

    prec_assign         =   2000,   //  A key break point.

    prec_from           =   2000,
    prec_in             =   2000,
    prec_cross          =   2100,
    prec_zip            =   2200,
    prec_where          =   2300,
    prec_whileuntil     =   2300,
    prec_then           =   2300,
    prec_finally        =   2300,
    prec_do             =   2400,
    prec_arrow          =  10000,
    prec_comma          =  20000,
    prec_semi           =  30000,
    prec_max            = 100000
};

#endif
