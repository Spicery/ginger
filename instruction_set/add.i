Ref ry = *( VMVP-- );
Ref rx = *( VMVP );
if ( IsSmall( rx ) && IsSmall( ry ) ) {
	long y = (long)ry;
	long x = (long)rx;
	long sum = x + y;
	if ( x < 0 ? sum <= y : sum >= y ) {
		*VMVP = ToRef( sum );
		RETURN( pc + 1 );
	} else {
    	throw Mishap( "Overflow detected in +" );
	}
} else {
	throw Mishap( "Small integers only" );
} 
