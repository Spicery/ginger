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
		cout << "<?xml version=\"1.0\"?>" << endl;
		cout << "<appginger>" << endl;
		level += 1;
	}
	void endDocument() {
		level -= 1;
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
private:
	bool with_comments;
	int level;
	int section_count;
	int value_count;
	bool in_std;			//	True iff we are in the std section.

public:
	JSONFormatter() : with_comments( false ), level( 0 ), in_std( false ) {
	}

private:
	void indent() {
		for ( int i = 0; i < this->level; i++ ) {
			cout << "  ";
		}		
	}

	void renderText( const std::string & str ) {
		for ( std::string::const_iterator it = str.begin(); it != str.end(); ++it ) {
			const unsigned char ch = *it;
			if ( ch == '"' ) {
				cout << "\\\"";
			} else if ( ch == '\\' ) {
				cout << "\\\\";
			} else {
				cout << ch;
			}
		}
	}


public:
	void startDocument() {
		cout << "{" << endl;
		this->level += 1;
		this->section_count = 0;
	}
	void endDocument() {
		if ( this->section_count > 0 ) {
			this->indent();
			if ( this->in_std ) {
				cout << "}" << endl;
			} else {
				cout << "]" << endl;
			}
		}
		this->level -= 1;
		cout << "}" << endl;
	}
	void startSection( const string & sectionName ) {
		if ( this->section_count > 0 ) {
			this->indent();
			cout << "]," << endl;
		}
		this->value_count = 0;
		this->indent();
		this->in_std = ( sectionName == "std" );
		if ( this->in_std ) {
			cout << "\"" << sectionName << "\": {" << endl;
		} else {
			cout << "\"" << sectionName << "\": [" << endl;
		}
		this->level += 1; 
	}
	void endSection() {
		if ( this->value_count > 0 ) {
			cout << " }" << endl;
		}
		this->level -= 1;
		this->section_count += 1;
	}
	void insertComment( const string & comment ) {
		//	JSON does not support comments, strictly speaking.
		if ( with_comments ) {
			this->indent();
			cout << "# " << comment << endl;
		}
	}
	void startValue( const string & topic ) {
		if ( this->value_count > 0 ) {
			cout << " }," << endl;
		}
		this->indent();
		if ( not this->in_std ) {
			cout << "{ \"topic\": \"" << topic << "\"";
		}
	}
	void addAttribute( const string & key, const string & val ) {
		//	N.B. We rely on the fact that the sysfn attributes are 
		//	emitted in the order name, docstring, <others> to get
		//	the formatting correct!
		if ( this->in_std && ( key == "name" ) ) {
			cout << "\"";
			this->renderText( val ); 
			cout << "\": { ";
		} else if ( this->in_std && ( key == "docstring" ) ) {
			cout << "\"" << key << "\": \"";
			this->renderText( val );
			cout << "\"";			
		} else {
			cout << ", \"" << key << "\": \"";
			this->renderText( val );
			cout << "\"";			
		}
	}
	void endValue() {
		this->value_count += 1;
	}
	virtual ~JSONFormatter() {}
};



class ReStructuredTextFormatter : public Formatter {
private:
	int section_count;

public:
	ReStructuredTextFormatter() : section_count( 0 ) {}

public:
	void startDocument() {
		cout << "Ginger Virtual Machine Information" << endl;
		cout << "==================================" << endl << endl;
	}
	void endDocument() {
	}
	void startSection( const string & sectionName ) {
		cout << sectionName << endl;
		size_t n = sectionName.size();
		for ( size_t i = 0; i < n; i++ ) {
			cout << "-";
		}
		cout << endl << endl;
	}
	void endSection() {
		cout << endl;
	}
	void insertComment( const string & comment ) {
		cout << comment << endl << endl;
	}
	void startValue( const string & valueName ) {
		cout << valueName << endl;
	}
	void addAttribute( const string & key, const string & val ) {
		cout << "    " << key << endl;
		cout << "        " << val << endl;
	}
	void endValue() {
		cout << endl;
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
	cout << "-K, --hexkeys         print cheat sheet for simple key values" << endl;
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
        { "hexkeys",		no_argument,			0, 'K' },
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
		this->formatter->startSection( "std" );

		this->formatter->insertComment( "Summary of the built-in functions" );

		for (
			SysMap::iterator it = sysMap.begin();
			it != sysMap.end();
			++it
		) {
			this->formatter->startValue( "sysfn" );
			this->formatter->addAttribute( "name", it->first );
			this->formatter->addAttribute( "docstring", ( it->second.docstring != NULL ? it->second.docstring : "-" ) );
			this->formatter->addAttribute( "in", it->second.in_arity.toString() );
			this->formatter->addAttribute( "out", it->second.out_arity.toString() );
			this->formatter->endValue();
		}

		this->formatter->endSection();
	}

	void printBuildInfo() {
		this->formatter->startSection( "release" );

		this->formatter->startValue( "version" );
		this->formatter->addAttribute( "number", APPGINGER_VERSION );
		this->formatter->endValue();

		this->formatter->startValue( "build" );
		this->formatter->addAttribute( "file", __FILE__ );
		this->formatter->addAttribute( "date", __DATE__ );
		this->formatter->addAttribute( "time", __TIME__ );
		this->formatter->endValue();

		this->formatter->endSection();
	}

	void printLicenseInfo() {
		this->formatter->startSection( "ipr" );
		
		this->formatter->insertComment( "Intellectual Property Rights" );

		this->formatter->startValue( "license" );
		this->formatter->addAttribute( "url", "http://www.gnu.org/licenses/gpl-3.0.txt" );
		this->formatter->endValue();

		this->formatter->startValue( "copyright" );
		this->formatter->addAttribute( "notice", "Copyright (c) 2010 Stephen Leach" );
		this->formatter->addAttribute( "email", "stephen.leach@steelypip.com" );
		this->formatter->endValue();
		
		this->formatter->endSection();
	}

	void printCommunityInfo() {
		this->formatter->startSection( "community" );
		
		this->formatter->insertComment( "URLs for all the services for users & the devteam" );
		
		this->formatter->insertComment( "We are obviously missing a user website, forum and mailing list" );
		
		this->formatter->startValue( "repository" );
		this->formatter->addAttribute( "type", "subversion" );
		this->formatter->addAttribute( "url", "http://svn6.assembla.com/svn/ginger/" );
		this->formatter->endValue();

		this->formatter->startValue( "issue_tracking" );
		this->formatter->addAttribute( "type", "trac" );
		this->formatter->addAttribute( "url", "http://trac6.assembla.com/ginger" );
		this->formatter->endValue();

		this->formatter->endSection();
	}

public:
	void printMetaInfo() {
		this->formatter->startDocument();

		this->formatter->insertComment( "Information about the AppGinger executable, its toolchain or community" );
		
		this->printBuildInfo();
		this->printLicenseInfo();
		this->printCommunityInfo();
		this->printStdInfo();
		this->formatter->endDocument();
	}

public:
	bool parseArgs( int argc, char **argv, char **envp ) {
	    if ( envp != NULL ) this->context.setEnvironmentVariables( envp );
		for(;;) {
	        int option_index = 0;
	        int c = getopt_long( argc, argv, "HjkrVx", long_options, &option_index );
	        
	        if ( c == -1 ) break;
	        switch ( c ) {
	            case 'x': {
	            	oformat = XML_FORMAT;
	                break;
	            }
	            case 'j': {
	            	oformat = JSON_FORMAT;
	            	break;
	            }
	            case 'r' : {
	            	oformat = RESTRUCTUREDTEXT_FORMAT;
	                break;
	            }
	            case 'K': {
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
