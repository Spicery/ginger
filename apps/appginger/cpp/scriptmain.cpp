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
#include <sstream>
#include <cstdlib>

#include <stddef.h>
#include <syslog.h>
#include <sys/errno.h>


#include <ext/stdio_filebuf.h> // __gnu_cxx::stdio_filebuf

#include "mnx.hpp"
#include "mishap.hpp"
#include "command.hpp"

#include "rcep.hpp"
#include "appcontext.hpp"
#include "toolmain.hpp"

#define APP_TITLE "ginger-script"

#include "wellknownpaths.hpp"

using namespace std;

class ScriptMain : public ToolMain {
private:
	void runFrom( RCEP & rcep, Ginger::MnxReader & gnx_read ) {
		for (;;) {
			shared< Ginger::Mnx > m = gnx_read.readMnx();
			if ( not m ) break;
			rcep.execGnx( m, cout );
			rcep.printResults( cout, 0 );
		}			
	}

	void executeCommand( RCEP & rcep, const string command ) {
		//	cerr << "Command so far: " << command << endl;
		FILE * gnxfp = popen( command.c_str(), "r" );
		if ( gnxfp == NULL ) {
			throw Ginger::Mishap( "Failed to translate input" );
		}
		// ... open the file, with whatever, pipes or who-knows ...
		// let's build a buffer from the FILE* descriptor ...
		__gnu_cxx::stdio_filebuf<char> pipe_buf( gnxfp, ios_base::in );
		
		// there we are, a regular istream is build upon the buffer.
		istream stream_pipe_in( &pipe_buf );
		
		while ( rcep.unsafe_read_comp_exec_print( stream_pipe_in, std::cout ) ) {}

		pclose( gnxfp );	
	}

	void executeFile( RCEP & rcep, const string filename ) {
		stringstream commstream;
		//	tail is 1-indexed!
		commstream << this->context.syntax( filename ) << " < " << shellSafeName( filename );
		this->executeCommand( rcep, commstream.str() );
	}

	void executeStdin( RCEP & rcep ) {
		this->executeCommand( rcep, this->context.syntax() );
	}

	void loadFileFromPackage( RCEP & rcep, Package * pkg, const std::string filename ) {
		Ginger::Command cmd( FETCHGNX );
		cmd.addArg( "-X" );
		{
			list< string > & folders = rcep.getMachine()->getAppContext().getProjectFolderList();
			for ( 
				list< string >::iterator it = folders.begin();
				it != folders.end();
				++it
			) {
				cmd.addArg( "-j" );
				cmd.addArg( *it );
			}
		}

		Ginger::MnxBuilder qb;
		qb.start( "fetch.load.file" );
		qb.put( "pkg.name", pkg->getTitle() );
		qb.put( "load.file", filename );
		qb.end();
		shared< Ginger::Mnx > query( qb.build() );

		#ifdef DBG_SCRIPT_MAIN
			cerr << "scriptmain asking for loadfile, using fetchgnx -X" << endl;
			cerr << "  [[";
			query->render( cerr );
			cerr << "]]" << endl;
		#endif	

		cmd.runWithInputAndOutput();
		int fd = cmd.getInputFD();   
		FILE * foutd = fdopen( cmd.getOutputFD(), "w" );
		query->frender( foutd );
		fflush( foutd );

		stringstream prog;
		for (;;) {
			static char buffer[ 1024 ];
			//	TODO: protect close with finally.
			int n = read( fd, buffer, sizeof( buffer ) );
			if ( n == 0 ) break;
			if ( n == -1 ) {
				if ( errno != EINTR ) {
					perror( "Script main loading file" );
					throw Ginger::Mishap( "Failed to read" );
				}
			} else if ( n > 0 ) {
				prog.write( buffer, n );
			}
		}

		//	TODO: protect close with finally.
		fclose( foutd );

		#ifdef DBG_SCRIPT_MAIN
			cerr << "  [[" << prog.str() << "]]" << endl;
		#endif
		while ( rcep.unsafe_read_comp_exec_print( prog, std::cout ) ) {}
	}
	
public:
	int run() {		
		MachineClass * vm = this->context.newMachine();
		Package * interactive_pkg = this->context.initInteractivePackage( vm );
		RCEP rcep( interactive_pkg );

		list< string > & load_files = this->context.getLoadFileList();
		for ( list< string >::iterator it = load_files.begin(); it != load_files.end(); ++it ) {
			this->loadFileFromPackage( rcep, interactive_pkg, *it );
		}
			
		vector< string > & args = this->context.arguments();
		for ( vector< string >::iterator it = args.begin(); it != args.end(); ++it ) {
			this->executeFile( rcep, *it );
		}
		if ( this->context.useStdin() ) {
			this->executeStdin( rcep );
		}
	    return EXIT_SUCCESS;
	}

public:
	ScriptMain( const char * name ) : ToolMain( name ) {
		this->context.printDetailLevel() = 1;
	}
	
	virtual ~ScriptMain() {}
};

int main( int argc, char **argv, char **envp ) {
	openlog( APP_TITLE, 0, LOG_LOCAL2 );
	setlogmask( LOG_UPTO( LOG_INFO ) );
	try {
		ScriptMain main( APP_TITLE );
		return main.parseArgs( argc, argv, envp ) ? main.run() : EXIT_SUCCESS;
	} catch ( Ginger::Problem & e ) {
		e.report();
		return EXIT_FAILURE;
	}
}
