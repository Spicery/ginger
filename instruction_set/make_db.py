#!/usr/bin/python3
# Creates the instruction_set.db SQLITE3 file that will live in
# ${GINGER_HOME}/share/ginger/instruction_set/instruction_set.db

from pathlib import Path
import sqlite3

def loadStats(): 
	stats = {}
	for p in Path( __file__ ).parent.iterdir():
		if p.is_file() and p.suffix.startswith( '.i' ):
			stats[ p.stem ] = p.suffix[1:]
	return stats

def createInstructionSetDB( db_fname ):
	with sqlite3.connect( db_fname ) as conn:
		conn.execute( 'DROP TABLE IF EXISTS instruction' )
		conn.execute( 'CREATE TABLE instruction( name text PRIMARY KEY, codename text UNIQUE, width int, fields text )' )
		conn.commit()
		for iname, ifields in loadStats().items():
			conn.execute( 
				'INSERT INTO instruction VALUES ( ?, ?, ?, ? )', 
				( iname, iname.replace( '_', '.' ), len( ifields ), ifields )
			)
		conn.commit()

if __name__ == "__main__":
	createInstructionSetDB( 'instruction_set.db' )
