Ref rx = *VMVP;
if ( IsSmall( rx ) ) {
	long x = (long)rx;
	long sum = x + (long)LongToSmall( 1 );
	if ( sum > x ) {
		*( VMVP ) = ToRef( sum );
		RETURN( pc + 1 );
	} else {
		throw Mishap( "integer overflow detected in x + 1" );
	}
} else {
	throw Mishap( "Small integer needed" );
}

