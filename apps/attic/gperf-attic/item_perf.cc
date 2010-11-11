/* ANSI-C code produced by gperf version 3.0.3 */
/* Command-line: gperf -D -t -T -L ANSI-C -e '#' item.perf  */
/* Computed positions: -k'1,$' */

#if !((' ' == 32) && ('!' == 33) && ('"' == 34) && ('#' == 35) \
      && ('%' == 37) && ('&' == 38) && ('\'' == 39) && ('(' == 40) \
      && (')' == 41) && ('*' == 42) && ('+' == 43) && (',' == 44) \
      && ('-' == 45) && ('.' == 46) && ('/' == 47) && ('0' == 48) \
      && ('1' == 49) && ('2' == 50) && ('3' == 51) && ('4' == 52) \
      && ('5' == 53) && ('6' == 54) && ('7' == 55) && ('8' == 56) \
      && ('9' == 57) && (':' == 58) && (';' == 59) && ('<' == 60) \
      && ('=' == 61) && ('>' == 62) && ('?' == 63) && ('A' == 65) \
      && ('B' == 66) && ('C' == 67) && ('D' == 68) && ('E' == 69) \
      && ('F' == 70) && ('G' == 71) && ('H' == 72) && ('I' == 73) \
      && ('J' == 74) && ('K' == 75) && ('L' == 76) && ('M' == 77) \
      && ('N' == 78) && ('O' == 79) && ('P' == 80) && ('Q' == 81) \
      && ('R' == 82) && ('S' == 83) && ('T' == 84) && ('U' == 85) \
      && ('V' == 86) && ('W' == 87) && ('X' == 88) && ('Y' == 89) \
      && ('Z' == 90) && ('[' == 91) && ('\\' == 92) && (']' == 93) \
      && ('^' == 94) && ('_' == 95) && ('a' == 97) && ('b' == 98) \
      && ('c' == 99) && ('d' == 100) && ('e' == 101) && ('f' == 102) \
      && ('g' == 103) && ('h' == 104) && ('i' == 105) && ('j' == 106) \
      && ('k' == 107) && ('l' == 108) && ('m' == 109) && ('n' == 110) \
      && ('o' == 111) && ('p' == 112) && ('q' == 113) && ('r' == 114) \
      && ('s' == 115) && ('t' == 116) && ('u' == 117) && ('v' == 118) \
      && ('w' == 119) && ('x' == 120) && ('y' == 121) && ('z' == 122) \
      && ('{' == 123) && ('|' == 124) && ('}' == 125) && ('~' == 126))
/* The character set is not based on ISO-646.  */
#error "gperf generated tables don't work with this execution character set. Please report a bug to <bug-gnu-gperf@gnu.org>."
#endif

#line 1 "item.perf"

#include <string>
using namespace std;

#include "functor.hpp"
#include "item.hpp"
#include "key.hpp"
#include "special.hpp"
#include "role.hpp"
//  Special constants to help with regular text layout.
#define Clo CloserRole
#define PrF PrefixRole
#define PoF PostfixRole
#define BoF BothfixRole
#define Imm ImmediateRole
#define POp PrefixOpRole
#define BOp BinaryOpRole
#define SOp SuffixOpRole
#define InL InLineRole

static ItemClass item_default_struct =
    {
        "<default>",       //  name
        fnc___fnc_default,   //  functor
        PrefixRole,
        0,
        0
    };
ItemClass * item_default = &item_default_struct;

static ItemClass item_unary_op_struct =
    {
        "<default>",       //  name
        fnc___fnc_default,   //  functor
        PrefixRole,
        0,
        0
    };
ItemClass * item_unary_op = &item_unary_op_struct;

static ItemClass item_binary_op_struct =
    {
        "<default>",        //  name
        fnc___fnc_default,  //  functor
        PrefixRole,
        0,
        0
    };
ItemClass * item_binary_op = &item_binary_op_struct;

bool item_is_neg_num( Item it ) {
    return(
        it->functor == fnc_int &&
        SmallToInt( it->extra ) < 0
    );
}

int item_int( Item it ) {
    return ToInt( it->extra );
}

bool item_is_prefix( Item it ) {
    return IsPrefix( it->role );
}

bool item_is_postfix( Item it ) {
    return IsPostfix( it->role );
}

bool item_is_anyfix( Item it ) {
    return IsAnyfix( it->role );
}

bool item_is_binary( Item it ) {
    return IsBinary( it->role );
}


