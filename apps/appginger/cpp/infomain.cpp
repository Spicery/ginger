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

#include <iostream>
#include <cstdlib>
#include <sstream>
#include <string>
#include <vector>

#include <getopt.h>


#include "mishap.hpp"
#include "appcontext.hpp"
#include "key.hpp"
#include "sys.hpp"

#define APP_TITLE "ginger-info"


using namespace std;

enum OutputFormat {
	XML_FORMAT,
	JSON_FORMAT,
	RESTRUCTUREDTEXT_FORMAT
};

class Formatter {
public:
	virtual void startDocument() = 0;
	virtual void endDocument() = 0;
	virtual void startSection( const string & sectionName ) = 0;
	virtual void endSection() = 0;
	virtual void insertComment( const string & comment ) = 0;
	virtual void startValue( const string & valueName ) = 0;
	virtual void addAttribute( const string & key, const string & val ) = 0;
	virtual void endValue() = 0;
	virtual ~Formatter() {}
};


class XMLFormatter : public Formatter {
private:
	int level;
	vector< string > sections;

public:
	XMLFormatter() : level( 0 ) {
	}

private:
	void indent() {
		for ( int i = 0; i < this->level; i++ ) {
			cout << "  ";
		}		
	}

	void renderText( std::ostream & out, const std::string & str ) {
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

public:
	void startDocument() {
		level += 1;
		cout << "<?xml version=\"1.0\"?>" << endl;
		cout << "<appginger>" << endl;
	}
	void endDocument() {
		cout << "</appginger>" << endl;
	}
	void startSection( const string & sectionName ) {
		this->sections.push_back( sectionName );
		this->indent();
		cout << "<" << sectionName << ">" << endl;
		this->level += 1;
	}
	void endSection() {
		const string & name = this->sections.back();
		this->sections.pop_back();
		this->level -= 1;
		this->indent();
		cout << "</" << name << ">" << endl;
	}
	void insertComment( const string & comment ) {
		this->indent();
		cout << "<!-- " << comment << " -->" << endl;
	}
	void startValue( const string & valueName ) {
		this->indent();
		cout << "<" << valueName;
	}
	void addAttribute( const string & key, const string & val ) {
		cout << " " << key << "=\"";
		renderText( cout, val );
		cout << "\"";
	}
	void endValue() {
		cout << " />" << endl;
	}
	virtual ~XMLFormatter() {}
};

class JSONFormatter : public Formatter {
public:
	void startDocument() {

	}
	void endDocument() {

	}
	void startSection( const string & sectionName ) {

	}
	void endSection() {

	}
	void insertComment( const string & comment ) {
		
	}
	void startValue( const string & valueName ) {

	}
	void addAttribute( const string & key, const string & val ) {

	}
	void endValue() {

	}
	virtual ~JSONFormatter() {}
};



class ReStructuredTextFormatter : public Formatter {
public:
	void startDocument() {

	}
	void endDocument() {

	}
	void startSection( const string & sectionName ) {

	}
	void endSection() {

	}
	void insertComment( const string & comment ) {
		
	}
	void startValue( const string & valueName ) {

	}
	void addAttribute( const string & key, const string & val ) {

	}
	void endValue() {

	}
	virtual ~ReStructuredTextFormatter() {}
};


static void printUsage() {
	cout << "Usage :  " << APP_TITLE << " [options] [files]" << endl << endl;
	cout << "OPTION                SUMMARY" << endl;
	cout << "-j, --json            output in JSON format" << endl;
	cout << "-r, --rst             output using reStructuredText" << endl;
	cout << "-x, --xml             output using XML (MinXML in fact)" << endl;
	cout << "-H, --help            print out this help info" << endl;
	cout << "-V, --version         print out version information and exit" << endl;
	cout << endl;
}	


static void printHelpHex() {
	cout << hex;
	cout << "absent         :\t" << SYS_ABSENT << endl;
	cout << "true           :\t" << SYS_TRUE << endl;
	cout << "false          :\t" << SYS_FALSE << endl;
	cout << "nil            :\t" << SYS_NIL << endl;
	cout << "termin         :\t" << SYS_TERMIN << endl;
	cout << "system_only    :\t" << SYS_SYSTEM_ONLY << endl;
	cout << "undef          :\t" << SYS_UNASSIGNED << endl;
	cout << "undef          :\t" << SYS_UNDEFINED << endl;
	cout << "undef          :\t" << SYS_INDETERMINATE << endl;
	cout << "Function       :\t" << sysFunctionKey << endl;
	cout << "CoreFunction   :\t" << sysCoreFunctionKey << endl;
	cout << "Method         :\t" << sysMethodKey << endl;
	cout << "AbsentKey      :\t" << sysAbsentKey << endl;
	cout << "BoolKey        :\t" << sysBoolKey << endl;
	cout << "KeyKey         :\t" << sysKeyKey << endl;
	cout << "TerminKey      :\t" << sysTerminKey << endl;
	cout << "NilKey         :\t" << sysNilKey << endl;
	cout << "PairKey        :\t" << sysPairKey << endl;
	cout << "VectorKey      :\t" << sysVectorKey << endl;
	cout << "StringKey      :\t" << sysStringKey << endl;
	cout << "SymbolKey      :\t" << sysSymbolKey << endl;
	cout << "SmallKey       :\t" << sysSmallKey << endl;
	cout << "FloatKey       :\t" << sysFloatKey << endl;
	cout << "UnicodeKey     :\t" << sysUnicodeKey << endl;
	cout << "CharKey        :\t" << sysCharKey << endl;
	cout << "MapletKey      :\t" << sysMapletKey << endl;
	cout << "HardEqMapKey   :\t" << sysHardEqMapKey << endl;
	cout << "HardIdMapKey   :\t" << sysHardIdMapKey << endl;
	cout << "WeakIdMapKey   :\t" << sysWeakIdMapKey << endl;
	cout << "CacheEqMapKey  :\t" << sysCacheEqMapKey << endl;
	cout << "AssocKey       :\t" << sysAssocKey << endl;
	cout << "IndirectionKey :\t" << sysIndirectionKey << endl;
	cout << "HardRefKey     :\t" << sysHardRefKey << endl;
	cout << "WeakRefKey     :\t" << sysWeakRefKey << endl;
	cout << "SoftRefKey     :\t" << sysSoftRefKey << endl;
}



extern char * optarg;
static struct option long_options[] =
    {
        { "help",           no_argument,      		0, 'H' },
        { "json",			no_argument,			0, 'j' },
        { "rst",            no_argument,      		0, 'r' },
        { "version",        no_argument,            0, 'V' },
        { "xml",          	no_argument,      		0, 'x' },
        { 0, 0, 0, 0 }
    };

class Main {
private:
	AppContext context;
	OutputFormat oformat;
	Formatter * formatter;

public:
	Main() : oformat( XML_FORMAT ), formatter( NULL ) {
	}

