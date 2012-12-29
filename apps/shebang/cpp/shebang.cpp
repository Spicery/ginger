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
#include <string>
#include <vector>
#include <iostream>

#include <stddef.h>
#include <syslog.h>
#include <stdlib.h>
#include <unistd.h>

//#define DBG_SHEBANG

#define SHEBANG "shebang"

using namespace std;

class Main {
private:
	vector< string > args;
	vector< char * > argvector;

private:	
	void fill( vector< string > & args, vector< char * > & argv ) {
		for ( 
			vector< string >::iterator it = args.begin();
			it != args.end();
			++it
		) {
			argv.push_back( const_cast< char * >( it->c_str() ) );
		}
		argv.push_back( NULL );
	}


public:
	/**	We expect to be called as a #! script so the arguments should be
	  *		shebang [OPTIONS] filename
	  *	for the moment we will skip processing the options.
	  */
	void parseArgs( int argc, char ** argv, char **envp ) {
		//	The last argument is the filename.
		fstream filestr( argv[ argc - 1 ], fstream::in );
		for ( int n = 0; true; n++ ) {
			string line;
			getline( filestr, line );
			
			//	If we have come to the end of input, stop.
			if ( not filestr.good() ) break;
			
			//	If we find a non-#! line, stop.
			if ( not ( line.size() >= 2 && line[ 0 ] == '#' && line[ 1 ] == '!' ) ) break;
			
			//	Skip the first #! line.
			if ( n == 0 ) continue;	
			
			//	Save this line, stripped of the leading #!.
			args.push_back( string( line.begin() + 2, line.end() ) );
		}
		
		//	Push this filename back on the end. In the future this will
		//	probably be controlled by a conditional flag.
		args.push_back( argv[ argc - 1 ] );
				
		if ( args.size() < 2 ) {
			cerr << "Too few arguments for shebang" << endl;
			for ( vector<string>::iterator it = args.begin(); it != args.end(); ++it ) {	
				cerr << "  argument: " << *it << endl;
			}
			exit( EXIT_FAILURE );
		}
		this->fill( args, this->argvector );
	}

public:
	int run() {
		#ifdef DBG_SHEBANG
			for ( vector<string>::iterator it = args.begin(); it != args.end(); ++it ) {	
				cerr << "  argument: " << *it << endl;
			}
		#endif
		execv( argvector[0], &argvector[0] );
		return EXIT_FAILURE;	//	If we reach here then the exec failed!
	}
};

int main( int argc, char ** argv, char **envp ) {
	openlog( SHEBANG, 0, LOG_LOCAL2 );
	setlogmask( LOG_UPTO( LOG_INFO ) );
	Main main;
	main.parseArgs( argc, argv, envp );
	return main.run();
}
