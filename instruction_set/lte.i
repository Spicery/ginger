Ref b = *( VMVP-- );
Ref a = *( VMVP );

if ( not( IsSmall( a ) ) or not( IsSmall( b ) ) ) {
	throw Mishap( "Small integers needed" );
}

*( VMVP ) = ToLong( a ) <= ToLong( b ) ? SYS_TRUE : SYS_FALSE;
RETURN( pc + 1 );
