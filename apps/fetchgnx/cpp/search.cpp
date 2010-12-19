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

#include <fstream>
#include <iostream>

#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>

#include "search.hpp"
#include "mishap.hpp"
#include "folderscan.hpp"

using namespace std;

//#define DBG_SEARCH

#define FILE2GNX "file2gnx"

Search::Search( std::string project_folder ) :
	project_folder( project_folder ),
	project_cache( project_folder )
{
}

Search::~Search() {
}

//
//	Insecure. We need to do this more neatly. It would be best if common2gnx
//	and lisp2gnx could handle being passed a filename as an argument. This
//	would be both more secure and efficient.
//
static void run( string command, string pathname ) {
	//cout << "running " << command << " " << pathname << endl;
	int pipe_fd[ 2 ];
	const char * cmd = command.c_str();
	pipe( pipe_fd );
	pid_t pid = fork();
	switch ( pid ) {
		case -1: {	//	Something went wrong.
			throw Mishap( "Child process unexpectedly failed" ).culprit( "Command", command ).culprit( "Argument", pathname );
			break;
		}
		case 0: {	//	Child process - exec into command.
			//	Close the unused read descriptor.
			close( pipe_fd[0] );
			//	Attach stdout to the write descriptor.
			dup2( pipe_fd[1], STDOUT_FILENO );
			execl( cmd, cmd, pathname.c_str(), NULL );
			break;
		}
		default: {	// 	Parent process.
			//	Close the unused write descriptor.
			close( pipe_fd[1] );
			
			//	Read from the read descriptor until exhausted.
			const int input_fd = pipe_fd[0];
			static char buf[ 1024 ];
			for (;;) {
				ssize_t n = read( input_fd, buf, sizeof( buf ) );
				if ( n == 0 ) break;
				write( STDOUT_FILENO, buf, n );
			}
			
			int return_value_of_child;
			wait( &return_value_of_child );
			break;
		}
	}
}

static void dumpFile( const string & fullname ) {
	//cout << "Dumping file " << fullname << endl;
	run( EXEC_DIR "/" FILE2GNX, fullname );
}

void Search::returnDefinition( PackageCache * c, string name ) {
	VarInfo * vfile = c->variableFile( name );
	/*cout << "name = " << name << endl;
	cout << "vfile " << vfile << endl;
	cout << "  pathname " << vfile->getPathName() << endl;
	cout << "  var_name " << vfile->getVarName() << endl;
	cout << "  frozen? " << vfile->frozen << endl;*/
	
	if ( vfile != NULL ) {
		dumpFile( vfile->getPathName() );
	} else {
		vector< string > from_list;
		c->fillFromList( from_list );
		for (
			vector< string >::iterator it = from_list.begin();
			it != from_list.end();
			++it
		) {
			PackageCache * c = this->project_cache.fetchPackageCache( *it );
			VarInfo * v = c->variableFile( name );
			if ( v != NULL ) {
				if ( vfile == NULL ) {
					vfile = v;
				} else {
					throw Mishap( "Ambiguous sources for definition" ).culprit( "Source 1", vfile->getPathName() ).culprit( "Source 2", v->getPathName() );
				}
			}
		}
		if ( vfile != NULL ) {
			dumpFile( vfile->getPathName() );
		} else {
			throw  (
				Mishap( "Cannot find variable" ).
				culprit( "Variable", name ).
				culprit( "Package", c->getPackageName() )
			);
		}
	}
}

void Search::findDefinition( string & pkg, string & name ) {
	this->returnDefinition( this->project_cache.fetchPackageCache( pkg ), name );
}

void Search::loadPackage( string & pkg ) {
	//cout << "Need to load package " << pkg << endl;
	PackageCache * c = this->project_cache.fetchPackageCache( pkg );
	string path = c->getLoadPath();
	if ( path.size() > 0 ) {
		dumpFile( path );	
	} else {
		cout << "  load path was not defined" << endl;
	}
}
