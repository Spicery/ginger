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

#ifndef DATABASE_HPP
#define DATABASE_HPP

#include <string>

#include <sqlite3.h>

namespace Ginger {

class SqliteDb {
friend class SqliteStmnt;
private:
	sqlite3 * db;
	int flags;
	
public:
	sqlite3 * database();
	void open( const char *filename, int flags, const char *zVfs );
	void openReadOnly( const char *filename );
	void openReadWrite( const char *filename );
	void openReadWriteCreate( const char *filename );
	void setFlag( int f );
	void setNoMutex();
	void setFullMutex();
	/*void setSharedCache();
	void setPrivateCache();*/
	void exec( const char * sql_command );
	
public:
	SqliteDb();
	
	SqliteDb( sqlite3 * d );
	
	~SqliteDb();
};

class SqliteStmnt {
private:
	sqlite3_stmt *ppStmnt;
	
public:
	void reset();
	bool step();
	void bindText( const int n, const std::string & s );
	const unsigned char * columnText( const int n );

	
public:
	SqliteStmnt( SqliteDb & db, const char * sql_command );
	~SqliteStmnt();
};

} //namespace Ginger

#endif
