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

#ifndef APP_CONTEXT_HPP
#define APP_CONTEXT_HPP

#include <string>
#include <vector>
#include <list>

#ifdef RUDECGI
	#include <rude/cgi.h>
#endif


#include "gngversion.hpp"

#define APPGINGER_NAME		"appginger"
#define APPGINGER_VERSION	PACKAGE_VERSION

class MachineClass;
class Package;

class AppContext {
private:
	int 						machine_impl_num;
	bool 						dbg_show_code;
	bool 						is_gctrace;
	std::list< std::string >	project_folder_list;
	char **						envp;
	std::vector< std::string > 	arg_list;
	#ifdef RUDECGI
		rude::CGI * 			cgi;
	#endif
	bool						use_stdin;
	int							print_level;
	bool						welcoming;
	std::string         		initial_syntax;

public:
	void setGCTrace( bool t ) { this->is_gctrace = t; }
	bool isGCTrace() { return this->is_gctrace; }
	void setMachineImplNum( const int n ) { this->machine_impl_num = n; }
	int getMachineImplNum() { return this->machine_impl_num; }
	const char * version() { return APPGINGER_VERSION; }
	void setShowCode() { this->dbg_show_code = true; }
	bool getShowCode() { return this->dbg_show_code; }
	std::list< std::string > & getProjectFolderList() { return this->project_folder_list; }
	void addProjectFolder( const std::string & folder );
	void addProjectFolder( const char * folder );
	char ** getEnvironmentVariables() { return this->envp; }
	void setEnvironmentVariables( char ** e ) { this->envp = e; }
	void addArgument( const char * s ) { this->arg_list.push_back( s ); }
	std::vector< std::string > & arguments() { return this->arg_list; }
	bool & useStdin() { return this->use_stdin; }
	int & printDetailLevel() { return this->print_level; }
	const char* cgiValue( const char* fieldname );
	void initCgi();
	bool & welcomeBanner() { return this->welcoming; }
	const char * syntax(); 
	const char * syntax( const std::string & filename ); 
	void setSyntax( const std::string s ) { this->initial_syntax = s; }

public:
	MachineClass * newMachine();
	Package * initInteractivePackage( MachineClass * vm );

public:
	AppContext() :
		//mode( InteractiveMode ),
		machine_impl_num( 1 ),
		dbg_show_code( false ),
		//is_trapping_mishap( true ),
		is_gctrace( false ),
		use_stdin( false ),
		print_level( 0 ),
		welcoming( true )
	{
		#ifdef RUDECGI	
			this->cgi = NULL;
		#endif
	}

	~AppContext() {}
};

#endif

