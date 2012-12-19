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

class ToolMain {
private:
	bool use_stdin;
	std::string input_file_name;
	bool gen_lnx;
	bool cstyle;
	const char * app_title;
	bool no_span;
	
public:
	ToolMain( bool cstyle, const char * title ) : cstyle( cstyle ), app_title( title ), no_span( false ) {}

public:
	std::string version() { return "0.2"; }
	void printGPL( const char * start, const char * end );
	void parseArgs( int argc, char **argv, char **envp );

private:
	void tokenise( FILE *in );
	void parse( FILE * in );
	void run( FILE * in );

public:
	int run();
};
