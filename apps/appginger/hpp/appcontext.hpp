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

#ifndef APP_CONTEXT_HPP
#define APP_CONTEXT_HPP

#include <string>
#include <vector>
#include <list>
#include <map>


#ifdef RUDECGI
	#include <rude/cgi.h>
#endif

#include "command.hpp"
#include "gson.hpp"
#include "xdgconfigfiles.hpp"

#include "gngversion.hpp"
#include "common.hpp"

#define APPGINGER_NAME		"appginger"
#define APPGINGER_VERSION	PACKAGE_VERSION
#define USER_SETTINGS_FILE	"settings.gson"

namespace Ginger {

class MachineClass;
class Package;

class PrintDetailLevel {
private:
	int print_level;
public:
	PrintDetailLevel() : print_level( 0 ) {}
public:
	int & level() { return this->print_level; }
	
	/** setBasic = non-silent */
	void setBasic() { this->print_level |= 0x1; }
	
	/** setChatty implies setBasic too */
	void setChatty() { this->print_level |= 0x3; }

	/** setTimed implies setChatty and hence setBasic too */
	void setTimed() { this->print_level |= 0x7; }

	bool isBasic() const { return ( this->print_level & 0x1 ) != 0; }

	bool isChatty() const { return ( this->print_level & 0x2 ) != 0; }

	bool isTimed() const { return ( this->print_level & 0x4 ) != 0; }

	bool isntSilent() const { return ( this->print_level ) != 0; }
};


const std::string RESULT_BULLET = "ResultBullet";
const std::string RESULT_HEADER = "ResultHeader";
const std::string RESULT_FOOTER = "ResultFooter";

class UserSettings {

private:
	std::string filename;
	bool initialised;
	Ginger::GSON settings;

public:
	UserSettings( const char * _filename ) :
		filename( _filename ),
		initialised( false )
	{
	}

	Ginger::GSON & getSettings() {
		if ( this->initialised ) return this->settings;

		Ginger::XDGConfigFiles config( this->filename.c_str() );
		if ( config.hasNext() ) {
			this->settings = Ginger::GSON::readSettingsFile( config.next() );
		} else {
			Ginger::GSONBuilder b;
			b.beginMap();
			b.endMap();
			this->settings = b.newGSON();
		}

		this->initialised = true;
		return this->settings;
	}

public:
	std::string stringSetting( const std::string & key, const std::string & def ) {
		Ginger::GSON value = this->getSettings().index( key );
		if ( not value || not value.isString() ) {
			return def;
		} else {
			return value.getString();
		}
	}
	
	std::string resultBullet() {
		return this->stringSetting( RESULT_BULLET, "" );
	}

	Ginger::GSON resultHeading( int n, const std::string & def ) {
		Ginger::GSON value = this->getSettings().index( RESULT_HEADER );
		if ( not value ) {
			return Ginger::GSON( new Ginger::StringGSONData( def ) );
		} else if ( value.isList() && not value.isEmpty() ) {
			if ( n < static_cast< int >( value.size() ) ) {
				return value.at( n );
			} else {
				return value.last();
			}
		} else {
			return value;
		}
	}

	//	@todo refactor to eliminate duplication
	Ginger::GSON resultFooter( int n, const std::string & def ) {
		Ginger::GSON value = this->getSettings().index( RESULT_FOOTER );
		if ( not value ) {
			return Ginger::GSON( new Ginger::StringGSONData( def ) );
		} else if ( value.isList() && not value.isEmpty() ) {
			if ( n < static_cast< int >( value.size() ) ) {
				return value.at( n );
			} else {
				return value.last();
			}
		} else {
			return value;
		}
	}


