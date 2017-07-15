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

#include <stddef.h>


//	Ginger libs
#include "mishap.hpp"
#include "mnxsax.hpp"

//	Local libs
#include "lnxreader.hpp"

namespace Ginger {
using namespace std;

class LnxHandler : public MnxSaxHandler {
private:
	int level;
	LnxReader * reader;
	LnxItem * item;
	bool done;
	bool on_start_close;
	
public:
	void startTagOpen( std::string & name ) {
		//	Check name is linx?
		//cout << "Start " << level << endl;
		level += 1;
		if ( this->level == 2 ) {
			this->item->resetToDefaults();
		}
	}
	
	void put( std::string & key, std::string & value ) {
		//cerr << "DEBUG: level " << level << " key " << key << " value " << value << endl;
		//cerr << "       reader = " << this->reader << endl;
		if ( level == 1 ) {
			//	Check name is linx?
			//	Check whether or not we already know it, otherwise add.	
			this->reader->put( key, value );
		} else if ( level == 2 ) {
			this->item->put( key, value );
		} else {
			//	This is an error. But we permit it by skipping.
			//cerr << "ERROR: level " << level << " key " << key << " value " << value << endl;
			throw "Debug";
		}
	}
	
	virtual void startTagClose( std::string & name ) {
		//	No action.
		this->on_start_close = true;
	}
	
	virtual void endTag( std::string & name ) {
		this->level -= 1;
		this->on_start_close = false;
		if ( this->level == 0 ) {
			this->done = true;
		}
		//cout << "End " << level << endl;
	}
	
public:
	bool isDone() { return this->done; }
	bool needsInit() {
		return this->level == 0 && !this->done;
	}
	
	bool isAtBreak() { 
		/*cerr << "IsAtBreak? " << "level = " << level;
		cerr << " on-start-close = " << on_start_close;
		cerr << " done = " << done;
		cerr << "; answer = " << ( this->done || this->on_start_close && this->level == 2 );
		cerr << endl;*/
		return this->on_start_close && this->level == 2;
	}
		
public:
	LnxHandler( LnxReader * reader, LnxItem * item ) : 
		level( 0 ), 
		reader( reader ), 
		item( item ),
		done( false ),
		on_start_close( false )
	{}
};

LnxReader::LnxReader( std::istream & input ) : 
	input( input ), 
	property_count( 0 )
//	current_item( new LnxItem( this ) ), 
//	handler( new LnxHandler( this, this->current_item ) ),
//	parser( new MnxSaxParser( this->input, *this->handler ) )
{
	this->current_item = new LnxItem( this );
	this->handler = new LnxHandler( this, this->current_item );
	this->parser = new MnxSaxParser( this->input, *this->handler );
}

LnxReader::~LnxReader() {
	if ( this->current_item != NULL ) {
		delete this->parser;
		delete this->handler;
		delete this->current_item;
	}
}

LnxItem * LnxReader::peek() {
	if ( this->handler->isAtBreak() ) {
		return this->current_item;
	} else if ( this->handler->isDone() ) {
		return NULL;
	} else {
		return this->read();
	}
}

void LnxReader::drop() {
	this->read();
}

LnxItem * LnxReader::read() {
	if ( this->handler->isDone() ) return NULL;
	for (;;) {
		this->parser->read();
		if ( this->handler->isDone() ) return NULL;
		if ( this->handler->isAtBreak() ) return this->current_item;
	}
}

void LnxReader::fillMap( std::map< std::string, std::string > & map ) {
	if ( this->handler->needsInit() ) {
		//	We are at the start & the columns have not been set up.
		//	A single read is enough to cause initialisation.
		this->parser->read();
	}
	for ( int i = 0; i < this->property_count; i++ ) {
		map[ this->property_keys[ i ] ] = this->property_defaults[ i ];
	}
}

int LnxReader::propertyCount() {
	if ( this->handler->needsInit() ) {
		//	We are at the start & the columns have not been set up.
		//	A single read is enough to cause initialisation.
		this->parser->read();
	}
	return this->property_count;
}


void LnxReader::put( const string & key, const string & value ) {
	map< string, int >::iterator it = this->property_index.find( key );
	if ( it == this->property_index.end() ) {
		this->property_index[ key ] = this->property_count++;
		this->property_defaults.push_back( value );
		this->property_keys.push_back( key );
	} else {
		throw Mishap( "Repeated attribute at start of LinX stream" ).culprit( "Attribute name", key );
	}
}

bool LnxReader::hasProperty( const std::string & key ) {
	map< string, int >::const_iterator it = this->property_index.find( key );
	return it != this->property_index.end();
}


int LnxReader::propertyIndex( const std::string & key ) { 
	map< string, int >::const_iterator it = this->property_index.find( key );
	if ( it != this->property_index.end() ) {
		return it->second;
	} else {
		throw Mishap( "Unexpected attribute" ).culprit( "Attribute", key );
	}
}

} // namespace
