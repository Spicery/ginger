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


#include "wellknownpaths.hpp"

#include <iostream>
#include <fstream>
#include <sstream>

//#include <cstdio>
#include <cstdlib>

#include <stddef.h>
#include <unistd.h>
#include <getopt.h>
#include <syslog.h>

#include "fileutils.hpp"
#include "common.hpp"
#include "rcep.hpp"
#include "appcontext.hpp"
#include "mishap.hpp"
#include "sys.hpp"
#include "machine.hpp"
#include "machine1.hpp"
#include "machine2.hpp"
#include "machine3.hpp"
#include "machine4.hpp"
#include "enginefactory.hpp"

using namespace std;

namespace Ginger {

Package * AppContext::initInteractivePackage( MachineClass * vm ) {
    const std::string name( this->getInteractivePackage() );
    Package * interactive_pkg = vm->getPackage( name );
    return interactive_pkg;
}

MachineClass * AppContext::newMachine() {
    Ginger::EngineFactory * e = Ginger::EngineFactoryRegistration::findMatch( this->machine_impl_name );
    if ( e == NULL ) throw Ginger::Mishap( "Unavailable implementation" ).culprit( "Implementation#", this->machine_impl_name );
    return e->newEngine( this );    
}

void AppContext::addLoadFile( const char * load_file_name ) {
    this->load_file_list.push_back( load_file_name );
}

void AppContext::addProjectFolder( const std::string & folder ) {
	this->project_folder_list.push_back( folder );
}

void AppContext::addProjectFolder( const char * folder ) { 
	this->project_folder_list.push_back( folder );
}

void AppContext::initCgi() {
    this->mode = CGI_MODE;
#ifdef RUDECGI
	this->cgi = new rude::CGI();
#endif
}

const char* AppContext::cgiValue( const char * fieldname ) {
    #ifdef RUDECGI
    	const char * answer = this->cgi == NULL ? "" : this->cgi->value( fieldname );
    	return answer;
    #else
    	return "";
    #endif
}

/*
 * This is the command that is run to acquire and parse user input into GNX.
 */
Ginger::Command AppContext::stdinSyntaxCommand() { 
    Ginger::Command cmd( ( USESNAP ? getenv( "SNAP" ) : nullptr ), FILE2GNX );
    if ( not this->initial_syntax.empty() ) {
        cmd.addArg( "-g" );
        cmd.addArg( this->initial_syntax );
    }
    return cmd;
}

/*
 * This is the command that is run to read and parse files into GNX.
 */
Ginger::Command AppContext::fileSyntaxCommand( const std::string & filename ) { 
    // cerr << "NON-INTERACTIVELY" << endl;
    Ginger::Command cmd( ( USESNAP ? getenv( "SNAP" ) : nullptr ), FILE2GNX );
    if ( not this->initial_syntax.empty() ) {
        cmd.addArg( "-g" );
        cmd.addArg( this->initial_syntax );
    }
    cmd.addArg( filename );
    return cmd;
}



class ReSTRuntimeInfoDriver {
public:
    void startInfo() {}
    void endInfo() {}

    void startSection( const string topic ) {
        cout << topic << endl;
        this->underline( '-', topic );
    }

    void endSection() {
        cout << endl;
    }

    void underline( const char u, const string topic ) {
        this->underline( u, topic.size() );
    }

    void startFreeText() {
        cout << endl << "::" << endl << endl;
    }

    void endFreeText() {
        cout << endl;
    }

    void showLine( const string line ) {
        cout << "\t" << line << endl;
    }

    void underline( const char u, const int n ) {
        for ( int i = 0; i < n; i++ ) {
            cout << u;
        }
        cout << endl;
    }

    void show( const string fieldname, const string value ) {
        this->show( fieldname.c_str(), value.c_str() );
    }
    void show( const int n, const string value ) {
        stringstream out;
        out << n;
        this->show( out.str(), value );
    }
    
    void show( const char * fieldname, const int n ) {
        stringstream out;
        out << n;
        this->show( fieldname, out.str().c_str() );
    }

    void show( const char * fieldname, const char * value ) {
        cout << "* " << fieldname << ": " << value << endl;
    }

    void showEnabled( const char * fieldname, const bool enabled ) {
        this->show( fieldname, enabled ? "enabled" : "disabled" );
    }

    void showEntry( const string e ) {
        cout << "* " << e << endl;
    }

};

void AppContext::initShell() {
    this->mode = SHELL_MODE;
}

void AppContext::initScript() {
    this->mode = SCRIPT_MODE;
}

bool AppContext::isCGIMode() {
    return this->mode == CGI_MODE;
}

bool AppContext::isScriptMode() {
    return this->mode == SCRIPT_MODE;
}

bool AppContext::isShellMode() {
    return this->mode == SHELL_MODE;
}

/**
 *  The main purpose of this function is to display information that will 
 *  help programmers understand the launch configuration and dynamic 
 *  environment of the Ginger virtual machine. Other information that is
 *  displayed should be kept short and link to other more detailed
 *  resources.
 */
void AppContext::showMeRuntimeInfo() {
    ReSTRuntimeInfoDriver d;
    d.startInfo();

    d.startSection( "Application Environment" );
    d.show( 
        "Startup mode", 
        this->isScriptMode() ? "Script" :
        this->isShellMode() ? "Shell" :
        this->isCGIMode() ? "CGI" : 
        "Other" 
    );
    d.endSection();

    d.startSection( "Main" );
    d.show( "Ginger version", this->version() );
    d.show( "VM Engine ID", this->machine_impl_name );
    d.showEnabled( "Garbage collection tracing", this->is_gctrace );
    d.showEnabled( "Code generation tracing", this->getShowCode() );
    d.show( "Reading standard input", this->use_stdin );
    d.show( "Level of print detail", this->print_detail_level.level() );
    d.showEnabled( "Showing welcome banner", this->welcoming );
    d.show( "Interactive package", this->interactive_package );
    d.show( "Default syntax", this->initial_syntax );
    d.endSection();

    d.startSection( "Project folders" );
    if ( this->project_folder_list.empty() ) {
        d.showEntry( "(None)" );
    } else {
        int count = 0;
        for ( 
            std::list< std::string >::iterator it = this->project_folder_list.begin();
            it != this->project_folder_list.end();
            ++it
        ) {
            d.show( ++count, *it );
        }
    }
    d.endSection();

    d.startSection( "Environment Variables" );
    {
        char ** envp = this->envp;
        while ( *envp != NULL ) {
            char * e = *envp++;
            d.showEntry( e );
        }
    }
    d.endSection();

    d.startSection( "Installation Folders" );
    d.show( "Executables folder", INSTALL_BIN );
    d.show( "Tools folder", INSTALL_TOOL );
    d.show( "Resources library (share) folder", INSTALL_LIB );
    d.endSection();

    d.startSection( "Legal" );
    d.show( "Copyright", "Copyright (c) 2010  Stephen Leach <sfkleach@gmail.com>" );
    d.show( "License", "GNU GENERAL PUBLIC LICENSE, Version 3, 29 June 2007 <http://www.gnu.org/licenses/gpl.txt>" );
    d.endSection();

    d.endInfo();
}

} // namespace Ginger
