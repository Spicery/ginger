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

#include <string.h>
#include <errno.h>
#include <getopt.h>
#include <unistd.h>
#include <stddef.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <stdio.h>
#include <unistd.h>
#include <sys/select.h>

// #define DEBUG 1
#define USE_GNU_READLINE 1
#ifdef USE_GNU_READLINE
    #include <readline/readline.h>
    #include <readline/history.h>
#endif

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

static void sendStdoutToTerminal() {
    //  Ensure that stdout goes to the terminal.
    if ( dup2( STDERR_FILENO, STDOUT_FILENO ) < 0 ) {
        perror( "gngreadline (dup failed)" );
        exit( EXIT_FAILURE );
    }    
}



// -- An interface to GNU READLINE ---------------------------------------------

#ifdef USE_GNU_READLINE

/* A static variable for holding the line. */
static char *line_read = (char *)NULL;

/* Read a string, and return a pointer to it.
   Returns NULL on EOF. */
char * gnu_readline() {
    /* 
        If the buffer has already been allocated,
        return the memory to the free pool. */
    if ( line_read ) {
        free( line_read );
        line_read = (char *)NULL;
    }

    /* Get a line from the user. */
    line_read = readline( "*** " );

    /* If the line has any text in it,
     save it on the history. */
    if ( line_read && *line_read ) {
        add_history( line_read );
    }

    return line_read;
}

#else
#endif

class MainLoop {
private:

    int from_subprocess;
    int to_subprocess;

public:

    MainLoop( int _f, int _t ) : from_subprocess( _f ), to_subprocess( _t ) {}

public:
    void writeLine( const std::string & line ) {
        int n = line.size();
        const char * buff = line.c_str();
        while ( n > 0 ) {
            int d = write( this->to_subprocess, buff, n );
            n -= d;
            buff += d;
        }
    }

public:
    bool getLine() {
        std::string line;
        
        // #ifdef USE_GNU_READLINE
            {
                char * ln = gnu_readline();
                if ( not ln ) {
                    return false;
                }
                line = ln;
            }
        // #else
        //     if ( getline( std::cin, line ).eof() ) {
        //         #ifdef DEBUG
        //             cerr << "End of terminal input" << endl;
        //         #endif
        //         return false;
        //     }
        // #endif

        if ( !line.empty() && line[0] == '!' ) {
            pid_t pid = fork();
            if ( pid == 0 ) {
                sendStdoutToTerminal();
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
            if ( fork() == 0 ) {
                sendStdoutToTerminal();
                execl( INSTALL_TOOL "/ginger-help", "ginger-help", (char *)0 );
                perror( "gngreadline" );
                exit( EXIT_FAILURE );
            }
        } else if ( startsWith( line, HELP ) ) {
            string topic( line, strlen( HELP ) );
            lttrim( topic );
            if ( fork() == 0 ) {
                sendStdoutToTerminal();
                execl( INSTALL_TOOL "/ginger-help", "ginger-help", topic.c_str(), (char *)0 );
                perror( "gngreadline" );
                exit( EXIT_FAILURE );
            }
        } else {
            line += '\n';
            this->writeLine( line );
        }
        return true;
    }

    bool relayOutputFromSubprocess() {
        char buff[ 4096 ];
        #ifdef DEBUG
            cerr << "Reading from subprocess" << endl;
        #endif
        int n = read( this->from_subprocess, buff, sizeof buff );
        #ifdef DEBUG
            cerr << "Read " << n << " bytes" << endl;
        #endif
        write( STDOUT_FILENO, buff, n );
        #ifdef DEBUG
            if ( n == 0 ) cerr << "End of subprocess output" << endl;
        #endif
        return n != 0;
    }

    bool doSelect() {
        
        fd_set rfds;
        FD_ZERO( &rfds );

        FD_SET( STDIN_FILENO, &rfds );
        FD_SET( this->from_subprocess, &rfds );

        #ifdef DEBUG 
            cerr << "Enter select" << endl;
        #endif
        int n = ( 
            select( 
                max( STDIN_FILENO, this->from_subprocess ) + 1,
                &rfds, NULL, NULL, NULL
            )
        );

        #ifdef DEBUG
            cerr << "Got select reply: " << n << endl;
        #endif

        if ( n < 0 ) exit( EXIT_FAILURE );

        if ( FD_ISSET( this->from_subprocess, &rfds ) ) {
            #ifdef DEBUG
                cerr << "Reply from subprocess" << endl;
            #endif
            return this->relayOutputFromSubprocess();
        } else {
            #ifdef DEBUG
                cerr << "Reply from terminal" << endl;
            #endif
            return this->getLine();
        }
    }

    void mainLoop() {
        // #ifdef USE_GNU_READLINE
        //     do {
        //         cout << "*** ";
        //         cout << flush;
        //         rl_set_prompt( "*** " );
        //         rl_on_new_line_with_prompt();
        //     } while ( doSelect() );
        // #else
            while ( doSelect() ) {
                //  Continue.
            }
        // #endif
    }

};




int main( int argc, char ** argv ) {

    int from_parent[ 2 ];
    int to_parent[ 2 ];

    if ( pipe( from_parent ) < 0 || pipe( to_parent ) ) {
        perror( "Cannot open pipe" );
        exit( EXIT_FAILURE );
    }

    pid_t pid = fork();
    if ( pid == 0 ) {
        //  Inside the child.
        
        #ifdef DEBUG
            cerr << "Forking " << endl;
            for ( char ** av = argv + 1; *av != NULL; av++ ) {
                cerr << "  Argument = " << *av << endl;
            }
        #endif
        
        //  from_parent is used to write from the parent to the child.
        //  So on the child end it is seen as a input - hence close the
        //  unwanted output.
        close( from_parent[ 1 ] );

        //  The other way round for the to_parent.
        close( to_parent[ 0 ] );

        //  Now attach these pipes to stdin and stdout.
        dup2( from_parent[ 0 ], STDIN_FILENO );
        dup2( to_parent[ 1 ], STDOUT_FILENO );

        execv( argv[ 1 ], argv + 1 );
        //  If execution reaches here an error has occurred.
        perror( "Cannot start subprocess" );
        exit( EXIT_FAILURE );
    } else {
        //  Inside the parent.

        //  from_parent is used to write from the parent to the child.
        //  So on the parent end it is seen as an output input - hence close 
        //  unwanted input.
        close( from_parent[ 0 ] );

        //  The other way round for the to_parent.
        close( to_parent[ 1 ] );

        MainLoop loop( to_parent[ 0 ], from_parent[ 1 ] );
        loop.mainLoop();
    }
    return EXIT_SUCCESS;
}
