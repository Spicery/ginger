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

#include <readline/readline.h>
#include <readline/history.h>

#define PACKAGE_NAME "ginger-admin"

using namespace std;


/* A static variable for holding the line. */
static char *line_read = (char *)NULL;

//  Read a string, and return a pointer to it. Returns NULL on EOF.
static char * getline( const char * prompt, bool reask_if_empty = true ) {
    //  If the buffer has already been allocated, return the memory to the free pool.
    if ( line_read ) {
        free( line_read );
        line_read = (char *)NULL;
    }

    // Get a line from the user.
    line_read = readline( prompt );

    if ( line_read == NULL ) {
        return NULL;
    } else if ( *line_read != '\0' ) {
        //  If the line has any text in it, save it on the history. 
        add_history( line_read );
        return line_read;
    } else if ( reask_if_empty ) {
        return getline( prompt, reask_if_empty );
    } else {
        //  No text on it, so don't add to history but we are required to
        //  return it if reasking is set to false.
        return line_read;
    }
}




class Interaction {
public:
    void interact() {
        for (;;) {
            const char * s = getline( "Question? " );
            if ( not s ) break;
            
            cout << s << endl;
        }
    }
};

static void printUsage() {
    cout << "Usage : ginger-admin [options]" << endl;
    cout << "OPTION                SUMMARY" << endl;
    cout << "--help                print out more detailed help" << endl;
    cout << "--license             print out license information and exit" << endl;
    cout << "--version             print out version information and exit" << endl;
    cout << endl;
}

/*
    This is the structure of struct option, which does not seem to be
    especially well documented. Included for handy reference.
    struct option {
        const char *name;   // option name
        int has_arg;        // 0 = no arg, 1 = mandatory arg, 2 = optional arg
        int *flag;          // variable to return result or NULL
        int val;            // code to return (when flag is null)
                            //  typically short option code
*/

extern char * optarg;
static struct option long_options[] =
    {
        { "help",           optional_argument,      0, 'H' },
        { "license",        optional_argument,      0, 'L' },
        { "version",        no_argument,            0, 'V' },
        { 0, 0, 0, 0 }
    };

class Task {
public:
    virtual bool acceptsMoreArgs() { return true; }

    virtual const char * option() {
        return "generic";
    }

    void setOptArg( const char * optarg ) {
        if ( optarg != NULL ) {
            this->setArg( optarg );
        }
    }

    virtual void setArg( const char * optarg ) {
        cerr << "Internal error! Should not be able to set this option: " << optarg << endl;
        exit( EXIT_FAILURE );
    }

    virtual int run() = 0;

public:
    virtual ~Task() {}
};

typedef std::tr1::shared_ptr< Task > TaskPtr;

class EndTask : public Task {
public:
    virtual bool acceptsMoreArgs() { return false; }
public:
    virtual ~EndTask() {}
};

class FailTask : public EndTask {
private:
    string msg;

public:
    int run() {
        cerr << "Error: " << this->msg << endl;
        cerr << "(Try --help for a usage summary)" << endl;
        return EXIT_FAILURE;
    }

public:
    FailTask( const string m ) : msg( m ) {}
    virtual ~FailTask() {}
};

class UsageTask : public EndTask {
private:
    int status;

public:
    int run() {
        printUsage();
        return this->status;
    }

public:
    UsageTask( int s ) : status( s ) {}
    UsageTask() : status( EXIT_SUCCESS ) {}
    virtual ~UsageTask() {}
};

class InteractiveTask : public Task {
public:
    const char * option() { return "interactive"; }

    int run() {
        Interaction interaction;
        interaction.interact();
        return EXIT_SUCCESS; 
    }

public:
    virtual ~InteractiveTask() {}
};


class VersionTask : public EndTask {
public:
    const char * option() { return "version"; }

    int run() {
        cout << PACKAGE_NAME << ": " << VERSION << ", Copyright (c) 2010  Stephen Leach" << endl;
        return EXIT_SUCCESS;
    }

public:
    virtual ~VersionTask() {}
};

static const char * LICENSE_FILE = ( INSTALL_LIB "/COPYING" );

class PrintLicenseTask : public EndTask {
private:
    const char * topic;

    void printGPL( const char * start, const char * end ) const {
        bool printing = false;
        ifstream license( LICENSE_FILE );
        std::string line;
        while ( getline( license, line ) )  {
            if ( !printing && ( start == NULL || line.find( start ) != string::npos ) ) {
                printing = true;
            } else if ( printing && end != NULL && line.find( end ) != string::npos ) {
                printing = false;
            }
            if ( printing ) {
                std::cout << line << std::endl;
            }
        }
    }

    int printLicense( const char * arg ) const {
        if ( arg == NULL || std::string( arg ) == std::string( "all" ) ) {
            this->printGPL( NULL, NULL );
        } else if ( std::string( arg ) == std::string( "warranty" ) ) {
            this->printGPL( "Disclaimer of Warranty.", "Limitation of Liability." );                 
        } else if ( std::string( arg ) == std::string( "conditions" ) ) {
            this->printGPL( "TERMS AND CONDITIONS", "END OF TERMS AND CONDITIONS" );
        } else {
            std::cerr << "Unknown license option: " << arg << std::endl;
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }


public:
    const char * option() { return "license"; }

    void setArg( const char * optarg ) {
        topic = optarg;
    }

    int run() {
        this->printLicense( this->topic );
        return EXIT_SUCCESS;
    }

public:
    PrintLicenseTask() : topic( NULL ) {}

    virtual ~PrintLicenseTask() {}
};    

class GngReadLine {
public:
    TaskPtr parseArgs( int argc, char ** argv ) {
        TaskPtr mode( new InteractiveTask() );
        while ( mode->acceptsMoreArgs() ) {
            int option_index = 0;
            int c = getopt_long( argc, argv, "H::L::V", long_options, &option_index );
            //cerr << "Got c = " << c << endl;
            if ( c == -1 ) break;
            switch ( c ) {
                case 'H': {
                    //  --help [TOPIC]
                    mode = TaskPtr( new UsageTask() );
                    break;
                }
                case 'L': {
                    //  --license [TOPIC]
                    mode = TaskPtr( new PrintLicenseTask() );
                    mode->setOptArg( optarg );
                    break;
                }
                case 'V': {
                    //  --version
                    mode = TaskPtr( new VersionTask() );
                    break;
                }
                case '?': {
                    return TaskPtr( new UsageTask( EXIT_FAILURE ) );
                }
                default: {
                    return TaskPtr( new FailTask( "Internal error! Option string incorrect" ) );
                }
            }
        }
        return mode;
    }

    int main( int argc, char ** argv ) {
        return this->parseArgs( argc, argv )->run();
    }

};

int main( int argc, char ** argv ) {
    GngReadLine gngreadline;
    return gngreadline.main( argc, argv );
}