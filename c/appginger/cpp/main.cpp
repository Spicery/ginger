#include "appcontext.hpp"

int main( int argc, char **argv, char **envp ) {
	AppContext appg;
	return appg.main( argc, argv, envp );
}
