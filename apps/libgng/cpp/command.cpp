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

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

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

Command::~Command() {
	if ( this->should_wait_on_close && this->child_pid != 0 ) {
		int return_value_of_child;
		wait( &return_value_of_child );
	}
}

void Command::addArg( const std::string arg ) {
	this->args.push_back( arg );
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
	throw Unreachable( __FILE__, __LINE__ );
	
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
		cout << "Command: " << this->command << endl;
		for ( 
			vector< string >::iterator it = this->args.begin();
			it != this->args.end();
			++it
		) {
			cout << "Arg: " << *it << endl;
		}
	#endif
	
	
	int pipe_fd[ 2 ];
	pipe( pipe_fd );
	pid_t pid = fork();
	switch ( pid ) {
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
	throw Unreachable( __FILE__, __LINE__ );
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
	
	int pipe_fd[ 2 ];
	pipe( pipe_fd );
	pid_t pid = fork();
	switch ( pid ) {
		case -1: {	//	Something went wrong.
			throw Mishap( "Child process unexpectedly failed" ).culprit( "Command", command );
			break;
		}
		case 0: {	//	Child process - exec into command.
			//	Attach stdin to the read descriptor.
			dup2( pipe_fd[0], STDIN_FILENO );
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
			this->output_fd = pipe_fd[1];
			return;
		}
	}
	throw Unreachable( __FILE__, __LINE__ );
}

} // namespace
