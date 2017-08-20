#!/usr/local/bin/ginger-script -O,-gcmn

define f( x ) =>>
	switch x case 0 then 'a' else 'b' endswitch;
enddefine;

print( [ f( 0 ), f( 1 ) ] );
