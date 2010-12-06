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
using namespace std;

#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>

#include "search.hpp"
#include "mishap.hpp"
#include "folderscan.hpp"

//#define DBG_SEARCH

#define AUTO_SUFFIX 			".auto"
#define AUTO_SUFFIX_SIZE 		sizeof( AUTO_SUFFIX )

#define GNX_SUFFIX 				".gnx"
#define GNX_SUFFIX_SIZE 		sizeof( GNX_SUFFIX )

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


static void dumpFile( string & fullname ) {
	run( EXEC_DIR "/" FILE2GNX, fullname );
}

static void returnDefinition( PackageCache * c, string name ) {
	VarInfo * vfile = c->variableFile( name );
	if ( vfile != NULL ) {
		dumpFile( vfile->pathname );
	} else {
		throw 
			Mishap( "Cannot find variable" ).
			culprit( "Variable", name ).
			culprit( "Package", c->getPackageName() );
	}
}


void Search::findDefinition( string pkg, string name ) {
	PackageCache * c = this->project_cache.getPackageCache( pkg );
	if ( c != NULL ) {
		returnDefinition( c, name );
	} else {
		//cout << "NOT FOUND" << endl;
		PackageCache * newc = new PackageCache( pkg );
		this->project_cache.putPackageCache( pkg, newc );
		
		FolderScan fscan( this->project_folder + "/" + pkg );
		while ( fscan.nextFolder() ) {
			string entry = fscan.entryName();
	
			//	Check that -entry- matches *.auto
			if ( entry.find( AUTO_SUFFIX, entry.size() - AUTO_SUFFIX_SIZE ) == string::npos ) continue;
			#ifdef DBG_SEARCH
				cout << "*.auto: " << entry << endl;
			#endif
			
			FolderScan files( fscan.folderName() + "/" + entry );
			while ( files.nextFile() ) {
				string fname = files.entryName();
				#ifdef DBG_SEARCH
					cout << "Entry : " << fname << endl;
				#endif
				
				size_t n = fname.rfind( '.' );
				if ( n == string::npos ) continue;
				
				
				string root = fname.substr( 0, n );
				string extn = fname.substr( n + 1 );
				
				#ifdef DBG_SEARCH
					cout << "Adding " << root << " -> " << ( files.folderName() + "/" + fname ) <<  endl;
				#endif
				newc->putPathName( root, files.folderName() + "/" + fname );		
			}	
		}
		
		returnDefinition( newc, name );
	}
}
