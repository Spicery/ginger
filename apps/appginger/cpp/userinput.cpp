/******************************************************************************\
    Copyright (c) 2010 Stephen Leach. AppGinger is distributed under the terms 
    of the GNU General Public License. This file is part of Ginger.

    Ginger is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Ginger is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Ginger.  If not, see <http://www.gnu.org/licenses/>.
\******************************************************************************/

#include <sstream> 
#include <algorithm>

#include <unistd.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "mnx.hpp"
#include "gnxconstants.hpp"
#include "userinput.hpp"

namespace Ginger {
using namespace std;

static bool startsWith( const string& haystack, const string& needle ) {
    return (
        needle.length() <= haystack.length() && 
        equal( needle.begin(), needle.end(), haystack.begin() )
    );
}

static string & lttrim( string& s ) {
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

static void pause() {
    int status;
    wait( &status );
    usleep( 100000 );
}

static std::string getSnapPath( const char * path ) {
    std::string cmd;
    if ( USESNAP ) {
        const char * snap = getenv( "SNAP" );
        if ( snap ) {
            cmd += snap;
        }
    }
    cmd += path;
    return cmd;
}

static std::string getGingerHelpSnapPath() {
    return getSnapPath( INSTALL_TOOL "/ginger-help" );
}

static void getHelp() {
    if ( fork() == 0 ) {
        const std::string cmd( getGingerHelpSnapPath() );
        execl( cmd.c_str(), cmd.c_str(), (char *)0 );
        perror( "ginger" );
        exit( EXIT_FAILURE );
    } else {
        pause();
    }
}

static void getHelp( std::string & line ) {
    string topic( line, strlen( "help " ) );
    lttrim( topic );
    if ( fork() == 0 ) {
        const std::string cmd( getGingerHelpSnapPath() );
        execl( cmd.c_str(), cmd.c_str(), topic.c_str(), (char *)0 );
        perror( "ginger" );
        exit( EXIT_FAILURE );
    } else {
        pause();
    }
}

static void invoke( std::string line ) {
    pid_t pid = fork();
    if ( pid == 0 ) {
        if ( line.size() == 1 ) {
            execl( "/bin/bash", "/bin/bash", "-c", "/bin/bash -i", (char *)0 );
            perror( "ginger (execl failed 1)" );
        } else {
            execl( "/bin/bash", "/bin/bash", "-c", line.c_str() + 1, (char *)0 );
            perror( "ginger (execl failed 2)" );
        }
        exit( EXIT_FAILURE );
    } else {
        pause();    
    }
}

// Read a string, and return a pointer to it.
// Returns NULL on EOF.
char * UserInput::gnu_readline() {
    static char *line_read = nullptr;  // Holds the line.
    for (;;) {
        // If the buffer has already been allocated,
        // return the memory to the free pool.
        if ( line_read ) {
            free( line_read );
            line_read = nullptr;
        }

        // Get a line from the user.
        line_read = readline( this->prompt.c_str() );
        if ( not line_read ) return line_read;
        
        // If the line has any text in it, save it on the history.
        if ( *line_read != '\0' ) {
            add_history( line_read );
        }

        std::string line( line_read );

        if ( line_read[0] == '!' ) {
            invoke( line_read );
            continue;
        } 

        if ( line == "help" ) {
            getHelp();
            continue;
        }
        if ( startsWith( line, "help " ) ) {
            getHelp( line );
            continue;
        }

        return line_read;
    }
}

Ginger::SharedMnx UserInput::nextMnx() {
    this->setPrompt( this->buffer.empty() ? ">-> " : "... " );
    char * line = gnu_readline();
    if ( line == nullptr ) {
        cout << endl;   //  We need to emit a new line at end of input because readline itself doesn't.
        return shared_ptr< Ginger::Mnx >();
    }
    // cerr << "BEFORE: " << line << endl;
    this->buffer += line;
    this->buffer += "\n";
    // cerr << "BUFFER: " << this->buffer << endl;
    stringstream prog;
    this->cmd.sendString( buffer, prog );
    // cerr << "PROG  : " << prog.str() << endl;
    Ginger::MnxReader reader( prog );
    Ginger::MnxBuilder builder;
    builder.start( GNX_SEQ );
    for (;;) {
        Ginger::SharedMnx m = reader.readMnx();
        if ( not m ) break;
        builder.add( m );
        if ( m->hasName( GNX_PROBLEM ) and m->hasAttribute( GNX_PROBLEM_CATEGORY, "CE" ) ) {
            return make_shared< Mnx >( GNX_SEQ );
        }
    }
    builder.end();
    this->buffer.clear();
    return builder.build();
}

}
