#!/usr/local/bin/ginger-script -O,-gcmn,--fn2code

define f( x ) =>>
	switch x case 0 then 'a' else 'b' endswitch;
enddefine;

[ f( 0 ), f( 1 ) ];
