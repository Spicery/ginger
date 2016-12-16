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

#include <string>
#include <iostream>
#include <sstream>

#include <stddef.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

//#include <boost/iostreams/stream_buffer.hpp>
//#include <boost/iostreams/device/file_descriptor.hpp>

#include "command.hpp"
#include "mishap.hpp"

using namespace std;
//using namespace boost;

//#define DBG_COMMAND

namespace Ginger {

Command::Command( const std::string command ) :
	command( command ),
	should_wait_on_close( false ),
	child_pid( 0 ),
	input_fd( -1 ),
	output_fd( -1 )
{
}

Command::Command( const char * prefix, const std::string & command ) :
	command( prefix ? std::string( prefix ) + command : command ),
	should_wait_on_close( false ),
	child_pid( 0 ),
	input_fd( -1 ),
	output_fd( -1 )
{
}


Command::~Command() {
	#ifdef DBG_COMMAND
		cerr << "~Command" << endl;
		cerr << "this->should_wait_on_close = " << this->should_wait_on_close << endl;
		cerr << "this->child_pid = " << this->child_pid << endl;
	#endif
	if ( this->should_wait_on_close && this->child_pid != 0 ) {
		int return_value_of_child;
		#ifdef DBG_COMMAND
			cerr << "Waiting ... " << this->child_pid << endl;
		#endif
		wait( &return_value_of_child );
		#ifdef DBG_COMMAND
			cerr << "OK!" << endl;
		#endif
	}
}

pid_t Command::getPid() {
	return this->child_pid;
}

void Command::interrupt() {
	#ifdef DBG_COMMAND
		cerr << "Killing " << this->child_pid << endl;
	#endif
	kill( this->child_pid, SIGKILL );
}

void Command::addArg( const std::string arg ) {
	#ifdef DBG_COMMAND
		cerr << "Adding arg: " << arg << endl;
		for ( 
			vector< string >::iterator it = this->args.begin();
			it != this->args.end();
			++it
		) {
			cerr << "  Current arg: " << *it << endl;
		}
	#endif
	this->args.push_back( arg );
}

void Command::wrap( const std::string cmd ) {
	#ifdef DBG_COMMAND
		cerr << "Wrapping replacing " << this->command << " with " << cmd << endl;
	#endif
	this->args.insert( this->args.begin(), this->command );
	this->command = cmd;
	#ifdef DBG_COMMAND
		cerr << "Old command " << this->args[ 0 ] << endl;
	#endif
}

void Command::fill( vector< char * > & argv ) {
	argv.push_back( const_cast< char * >( this->command.c_str() ) );
	for ( 
		vector< string >::iterator it = this->args.begin();
		it != this->args.end();
		++it
	) {
		argv.push_back( const_cast< char * >( it->c_str() ) );
	}
	argv.push_back( NULL );
}

const std::string Command::asPrintString() {
	std::stringstream str;
	str << this->command.c_str();
	for ( 
		vector< string >::iterator it = this->args.begin();
		it != this->args.end();
		++it
	) {
		str << " " << it->c_str();
	}
	return str.str();
}

void Command::runSilent() {
	vector< char * > arg_vector;
	this->fill( arg_vector );
	
	#ifdef DBG_COMMAND
		cout << "Command: " << this->command << endl;
		for ( 
			vector< string >::iterator it = this->args.begin();
			it != this->args.end();
			++it
		) {
			cout << "Arg: " << *it << endl;
		}
	#endif
	
	pid_t pid = fork();
	switch ( pid ) {
		case -1: {	//	Something went wrong.
			throw Mishap( "Child process unexpectedly failed" ).culprit( "Command", command );
			break;
		}
		case 0: {	//	Child process - exec into command.
			execv( this->command.c_str(), &arg_vector[0] );
			break;
		}
		default: {	// 	Parent process.
			pid_t stat_loc;
			wait( &stat_loc );
			return;
		}
	}
	throw UnreachableError();
}

/*
	This is how to convert a file descriptor to a output stream using
	Boost.
	
		int fd = cmd.runWithOutput();
		iostreams::file_descriptor_source fdsource( fd, iostreams::close_handle );
		fdistreambuf input_stream_buf( fdsource );
		istream input_stream( &input_stream_buf );

*/
int Command::runWithOutput() {

	vector< char * > arg_vector;
	this->fill( arg_vector );
	
	#ifdef DBG_COMMAND
		cerr << "Command: " << this->command << endl;
		int n = 1;
		for ( 
			vector< string >::iterator it = this->args.begin();
			it != this->args.end();
			++it, n += 1
		) {
			cerr << "Arg[" << n << "]: " << *it << endl;
		}
	#endif
	
	int pipe_fd[ 2 ];
	pipe( pipe_fd );
	this->child_pid = fork();
	switch ( this->child_pid ) {
		case -1: {	//	Something went wrong.
			throw Mishap( "Child process unexpectedly failed" ).culprit( "Command", command );
			break;
		}
		case 0: {	//	Child process - exec into command.
			//	Close the unused read descriptor.
			close( pipe_fd[0] );
			//	Attach stdout to the write descriptor.
			dup2( pipe_fd[1], STDOUT_FILENO );
			execv( this->command.c_str(), &arg_vector[0] );
			break;
		}
		default: {	// 	Parent process.
			this->should_wait_on_close = true;
			
			//	Close the unused write descriptor.
			close( pipe_fd[1] );
			
			this->input_fd = pipe_fd[0];
			return this->input_fd;
		}
	}
	throw UnreachableError();
}

void Command::runWithInputAndOutput() {

	vector< char * > arg_vector;
	this->fill( arg_vector );
	
	#ifdef DBG_COMMAND
		cout << "Command: " << this->command << endl;
		for ( 
			vector< string >::iterator it = this->args.begin();
			it != this->args.end();
			++it
		) {
			cout << "Arg: " << *it << endl;
		}
	#endif	
	
	//	In the child this is a read, in the parent it is write.
	int pipe_incoming_fd[ 2 ];
	
	//	In the child this is a write, in the parent it is a read.
	int pipe_outgoing_fd[ 2 ];
	
	pipe( pipe_outgoing_fd );
	pipe( pipe_incoming_fd );
	this->child_pid  = fork();
	switch ( this->child_pid ) {
		case -1: {	//	Something went wrong.
			throw Mishap( "Child process unexpectedly failed" ).culprit( "Command", command );
			break;
		}
		case 0: {	//	Child process - exec into command.
			//	Close the unused read descriptor.
			close( pipe_outgoing_fd[0] );
			//	Close the unused write descriptor.
			close( pipe_incoming_fd[1] );
			
			//	Attach stdin to the read descriptor.
			dup2( pipe_incoming_fd[0], STDIN_FILENO );
			//	Attach stdout to the write descriptor.
			dup2( pipe_outgoing_fd[1], STDOUT_FILENO );
			
			execv( this->command.c_str(), &arg_vector[0] );
			break;
		}
		default: {	// 	Parent process.
			this->should_wait_on_close = true;
			
			this->input_fd = pipe_outgoing_fd[0];
			this->output_fd = pipe_incoming_fd[1];
			
			//	Close the unused write descriptor.
			close( pipe_outgoing_fd[1] );
			
			// Close the unused read descriptor.
			close( pipe_incoming_fd[0] );
			return;
		}
	}
	throw UnreachableError();
}

void Command::readIntoStringStream( stringstream & prog ) {
	const int fd_in = this->getInputFD();
	for (;;) {
        static char buffer[ 1024 ];
        //  TODO: protect close with finally.
        int n = read( fd_in, buffer, sizeof( buffer ) );
        if ( n == 0 ) break;
        if ( n == -1 ) {
            if ( errno != EINTR ) {
                perror( "" );
                throw Ginger::Mishap( "Failed to read" );
            }
        } else if ( n > 0 ) {
            prog.write( buffer, n );
        }
    }
}

void Command::sendString( std::string input, std::stringstream & output ) {
	this->runWithInputAndOutput();

	FILE * f_out = fdopen( this->getOutputFD(), "w" );
	fputs( input.c_str(), f_out );
	fflush( f_out );
	fclose( f_out );
	
    this->readIntoStringStream( output );

	close( this->getInputFD() );
}

Ginger::SharedMnx Command::sendMnx( Ginger::SharedMnx mnx ) {
	this->runWithInputAndOutput();

	FILE * f_out = fdopen( this->getOutputFD(), "w" );
	
	mnx->frender( f_out );
	fprintf( f_out, "\n" );
	fflush( f_out );
	
    stringstream prog;
    this->readIntoStringStream( prog );

    Ginger::MnxReader reader( prog );
    Ginger::SharedMnx m = reader.readMnx();

	close( this->getInputFD() );
	fclose( f_out );

    return m;
}

} // namespace