	//	@todo refactor to eliminate duplication
	Ginger::GSON resultFooter( int n ) {
		Ginger::GSON value = this->getSettings().index( RESULT_FOOTER );
		if ( not value ) {
			return Ginger::GSON( new Ginger::AbsentGSONData() );
		} else if ( value.isList() && not value.isEmpty() ) {
			if ( n < static_cast< int >( value.size() ) ) {
				return value.at( n );
			} else {
				return value.last();
			}
		} else {
			return value;
		}
	}



};


class AppContext {
public:
	enum {
		SCRIPT_MODE,
		SHELL_MODE,
		CGI_MODE
	}							mode;

private:
	std::string 				machine_impl_name;
	bool 						dbg_show_code;
	bool 						is_gctrace;
	std::list< std::string >	project_folder_list;
	std::list< std::string >    load_file_list;
	char **						envp;
	std::vector< std::string > 	arg_list;
	#ifdef RUDECGI
		rude::CGI * 			cgi;
	#endif
	bool						use_stdin;
	bool						use_fn2code;
	PrintDetailLevel			print_detail_level;
	bool						welcoming;
	std::string         		initial_syntax;
	std::string					interactive_package;
	UserSettings 				user_settings;

public:
	void setGCTrace( bool t ) { this->is_gctrace = t; }
	bool isGCTrace() { return this->is_gctrace; }
	void setMachineImplName( const std::string & n ) { this->machine_impl_name = n; }
	const std::string getMachineImplName() { return this->machine_impl_name; }
	const char * version() { return APPGINGER_VERSION; }
	void setShowCode( const bool b = true ) { this->dbg_show_code = b; }
	bool getShowCode() { return this->dbg_show_code; }
	std::list< std::string > & getProjectFolderList() { return this->project_folder_list; }
	void addProjectFolder( const std::string & folder );
	void addProjectFolder( const char * folder );
	void addLoadFile( const char * filename );
	std::list< std::string > & getLoadFileList() { return this->load_file_list; }
	char ** getEnvironmentVariables() { return this->envp; }
	void setEnvironmentVariables( char ** e ) { this->envp = e; }
	void addArgument( const char * s ) { this->arg_list.push_back( s ); }
	std::vector< std::string > & arguments() { return this->arg_list; }
	bool & useStdin() { return this->use_stdin; }

	PrintDetailLevel & printDetailLevel() { return this->print_detail_level; }

	const char* cgiValue( const char* fieldname );
	void initCgi();
	bool isCGIMode();
	void initShell();
	bool isShellMode();
	void initScript();
	bool isScriptMode();
	bool & welcomeBanner() { return this->welcoming; }

	#ifdef OLD_SHELL_ESCAPES_REQUIRED
		const std::string syntax( const bool interactively ); 
		const std::string syntax( const std::string & filename ); 
	#endif

	Ginger::Command stdinSyntaxCommand(); 
	Ginger::Command fileSyntaxCommand( const std::string & filename ); 
	void setSyntax( const std::string s ) { this->initial_syntax = s; }
	void setFn2Code( const bool b ) { this->use_fn2code = b; }
	const std::string getInteractivePackage() { return this->interactive_package; }
	void setInteractivePackage( const std::string & ip ) { this->interactive_package = ip; }
 	void showMeRuntimeInfo();

 	UserSettings & userSettings() { return this->user_settings; }

public:
	MachineClass * newMachine();
	Package * initInteractivePackage( MachineClass * vm );

public:
	AppContext() :
		//mode( InteractiveMode ),
		machine_impl_name( "sysfn" ),
		dbg_show_code( false ),
		//is_trapping_mishap( true ),
		is_gctrace( false ),
		use_stdin( false ),
		welcoming( true ),
		interactive_package( INTERACTIVE_PACKAGE ),
		user_settings( USER_SETTINGS_FILE )
	{
		//std::cout << "AppContext: " << interactive_package << std::endl;
		#ifdef RUDECGI	
			this->cgi = NULL;
		#endif
	}

	~AppContext() {
		//std::cout << "~Appcontext" << std::endl;
	}
};

} // namespace Ginger

#endif