	~Main() {
		delete this->formatter;
	}

private:
	void renderText( std::ostream & out, const std::string & str ) {
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

	void printAttr( const char * name, const std::string & val ) {
		cout << name << "=\"";
		renderText( cout, val );
		cout << "\" ";
	}

	void printStdInfo() {
		//cout << "  <std>" << endl;
		this->formatter->startSection( "std" );

		//cout << "    <!-- Summary of the built-in functions -->" << endl;
		this->formatter->insertComment( "Summary of the built-in functions" );

		for (
			SysMap::iterator it = sysMap.begin();
			it != sysMap.end();
			++it
		) {
			
			//cout << "    <sysfn ";
			//printAttr( "name", it->first );
			//printAttr( "docstring", ( it->second.docstring != NULL ? it->second.docstring : "-" ) );
			//printAttr( "in", it->second.in_arity.toString() );
			//printAttr( "out", it->second.out_arity.toString() );
			//cout << "/>" << endl;

			this->formatter->startValue( "sysfn" );
			this->formatter->addAttribute( "name", it->first );
			this->formatter->addAttribute( "docstring", ( it->second.docstring != NULL ? it->second.docstring : "-" ) );
			this->formatter->addAttribute( "in", it->second.in_arity.toString() );
			this->formatter->addAttribute( "out", it->second.out_arity.toString() );
			this->formatter->endValue();

		}

		//cout << "  </std>" << endl;
		this->formatter->endSection();
	}

	void printBuildInfo() {
		//cout << "  <release>" << endl;
		this->formatter->startSection( "release" );

		//cout << "    <version "; printAttr( "number", APPGINGER_VERSION ); cout << "/>" << endl;
		this->formatter->startValue( "version" );
		this->formatter->addAttribute( "number", APPGINGER_VERSION );
		this->formatter->endValue();



		//cout << "    <build ";
		//printAttr( "file", __FILE__ );
		//printAttr( "date", __DATE__ ); 
		//printAttr( "time", __TIME__ ); 
		//cout << "/>" << endl;
		this->formatter->startValue( "build" );
		this->formatter->addAttribute( "file", __FILE__ );
		this->formatter->addAttribute( "date", __DATE__ );
		this->formatter->addAttribute( "time", __TIME__ );
		this->formatter->endValue();

		//cout << "  </release>" << endl;
		this->formatter->endSection();
	}

	void printLicenseInfo() {
		//cout << "  <ipr>" << endl;
		this->formatter->startSection( "ipr" );
		
		//cout << "    <!-- Intellectual Property Rights -->" << endl;
		this->formatter->insertComment( "Intellectual Property Rights" );

		//cout << "    <license url=\"http://www.gnu.org/licenses/gpl-3.0.txt\" />" << endl;
		this->formatter->startValue( "license" );
		this->formatter->addAttribute( "url", "http://www.gnu.org/licenses/gpl-3.0.txt" );
		this->formatter->endValue();

		//cout << "    <copyright notice=\"Copyright (c) 2010 Stephen Leach\" email=\"stephen.leach@steelypip.com\"/>" << endl;
		this->formatter->startValue( "copyright" );
		this->formatter->addAttribute( "notice", "Copyright (c) 2010 Stephen Leach" );
		this->formatter->addAttribute( "email", "stephen.leach@steelypip.com" );
		this->formatter->endValue();
		
		//cout << "  </ipr>" << endl;
		this->formatter->endSection();
	}

	void printCommunityInfo() {
		//cout << "  <community>" << endl;
		this->formatter->startSection( "community" );
		
		//cout << "    <!-- URLs for all the services for users & the devteam -->" << endl;
		this->formatter->insertComment( "URLs for all the services for users & the devteam" );
		
		//cout << "    <!-- We are obviously missing a user website, forum and mailing list -->" << endl;
		this->formatter->insertComment( "We are obviously missing a user website, forum and mailing list" );
		
		//cout << "    <repository type=\"subversion\" url=\"http://svn6.assembla.com/svn/ginger/\" />" << endl;
		this->formatter->startValue( "repository" );
		this->formatter->addAttribute( "type", "subversion" );
		this->formatter->addAttribute( "url", "http://svn6.assembla.com/svn/ginger/" );
		this->formatter->endValue();

		//cout << "    <issue_tracking type=\"trac\" url=\"http://trac6.assembla.com/ginger\" />" << endl;
		//	TODO! issue_tracking is not a valid element name! No underbars allowed.
		this->formatter->startValue( "issue_tracking" );
		this->formatter->addAttribute( "type", "trac" );
		this->formatter->addAttribute( "url", "http://trac6.assembla.com/ginger" );
		this->formatter->endValue();

		//cout << "  </community>" << endl;
		this->formatter->endSection();
	}

public:
	void printMetaInfo() {
		//cout << "<?xml version=\"1.0\"?>" << endl;
		//cout << "<appginger>" << endl;
		this->formatter->startDocument();

		//cout << "  <!-- Information about the AppGinger executable, its toolchain or community -->" << endl;
		this->formatter->insertComment( "Information about the AppGinger executable, its toolchain or community" );
		
		this->printBuildInfo();
		this->printLicenseInfo();
		this->printCommunityInfo();
		this->printStdInfo();
		this->formatter->endDocument();
		//cout << "</appginger>" << endl;
	}

public:
	bool parseArgs( int argc, char **argv, char **envp ) {
	    if ( envp != NULL ) this->context.setEnvironmentVariables( envp );
		for(;;) {
	        int option_index = 0;
	        int c = getopt_long( argc, argv, "HjkrVx", long_options, &option_index );
	        //cerr << "Got c = " << c << endl;
	        if ( c == -1 ) break;
	        switch ( c ) {
	            case 'x': {
	            	oformat = XML_FORMAT;
	                break;
	            }
	            case 'j': {
	            	oformat = JSON_FORMAT;
	            }
	            case 'r' : {
	            	oformat = RESTRUCTUREDTEXT_FORMAT;
	                break;
	            }
	            case 'k': {
	            	printHelpHex();
	            	return false;
	            }
	            case 'H': {
	                printUsage();
	                return false;
	            }
	            case 'V': {
	            	cout << APP_TITLE << ": version " << this->context.version() << " (" << __DATE__ << " " << __TIME__ << ")" << endl;
	            	return false;
	            }
	            case '?': {
	            	//	Invalid option: exit.
	                return false;
	            }
	            default: {
	            	//	This should not happen. It indicates that the option string 
	            	//	does not conform to the cases of this switch statement.
	            	throw Ginger::SystemError( "Unrecognised option" ).culprit( "Option code", static_cast< long >( c ) );
	            }
	        }
	    }

		//	Aggregate the remaining arguments, which are effectively filenames (paths).
		if ( optind < argc ) {
			 while ( optind < argc ) {
			   	this->context.addArgument( argv[ optind++ ] );
			 }
		}

		this->formatter = (
			oformat == XML_FORMAT ? dynamic_cast< Formatter * >( new XMLFormatter() ) :
			oformat == JSON_FORMAT ? dynamic_cast< Formatter * >( new JSONFormatter() ) :
			oformat == RESTRUCTUREDTEXT_FORMAT ? dynamic_cast< Formatter * >( new ReStructuredTextFormatter() ) :
			NULL
		);
		
		return true;
	}
};

int main( int argc, char **argv, char **envp ) {
	Main main;
	if ( main.parseArgs( argc, argv, envp ) ) {
		main.printMetaInfo();
	}
	return EXIT_SUCCESS;
}
