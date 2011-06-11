#include <stdlib.h>

#include "gnx.hpp"

using namespace Ginger;

int main( int argc, char **argv, char **envp ) {
	GnxReader reader;
	shared< Gnx > g( reader.readGnx() );
	g->prettyPrint();
	return EXIT_SUCCESS;
}
