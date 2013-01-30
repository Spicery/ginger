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
#include <algorithm>    // std::find_if
#include <string>
#include <map>
#include <fstream>
#include <sstream>
#include <tr1/memory>

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <getopt.h>
#include <unistd.h>
#include <stddef.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <readline/readline.h>
#include <readline/history.h>

#define PACKAGE_NAME "ginger-admin"

using namespace std;

bool startsWith( const string& haystack, const string& needle ) {
    return (
        needle.length() <= haystack.length() && 
        equal( needle.begin(), needle.end(), haystack.begin() )
    );
}

static string& lttrim( string& s ) {
    s.erase(
        s.begin(),
        find_if( 
            s.begin(), 
            s.end(),
            not1( ptr_fun< int, int >( isspace ) )
        )
    );
    return s;
}

static const char * HELP = "help";

int main( int argc, char ** argv ) {
    //std::cerr<< "GNGREADLINE" << std::endl;
    std::string line;
    while ( !getline( std::cin, line ).eof() ) {
        if ( !line.empty() && line[0] == '!' ) {
            pid_t pid = fork();
            if ( pid == 0 ) {
                //  Ensure that stdout goes to the terminal.
                if ( dup2( STDERR_FILENO, STDOUT_FILENO ) < 0 ) {
                    perror( "gngreadline (dup failed)" );
                    exit( EXIT_FAILURE );
                }
                //cerr << "Forking! (child)" << endl;
                if ( line.size() == 1 ) {
                    execl( "/bin/bash", "/bin/bash", "-c", "/bin/bash -i", (char *)0 );
                    perror( "gngreadline (execl failed 1)" );
                } else {
                    execl( "/bin/bash", "/bin/bash", "-c", line.c_str() + 1, (char *)0 );
                    perror( "gngreadline (execl failed 2)" );
                }
                exit( EXIT_FAILURE );
            } else {
                int status;
                wait( &status );
            }
        } else if ( line == HELP ) {
            //  TODO: OPEN default web browser on the starting page.
            if ( fork() == 0 ) {
                execl( INSTALL_TOOL "/ginger-help", "ginger-help", (char *)0 );
                perror( "gngreadline" );
                return EXIT_FAILURE;
            }
        } else if ( startsWith( line, HELP ) ) {
            //  TODO: Open default web browser on the page with best matches to tags.
            //  Chop off the leading "help", trim whitespace & pass through.
            string topic( line, strlen( HELP ) );
            lttrim( topic );
            if ( fork() == 0 ) {
                execl( INSTALL_TOOL "/ginger-help", "ginger-help", topic.c_str(), (char *)0 );
                perror( "gngreadline" );
                return EXIT_FAILURE;
            }
        } else {
            std::cout << line << std::endl;
        }
    }
    //std::cerr << "End of file" << endl;
}