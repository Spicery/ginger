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

#ifndef COMMAND_HPP
#define COMMAND_HPP

#include <string>
#include <vector>
#include <sstream>

#include "mnx.hpp"

namespace Ginger {

class Command {
protected:
	std::string 						command;
	std::vector< std::string > 			args;
	bool								should_wait_on_close;
	pid_t 								child_pid;
	int									input_fd;
	int									output_fd;
	
private:
	void fill( std::vector< char * > & argv );
	void readIntoStringStream( std::stringstream & prog );
	
public:
	void addArg( const std::string arg );
	void wrap( const std::string arg );
	int getInputFD() { return this->input_fd; }
	int getOutputFD() { return this->output_fd; }
	const std::string asPrintString();
	pid_t getPid();
	
public:
	int runWithOutput();
	void runWithInputAndOutput();
	void runSilent();
	void interrupt();
	Ginger::SharedMnx sendMnx( Ginger::SharedMnx );
	void sendString( std::string, std::stringstream & );
	
public:
	Command( const char * prefix, const std::string & command );
	Command( const std::string command = "" );
	~Command();
};

}

#endif

