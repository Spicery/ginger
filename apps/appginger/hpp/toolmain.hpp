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

class ToolMain {
protected:
	const char * appname;
	AppContext context;

protected:
	std::string safeFileName( const std::string & filename );
	int printLicense( const char * arg ) const;
	void runAsCgi( const bool run_init_cgi );
	void mainLoop();

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
