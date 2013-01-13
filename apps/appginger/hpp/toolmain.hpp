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

#ifndef TOOL_MAIN_HPP
#define TOOL_MAIN_HPP

#include "rcep.hpp"

class ToolMain {
protected:
	const char * appname;
	AppContext context;

protected:
	void integrityChecks();
	std::string shellSafeName( const std::string & filename );
	int printLicense( const char * arg ) const;

protected:
	void executeLoadFileList( RCEP & rcep );
	void loadFileFromPackage( RCEP & rcep, Package * pkg, const std::string filename );
	void executeFileArguments( RCEP & rcep );
	void executeStdin( RCEP & rcep );
	void executeFile( RCEP & rcep, const std::string filename );
	void executeCommand( RCEP & rcep, const std::string command );
	void runFrom( RCEP & rcep, Ginger::MnxReader & gnx_read );


public:
	bool parseArgs( int argc, char **argv, char **envp );
	void printGPL( const char * start, const char * end ) const;
	const char * appName() { return this->appname; }

public:
	virtual int run() = 0;

public:
	ToolMain( const char * name ) : appname( name ) {}
	virtual ~ToolMain() {}
};


#endif
