#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>

#include <unistd.h>

using namespace std;

// #define DEBUG 1
#define GINGER "ginger"

static std::string programPath() {
	string path;
	if ( USESNAP ) {
		const char * snap = getenv( "SNAP" );
		if ( snap ) {
			path += snap;
		}
	}
	path += INSTALL_BIN "/";
	return path;
}

static void runDefaultProgram() {
	std::string cmd( programPath() );
	cmd += "ginger-cli";
	execl( cmd.c_str(), cmd.c_str(), "--grammar=common", (char *)0 );	
	cerr << "Installation error [1]: ginger-cli executable cannot be found: " << cmd << endl;
}

static void lacksCmdOption( int argc, char * argv[] ) {
	string cmd( programPath() );
	cmd += "ginger-cli";

	vector< char * > args;
	args.push_back( &cmd[0] );
	args.push_back( const_cast< char * >( "--grammar=common" ) );
	for ( int i = 1; i < argc; i++ ) {
		args.push_back( argv[ i ] );
	}		
	args.push_back( 0 );

	execv( &cmd[0], &args[ 0 ] );

	cerr << "Installation error [2]: ginger-cli executable cannot be found: " << cmd << endl;
}

static void hasCmdOption( int argc, char * argv[] ) {
	string cmd( programPath() );
	cmd += string( GINGER "-" );
	cmd += argv[1];

	vector< char * > args;
	args.push_back( &cmd[0] );
	for ( int i = 2; i < argc; i++ ) {
		args.push_back( argv[ i ] );
	}		
	args.push_back( 0 );

	execv( &cmd[0], &args[ 0 ] );

	cerr << "Installation error [3]: ginger-cli executable cannot be found: " << cmd << endl;
}

int main( int argc, char * argv[] ) {
	if ( argc <= 1 ) {
		runDefaultProgram();
	} else {
		const bool has_cmd = argv[1][0] != '-';
		if ( has_cmd ) {
			hasCmdOption( argc, argv );
		} else {
			lacksCmdOption( argc, argv );
		}
	}
	std::cerr << "Usage: " << GINGER << " <command> [<options>] [<arguments>]" << std::endl;
	return EXIT_FAILURE;
}
