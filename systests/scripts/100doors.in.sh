#!/usr/local/bin/ginger-script -O,-m1,-gcommon

val numdoors := 100;

val doors := initMutableVector( numdoors, false );
for i from 1 to numdoors do
    for j from i by i to numdoors do
    	#setIndexVector( not( doors[ j ] ), j, doors );
    	doors[j] ::= not( doors[j] )
    endfor;
    if doors[ i ] then
    	println( "Door ", i, " open" );
    endif
endfor;