#define TOTAL_KEYWORDS 55
#define MIN_WORD_LENGTH 1
#define MAX_WORD_LENGTH 10
#define MIN_HASH_VALUE 1
#define MAX_HASH_VALUE 92
/* maximum key range = 92, duplicates = 0 */

#ifdef __GNUC__
__inline
#else
#ifdef __cplusplus
inline
#endif
#endif
static unsigned int
hash (register const char *str, register unsigned int len)
{
  static unsigned char asso_values[] =
    {
      93, 93, 93, 93, 93, 93, 93, 93, 93, 93,
      93, 93, 93, 93, 93, 93, 93, 93, 93, 93,
      93, 93, 93, 93, 93, 93, 93, 93, 93, 93,
      93, 93, 93, 45, 45, 93, 93, 93, 45, 40,
      30, 51, 25, 25, 20, 46, 25, 60, 30, 93,
      93, 93, 93, 93, 93, 93, 93, 93, 93, 20,
      35,  5,  0, 35, 25, 55, 93, 93, 93, 93,
      93, 93, 93, 93, 93, 93, 93, 93, 93, 93,
      93, 93, 93, 93, 93, 93, 93, 93, 93, 93,
      93, 40, 35, 35, 35, 93, 15, 93, 25, 93,
       0, 10,  5, 15, 93, 93, 50, 93,  0, 93,
       0, 10, 35, 93,  0, 20,  0,  0, 40,  0,
       5, 93, 93, 30, 30,  0,  0, 93, 93, 93,
      93, 93, 93, 93, 93, 93, 93, 93, 93, 93,
      93, 93, 93, 93, 93, 93, 93, 93, 93, 93,
      93, 93, 93, 93, 93, 93, 93, 93, 93, 93,
      93, 93, 93, 93, 93, 93, 93, 93, 93, 93,
      93, 93, 93, 93, 93, 93, 93, 93, 93, 93,
      93, 93, 93, 93, 93, 93, 93, 93, 93, 93,
      93, 93, 93, 93, 93, 93, 93, 93, 93, 93,
      93, 93, 93, 93, 93, 93, 93, 93, 93, 93,
      93, 93, 93, 93, 93, 93, 93, 93, 93, 93,
      93, 93, 93, 93, 93, 93, 93, 93, 93, 93,
      93, 93, 93, 93, 93, 93, 93, 93, 93, 93,
      93, 93, 93, 93, 93, 93, 93, 93, 93, 93,
      93, 93, 93, 93, 93, 93, 93
    };
  return len + asso_values[(unsigned char)str[len - 1]] + asso_values[(unsigned char)str[0]+1];
}

