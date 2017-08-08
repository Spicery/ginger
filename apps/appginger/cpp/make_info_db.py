#!/usr/bin/python3

import sys
import json
import sqlite3

def addSysfns( data, conn ):
	conn.execute( 'DROP TABLE IF EXISTS sysfn' )
	conn.execute( 'CREATE TABLE sysfn( name text PRIMARY KEY, [in] text, [out] text, docstring text, flavour text )'  )
	for ( sysapp_name, info ) in data.items():
		conn.execute( 
			'INSERT INTO sysfn VALUES( ?, ?, ?, ?, ? )', 
			( sysapp_name, info[ 'in' ], info[ 'out' ], info[ 'docstring' ], info[ 'flavour' ] ) 
		)

def addEngines( data, conn ):
	conn.execute( 'DROP TABLE IF EXISTS engine' )
	conn.execute( 'CREATE TABLE engine( short_name text PRIMARY KEY, long_name text, description text )'  )
	for info in data:
		conn.execute( 
			'INSERT INTO engine VALUES( ?, ?, ? )', 
			( info[ 'short.name' ], info[ 'long.name' ], info[ 'description' ] )
		)			

  # "ipr": [
  #   { "topic": "license", "url": "http://www.gnu.org/licenses/gpl-3.0.txt" },
  #   { "topic": "copyright", "notice": "Copyright (c) 2010 Stephen Leach", "email": "stephen.leach@steelypip.com" }
  # ],
def addPathValues( key, jdata, conn ):
	for d in jdata[ key ]:
		p = '.'.join(( key, d[ 'topic' ] ))
		for ( k, v ) in d.items():
			if k != 'topic':
				conn.execute( 
					'INSERT INTO property VALUES( ?, ? )', 
					( '.'.join(( p, k )), v )
				)			

def addProperties( data, conn ):
	conn.execute( 'DROP TABLE IF EXISTS property' )
	conn.execute( 'CREATE TABLE property( key_path text, value text )' )
	addPathValues( 'release', data, conn )
	addPathValues( 'ipr', data, conn )
	addPathValues( 'community', data, conn )

def main( jdata, fname ):
	with sqlite3.connect( fname ) as conn:
		addSysfns( jdata[ 'std' ], conn )
		addEngines( jdata[ 'engines' ], conn )
		addProperties( jdata, conn )

if __name__ == "__main__":
	main( json.load( sys.stdin ), 'info-debug.db' )
