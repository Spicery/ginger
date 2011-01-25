/******************************************************************************\
	Copyright (c) 2010 Stephen Leach. AppGinger is distributed under the terms 
	of the GNU General Public License. This file is part of AppGinger.

    AppGinger is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    AppGinger is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with AppGinger.  If not, see <http://www.gnu.org/licenses/>.
\******************************************************************************/

#include <string>
#include <iostream>

#include "database.hpp"

#include "mishap.hpp"

namespace Ginger {
using namespace std;

sqlite3 * SqliteDb::database() {
	return db;
}

void SqliteDb::open( const char *filename, int flags, const char *zVfs ) {
	int status = sqlite3_open_v2( filename, &this->db, flags, zVfs );
	if ( status != SQLITE_OK ) {
		throw SystemError( "Cannot open SQLITE database" ).culprit( "Filename", filename );
	}
}

void SqliteDb::openReadOnly( const char *filename ) {
	this->open( filename, SQLITE_OPEN_READONLY | this->flags, NULL );
}

void SqliteDb::openReadWrite( const char *filename ) {
	this->open( filename, SQLITE_OPEN_READWRITE | this->flags, NULL );
}

void SqliteDb::openReadWriteCreate( const char *filename ) {
	this->open( filename, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | this->flags, NULL );
}

void SqliteDb::setFlag( int f ) {
	this->flags |= f;
}
	
void SqliteDb::setNoMutex() {
	this->flags |= SQLITE_OPEN_NOMUTEX;
}

void SqliteDb::setFullMutex() {
	this->flags |= SQLITE_OPEN_FULLMUTEX;
}

/*void SqliteDb::setSharedCache() {	
	this->flags |= SQLITE_OPEN_SHAREDCACHE;
}

void SqliteDb::setPrivateCache() {
	this->flags |= SQLITE_OPEN_PRIVATECACHE;
}*/		

void SqliteDb::exec( const char * sql_command ) {
	char * errMsg;
	int status = sqlite3_exec( this->db, sql_command, NULL, NULL, &errMsg );
	if ( status != SQLITE_OK ) {
		throw SystemError( errMsg );
	}
}

	
SqliteDb::SqliteDb() : db( NULL ), flags( 0 ) {}

SqliteDb::SqliteDb( sqlite3 * d ) : db( d ), flags( 0 ) {}

SqliteDb::~SqliteDb() {
	if ( db != NULL ) {
		int status = sqlite3_close( this->db );
		if ( status != SQLITE_OK ) {
			throw SystemError( "Failed to close SQLITE database" );
		}
	}
}

void SqliteStmnt::reset() {
	if ( sqlite3_reset( ppStmnt ) != SQLITE_OK ) throw SystemError( "Cannot reset SQLITE prepared statement" );
}

bool SqliteStmnt::step() {
	int status = sqlite3_step( ppStmnt );
	if ( status != SQLITE_DONE && status != SQLITE_OK && status != SQLITE_ROW ) {
		cerr << "SQL statement failed: " << sqlite3_errmsg( sqlite3_db_handle( this->ppStmnt ) ) << endl;
		throw SystemError( "Failed to step SQLITE statement" );
	}
	return status == SQLITE_ROW;
}

void SqliteStmnt::bindText( const int n, const string & s ) {
	int status = sqlite3_bind_text( this->ppStmnt, n, s.c_str(), -1, SQLITE_TRANSIENT );
	if ( status != SQLITE_OK ) {
		throw SystemError( "Could not bind text to argument of prepared statement" ).culprit( "Text", s );
	}
}

const unsigned char * SqliteStmnt::columnText( const int n ) {
	const unsigned char * t = sqlite3_column_text( this->ppStmnt, n );
	if ( t == NULL ) {
		throw SystemError( "Cannot get text from column" ).culprit( "Column", n );
	}
	return t;
}

const unsigned char *sqlite3_column_text(sqlite3_stmt*, int iCol);


SqliteStmnt::SqliteStmnt( SqliteDb & db, const char * sql_command ){
	int status = sqlite3_prepare_v2( db.database(), sql_command, -1, &ppStmnt, NULL );
	if ( status != SQLITE_OK ) {
		throw SystemError( "Cannot prepare SQLITE statement" ).culprit( "Command", sql_command );
	}
}

	
SqliteStmnt::~SqliteStmnt() {
	if ( ppStmnt != NULL ) {
		int status = sqlite3_finalize( ppStmnt );
		if ( status != SQLITE_OK ) {
			std::cerr << "Error disposing of statement" << std::endl;
			std::cerr << "Status " << status << std::endl;
			throw SystemError( "Failed to dispose of SQLITE statement" );
		}
	}
}

} // namespace Ginger