#ifdef __GNUC__
__inline
#ifdef __GNUC_STDC_INLINE__
__attribute__ ((__gnu_inline__))
#endif
#endif
ItemClass *
in_word_set (register const char *str, register unsigned int len)
{
  static ItemClass wordlist[] =
    {
#line 135 "item.perf"
      {"}",              fnc_cbrace,         PoF,  0,              0},
#line 99 "item.perf"
      {"=>",             fnc_arrow,          BOp,  prec_arrow,     0},
#line 131 "item.perf"
      {"var",            fnc_var,            PrF,  0,              0},
#line 127 "item.perf"
      {"then",           fnc_then,           PrF,  0,              0},
#line 110 "item.perf"
      {"const",          fnc_const,          PrF,  0,              0},
#line 97 "item.perf"
      {"=",              fnc_eq,             BOp,  1100,           0},
#line 98 "item.perf"
      {"==",             fnc_eq,             BOp,  1000,           0},
#line 117 "item.perf"
      {"endfn",          fnc_endfn,          PrF,  0,              0},
#line 118 "item.perf"
      {"endfor",         fnc_endfor,         PrF,  0,              0},
#line 96 "item.perf"
      {"<=",             fnc_lte,            BOp,  1400,           0},
#line 125 "item.perf"
      {"not",            fnc_not,            POp,  prec_not,       0},
#line 128 "item.perf"
      {"true",           fnc_true,           Imm,  0,              sys_true},
#line 119 "item.perf"
      {"endif",          fnc_endif,          PrF,  0,              0},
#line 114 "item.perf"
      {"elseif",         fnc_elseif,         PrF,  0,              0},
#line 122 "item.perf"
      {"fn",             fnc_fn,             PrF,  0,              0},
#line 123 "item.perf"
      {"for",            fnc_for,            PrF,  0,              0},
#line 113 "item.perf"
      {"else",           fnc_else,           PrF,  0,              0},
#line 132 "item.perf"
      {"while",          fnc_while,          PrF,  0,              0},
#line 106 "item.perf"
      {"_print",         fnc__print,         InL,  0,              &spc__print},
#line 112 "item.perf"
      {"do",             fnc_do,             PrF,  0,              0},
#line 107 "item.perf"
      {"_putchar",       fnc__putchar,       InL,  0,              &spc__putchar},
#line 116 "item.perf"
      {"enddefine",      fnc_enddefine,      PrF,  0,              0},
#line 111 "item.perf"
      {"define",         fnc_define,         PrF,  0,              0},
#line 93 "item.perf"
      {":=",             fnc_assign,         BOp,  2000,           0},
#line 109 "item.perf"
      {"and",            fnc_and,            BOp,  1700,           0},
#line 121 "item.perf"
      {"false",          fnc_false,          Imm,  0,              sys_false},
#line 108 "item.perf"
      {"absent",         fnc_absent,         Imm,  0,              sys_absent},
#line 134 "item.perf"
      {"||",             fnc_absor,          BOp,  1800,           0},
#line 120 "item.perf"
      {"endunless",      fnc_endunless,      PrF,  0,              0},
#line 115 "item.perf"
      {"elseunless",     fnc_elseunless,     PrF,  0,              0},
#line 100 "item.perf"
      {">",              fnc_gt,             BOp,  1300,           0},
#line 126 "item.perf"
      {"or",             fnc_or,             BOp,  1800,           0},
#line 95 "item.perf"
      {"<",              fnc_lt,             BOp,  1200,           0},
#line 101 "item.perf"
      {">=",             fnc_gte,            BOp,  1500,           0},
#line 130 "item.perf"
      {"until",          fnc_until,          PrF,  0,              0},
#line 87 "item.perf"
      {"+",              fnc_add,            BOp,  600,            0},
#line 88 "item.perf"
      {"++",             fnc_append,         BOp,  800,            0},
#line 85 "item.perf"
      {"*",              fnc_mul,            BOp,  400,            0},
#line 86 "item.perf"
      {"**",             fnc_pow,            BOp,  300,            0},
#line 94 "item.perf"
      {";",              fnc_semi,           PoF,  prec_semi,      0},
#line 124 "item.perf"
      {"if",             fnc_if,             PrF,  0,              0},
#line 133 "item.perf"
      {"{",              fnc_obrace,         PrF,  100,            0},
#line 102 "item.perf"
      {"??",             fnc_present,        PoF,  1900,           0},
#line 129 "item.perf"
      {"unless",         fnc_unless,         PrF,  0,              0},
#line 89 "item.perf"
      {",",              fnc_comma,          BOp,  prec_comma,     0},
#line 105 "item.perf"
      {"]",              fnc_cbracket,       Clo,  0,              0},
#line 90 "item.perf"
      {"-",              fnc_sub,            BOp,  700,            0},
#line 104 "item.perf"
      {"[",              fnc_obracket,       PrF,  100,            0},
#line 84 "item.perf"
      {")",              fnc_cparen,         Clo,  0,              0},
#line 103 "item.perf"
      {"@",              fnc_at,             PoF,  900,            0},
#line 83 "item.perf"
      {"(",              fnc_oparen,         BoF,  100,            0},
#line 91 "item.perf"
      {".",              fnc_dot,            PoF,  200,            0},
#line 82 "item.perf"
      {"&&",             fnc_absand,         BOp,  1700,           0},
#line 92 "item.perf"
      {"/",              fnc_div,            BOp,  500,            0},
#line 81 "item.perf"
      {"!!",             fnc_absnot,         POp,  prec_not,       0}
    };

  static signed char lookup[] =
    {
      -1,  0,  1,  2,  3,  4,  5,  6, -1, -1,  7,  8,  9, 10,
      11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, -1, 22, 23,
      24, -1, 25, 26, 27, -1, 28, 29, 30, 31, -1, -1, -1, 32,
      33, -1, -1, 34, 35, 36, -1, -1, -1, 37, 38, -1, -1, -1,
      39, 40, -1, -1, -1, 41, 42, -1, -1, -1, 43, 44, -1, -1,
      -1, 45, 46, -1, -1, -1, 47, 48, -1, -1, -1, 49, 50, -1,
      -1, -1, 51, 52, -1, -1, -1, 53, 54
    };

  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      register int key = hash (str, len);

      if (key <= MAX_HASH_VALUE && key >= 0)
        {
          register int index = lookup[key];

          if (index >= 0)
            {
              register const char *s = wordlist[index].name;

              if (*str == *s && !strcmp (str + 1, s + 1))
                return &wordlist[index];
            }
        }
    }
  return 0;
}
