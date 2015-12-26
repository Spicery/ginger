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
#include <iostream>
#include <vector>
#include <sstream>
#include <string>

#include <stddef.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>


#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>

#include "mnx.hpp"

#include "importinfo.hpp"
#include "search.hpp"
#include "mishap.hpp"
#include "folderscan.hpp"
#include "packagecache.hpp"
#include "resolvervisitor.hpp"

#define BROWSE_INFO 	"browse.info"
#define VAR 			"var.info"
#define VAR_NAME 		"var.name"
#define PKG 			"pkg.info"
#define PKG_NAME 		"pkg.name"
#define ENC_PKG 		"enc.pkg"
#define DEF_PKG 		"def.pkg"
#define PATH_SEPARATOR 	"/"

using namespace std;

//#define DBG_SEARCH

#define FILE2GNX "file2gnx"

static void renderText( std::ostream & out, const std::string & str ) {
	for ( std::string::const_iterator it = str.begin(); it != str.end(); ++it ) {
		const unsigned char ch = *it;
		if ( ch == '<' ) {
			out << "&lt;";
		} else if ( ch == '>' ) {
			out << "&gt;";
		} else if ( ch == '&' ) {
			out << "&amp;";
		} else if ( 32 <= ch && ch < 127 ) {
			out << ch;
		} else {
			out << "&#" << (int)ch << ";";
		}
	}
}

static void renderText( const std::string & str ) {
	renderText( cout, str );
}

Search::Search( vector< string > & project_folders, const bool undefined_allowed ) :
	project_cache( ProjectCache( this, project_folders ) ),
	undefined_allowed( undefined_allowed )
{
}

Search::~Search() {
}

//
//	Insecure. We need to do this more neatly. It would be best if common2gnx
//	and lisp2gnx could handle being passed a filename as an argument. This
//	would be both more secure and efficient.
//
static void run( string command, string pathname, ostream & out ) {
	//cout << "running " << command << " " << pathname << endl;
	int pipe_fd[ 2 ];
	const char * cmd = command.c_str();
	pipe( pipe_fd );
	pid_t pid = fork();
	switch ( pid ) {
		case -1: {	//	Something went wrong.
			throw Mishap( "Child process unexpectedly failed" ).culprit( "Command", command ).culprit( "Argument", pathname );
			break;
		}
		case 0: {	//	Child process - exec into command.
			//	Close the unused read descriptor.
			close( pipe_fd[0] );
			//	Attach stdout to the write descriptor.
			dup2( pipe_fd[1], STDOUT_FILENO );
			execl( cmd, cmd, pathname.c_str(), NULL );
			break;
		}
		default: {	// 	Parent process.
			//	Close the unused write descriptor.
			close( pipe_fd[1] );
			
			//	Read from the read descriptor until exhausted.
			const int input_fd = pipe_fd[0];
			static char buf[ 1024 ];
			for (;;) {
				ssize_t n = read( input_fd, buf, sizeof( buf ) );
				if ( n == 0 ) break;
				//write( STDOUT_FILENO, buf, n );	//	Use this command if you want faster output only to stdout.
				out.write( buf, n );				//	This is more general.
			}
			
			int return_value_of_child;
			wait( &return_value_of_child );
			break;
		}
	}
}

void Search::cacheDefinitionFile( PackageCache * c, const string & name, const string & pathname ) {
	ostringstream out;
	run( INSTALL_TOOL PATH_SEPARATOR FILE2GNX, pathname, out );
	cout << out.str();
}

static bool tagMatches( ImportInfo & imp, VarInfo * vinfo ) {
	return imp.matchTags()->isntEmptyIntersection( vinfo->varInfoTags() );
}

void Search::printAbsoluteDefinition( PackageCache * x, const string name ) {
	VarInfo * vfile = x->absoluteVarInfo( name );
	if ( vfile != NULL ) {
		this->cacheDefinitionFile( x, name, vfile->getPathName() );
	} else {
		throw  (
			Mishap( "Cannot find variable" ).
			culprit( "Variable", name ).
			culprit( "Package", x->getPackageName() )
		);
	}
}

static ImportInfo * importWithAlias( PackageCache * x, const string alias ) {
	vector< ImportInfo > & imports = x->importVector();
	for ( 
		vector< ImportInfo >::iterator it = imports.begin();
		it != imports.end();
		++it
	) {
		ImportInfo & imp = *it;
		if ( imp.hasAlias( alias ) ) return &imp;
	}
	return NULL;
}



static void outputUnqualified( const string & enc_pkg_name, const string & name, const string & def_pkg ) {
	cout << "<resolve " ENC_PKG "=\"";
	renderText( enc_pkg_name );
	cout << "\" name=\"";
	renderText( name );
	cout << "\" " DEF_PKG "=\"";
	renderText( def_pkg );
	cout << "\" />" << endl;
}



static void outputQualified( const string & enc_pkg_name, const string & alias_name, const string & var_name, const string & def_pkg ) {
	cout << "<resolve " ENC_PKG "=\"";
	renderText( enc_pkg_name );
	cout << "\" alias=\"";
	renderText( alias_name );
	cout << "\" name=\"";
	renderText( var_name );
	cout << "\" " DEF_PKG "=\"";
	renderText( def_pkg );
	cout << "\" />" << endl;	
}

