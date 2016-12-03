#include <iostream>
#include <cstdlib>
#include <string>
#include <unistd.h>
#include <vector>

using namespace std;

// #define DEBUG 1
#define GINGER "ginger"

static void runDefaultProgram() {
	execlp( "ginger-cli", "ginger-cli", "--grammar=common", (char *)0 );	
}

static void lacksCmdOption( int argc, char * argv[] ) {
	string cmd_name = "ginger-cli";

	vector< char * > args;
	args.push_back( &cmd_name[0] );
	args.push_back( const_cast< char * >( "--grammar=common" ) );
	for ( int i = 1; i < argc; i++ ) {
		args.push_back( argv[ i ] );
	}		
	args.push_back( 0 );

	execvp( &cmd_name[0], &args[ 0 ] );

	cerr << "Installation error: ginger-cli executable cannot be found." << endl;
}

static void hasCmdOption( int argc, char * argv[] ) {
	string cmd_name = string( GINGER "-" ) + argv[1];

	vector< char * > args;
	args.push_back( &cmd_name[0] );
	for ( int i = 2; i < argc; i++ ) {
		args.push_back( argv[ i ] );
	}		
	args.push_back( 0 );

	execvp( &cmd_name[0], &args[ 0 ] );

	cerr << "Cannot execute command: " << cmd_name << endl;
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
