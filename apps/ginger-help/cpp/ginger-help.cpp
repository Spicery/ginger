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

#include <iostream>
#include <string>
#include <map>
#include <fstream>
#include <sstream>
#include <memory>

#include <string.h>
#include <errno.h>

#include <getopt.h>
#include <unistd.h>
#include <stddef.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>


#define PACKAGE_NAME "ginger-help"

using namespace std;

const char * open_exe =
#ifdef __APPLE__
    "/usr/bin/open"
#else
    "/usr/bin/xdg-open"
#endif
;

int main( int argc, char ** argv ) {
    string topic;
    if ( argc == 2 ) {
        topic = argv[ 1 ];
    } else if ( argc == 1 ) {
        //  Default is fine.
        topic = "index";
    } else {
        cerr << "Usage: ginger-help <topic>" << endl;
        return EXIT_FAILURE;
    }
    string filename( INSTALL_LIB "/html/" );
    filename += topic;
    filename += ".html";
    if ( fork() == 0 ) {
        execl( open_exe, open_exe, filename.c_str(), (char *)0 );
        perror( PACKAGE_NAME );
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
