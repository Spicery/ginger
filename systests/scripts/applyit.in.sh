#!/usr/local/bin/ginger-script -gcmn

f := fn (x) =>> x + 1 endfn;
define applyit( x, f ) =>> f( x ) enddefine;
applyit( 99, f );
