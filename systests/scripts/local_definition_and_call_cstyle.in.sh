#!/usr/local/bin/ginger-script --grammar=cstyle

function enlist( x ) {
	return [ x, x ];
}

enlist( "C-Style!" );