#include "cord.hpp"
#include "hash.hpp"

#define D0 ( sizeof( size_t ) )
#define D1 ( sizeof( unt ) )
#define D  ( D0 + D1 )

#define ToSizeTPtr( x ) 	((size_t *)(x))
#define ToUntPtr( x )		((unt *)(x))

//Cord cord_new( Pool pool, const char *s ) {
//	Cord c = new CordClass( s );
//	c->hashcode = hash_function( s );
//	return c;
//}
//
//size_t cord_length( Cord c ) {
//	return *ToSizeTPtr( c - D );
//}

//unt cord_hash( Cord c ) {
//	return c->hashcode;
//}
//
//bool cord_eq( Cord x, Cord y ) {
//	return(
//		x->hashcode == y->hashcode ?
//		x->text.compare( y->text ) :
//		false
//	);
//}
//
////	This is a hack!  However, is gets us by for a while :-)
//Cord cord_copy( Pool pool, Cord c ) {
//	return new CordClass( c );
//}
//