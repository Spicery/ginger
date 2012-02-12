#ifndef TOKTYPE_HPP
#define TOKTYPE_HPP

#define X(a,b) a,
typedef enum TokType {
#include "toktype.xdef"
} TokType;
#undef X


extern const bool tok_type_as_direction( TokType fnc );
extern const char *tok_type_name( TokType fnc );
extern const char *tok_type_as_tag( TokType fnc );
extern const char *tok_type_as_sysapp( TokType fnc );
extern const char *tok_type_as_type( TokType fnc );
//const char *functor_as_value( Functor fnc );

#endif

