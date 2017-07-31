from pathlib import Path

WIDTHS = {}

def loadWidths():
	for p in Path( __file__ ).parent.joinpath( '../../../instruction_set' ).iterdir():
		if p.is_file() and p.suffix.startswith( '.i' ):
			WIDTHS[ p.stem.replace( '_', '.' ) ] = len( p.suffix[1:] )

def width( instruction_name ):
	if not WIDTHS:
		loadWidths()
	return WIDTHS[ instruction_name ]