string Search::findDefPkgUnqualified( const string & enc_pkg_name, const string & name ) {
	//cerr << "UNQUALIFIED" << endl;
	PackageCache * x = this->project_cache.fetchPackageCache( enc_pkg_name );
	VarInfo * vfile = x->absoluteVarInfo( name );
	if ( vfile != NULL ) {
		return enc_pkg_name;
		//this->cacheDefinitionFile( x, name, vfile->getPathName() );
	} else {
		vector< ImportInfo > & imports = x->importVector();
		PackageCache * def_pkg = NULL;
		for ( 
			vector< ImportInfo >::iterator it = imports.begin();
			it != imports.end();
			++it
		) {
			ImportInfo & imp = *it;
			PackageCache * c = this->project_cache.fetchPackageCache( imp.getFrom() );
			VarInfo * v = c->absoluteVarInfo( name );
			if ( v != NULL ) {
				if ( tagMatches( imp, v ) ) {
					if ( vfile == NULL ) {
						vfile = v;
						def_pkg = c;
					} else {
						throw Mishap( "Ambiguous sources for definition" ).culprit( "Source 1", vfile->getPathName() ).culprit( "Source 2", v->getPathName() );
					}
				}
			}
		}
		if ( vfile != NULL ) {
			return def_pkg->getPackageName();
			//this->cacheUnqualifiedLookup( x->getPackageName(), name, def_pkg->getPackageName(), name );
			//this->cacheDefinitionFile( def_pkg, name, vfile->getPathName() );
		} else {
			if ( this->undefined_allowed ) {
				return enc_pkg_name;
			} else {	
				throw  (
					Mishap( "Cannot find variable" ).
					culprit( "Variable", name ).
					culprit( "Package", x->getPackageName() )
				);
			}
		}
	}
}


void Search::resolveUnqualified( const string & enc_pkg_name, const string & name ) {
	outputUnqualified( enc_pkg_name, name, findDefPkgUnqualified( enc_pkg_name, name ) );
}

string Search::findDefPkgQualified( const string & enc_pkg_name, const string & alias_name, const string & var_name ) {
	//cerr << "QUALIFIED" << endl;
	PackageCache * x = this->project_cache.fetchPackageCache( enc_pkg_name );
	VarInfo * vfile = NULL;
	PackageCache * def_pkg = NULL;
	ImportInfo * imp = importWithAlias( x, alias_name );
	if ( imp != NULL ) {
		PackageCache * c = this->project_cache.fetchPackageCache( imp->getFrom() );
		VarInfo * v = c->absoluteVarInfo( var_name );
		if ( v != NULL ) {
			if ( tagMatches( *imp, v ) ) {
				if ( vfile == NULL ) {
					vfile = v;
					def_pkg = c;
				} else {
					throw Mishap( "Ambiguous sources for definition" ).culprit( "Source 1", vfile->getPathName() ).culprit( "Source 2", v->getPathName() );
				}
			}
		}
	}
	if ( vfile != NULL ) {
		return def_pkg->getPackageName();
		//this->cacheUnqualifiedLookup( x->getPackageName(), name, def_pkg->getPackageName(), name );
		//this->cacheDefinitionFile( def_pkg, name, vfile->getPathName() );
	} else {
		if ( this->undefined_allowed ) {
			return enc_pkg_name;
		} else {
			throw  (
				Mishap( "Cannot find variable" ).
				culprit( "Variable", var_name ).
				culprit( "Package", enc_pkg_name )
			);
		}
	}
}

void Search::resolveQualified( const string & enc_pkg_name, const string & alias_name, const string & var_name ) {
	outputQualified( enc_pkg_name, alias_name, var_name, findDefPkgQualified( enc_pkg_name, alias_name, var_name ) );	
}

void Search::fetchDefinition( const string & pkg_name, const string & var_name ) {
	//cerr << "FETCH" << endl;
	PackageCache * pkg = this->project_cache.fetchPackageCache( pkg_name );
	this->printAbsoluteDefinition( pkg, var_name );
}

void Search::loadPackage( const string & pkg ) {
	PackageCache * c = this->project_cache.fetchPackageCache( pkg );
	string pathname = c->getInitLoadPath();
	if ( pathname.size() > 0 ) {
		run( INSTALL_TOOL PATH_SEPARATOR FILE2GNX, pathname, cout );
	} else {
		//cout << "<seq><!-- load path was not defined --></seq>" << endl;
	}
}

void Search::loadFileFromPackage( const string & pkg, const string & load_file ) {
	PackageCache * c = this->project_cache.fetchPackageCache( pkg );
	string pathname = c->getLoadFolder();
	if ( pathname.size() <= 0 ) {
		throw Mishap( "No load folder for this package" ).culprit( "Package", pkg );
	}
	const string file_name( pathname + PATH_SEPARATOR + load_file );
	if ( access( file_name.c_str(), R_OK ) == 0 ) {
		run( INSTALL_TOOL PATH_SEPARATOR FILE2GNX, file_name, cout );
	} else {
		throw Mishap( "Cannot find load file for this package" ).culprit( "Load file", load_file ).culprit( "Package", pkg );
	}	
}



void Search::browsePackages() {
	vector< PackageCache * > pkgs = this->project_cache.allPackageCaches();
	Ginger::MnxBuilder all;
	all.start( BROWSE_INFO );
	for ( vector< PackageCache * >::iterator it = pkgs.begin(); it != pkgs.end(); ++it ) {
		PackageCache * pkgc = *it;
		all.start( PKG );
		all.put( PKG_NAME, pkgc->getPackageName() );
		vector< VarInfo * > vars = pkgc->allVarInfo();
		for ( vector< VarInfo * >::iterator it = vars.begin(); it != vars.end(); ++it ) {
			VarInfo * vi = *it;
			all.start( VAR );
			all.put( VAR_NAME, vi->name() );
			all.end();
		}
		all.end();
	}
	all.end();
	all.build()->render();
	cout << endl;
}

void Search::resolveGnxInPlace( Ginger::SharedMnx gnx ) {
	ResolverVisitor resolver( this );
	gnx->visit( resolver );
	gnx->render();
	cout << endl;
}
