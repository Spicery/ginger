#include <iostream>
#include <string>

#include <stdlib.h>
#include <unistd.h>

#include "mishap.hpp"
#include "xdgconfigfiles.hpp"

using namespace Ginger;

static const char * HOME_ENV_VAR = "HOME";
static const char * GNG_SUB_DIR = "/ginger/";
static const char * GNG_HOME_SUB_DIR = "/.config/ginger/";

bool XDGConfigFiles::tryReturnOrHasNext( const std::string & dir, const char * subdir, const State next_state ) {
	std::string file( dir );
	file += subdir;
	file += this->base_name;
	if ( access( file.c_str(), R_OK ) == 0 ) {
		this->state = next_state;
		this->answer = file;
		return true;
	} else {
		this->state = next_state;
		return this->hasNext();
	}
}

bool XDGConfigFiles::lookup( const std::string & env, const char * subdir, const State fail_state, const State ok_state ) {
	char * dir = getenv( env.c_str() );
	if ( isNullOrEmpty( dir ) ) {
		this->state = fail_state;
		return this->hasNext();
	} else {
		return this->tryReturnOrHasNext( std::string( dir ), subdir, ok_state );
	}		
}

bool XDGConfigFiles::callHasNext() {
	switch ( this->state ) {
		case HomeState: {
			return lookup( this->config_home_env, GNG_SUB_DIR, DefaultHomeState, DirsState );
		}
		case DefaultHomeState: {
			return lookup( std::string( HOME_ENV_VAR ), GNG_HOME_SUB_DIR, DirsState, DirsState );
		}
		case DirsState: {
			char * ds = getenv( this->config_dirs_env.c_str() );
			if ( isNullOrEmpty( ds ) ) {
				this->state = DefaultDirsState;
			} else {
				this->state = DirsStateCont;
				this->config_dirs_val = ds;
			}
			return this->hasNext();
		}
		case DirsStateCont: {
			size_t next = this->config_dirs_val.find( ':', this->next_dir_index );
			if ( next != std::string::npos ) {
				size_t dir_index = this->next_dir_index;
				this->next_dir_index = next + 1;
				return this->tryReturnOrHasNext( this->config_dirs_val.substr( dir_index, next - dir_index ), GNG_SUB_DIR, DirsState );
			} else {
				return this->tryReturnOrHasNext( this->config_dirs_val.substr( this->next_dir_index ), GNG_SUB_DIR, DoneState );
			}
			break;
		}
		case DefaultDirsState: {
			return this->tryReturnOrHasNext( this->fallback_dir, GNG_SUB_DIR, DoneState );
		}
		case DoneState: {
			return false;
		}
		default: {
			throw Unreachable( __FILE__, __LINE__ );
		}
	}
}

std::string XDGConfigFiles::next() {
	if ( this->hasNext() ) {
		this->cache_is_ok = false;
		return this->answer;
	} else {
		throw Ginger::Mishap( "No more files" );
	}
}

bool XDGConfigFiles::hasNext() {
	if ( this->cache_is_ok ) return this->cached_has_next;
	this->cached_has_next = this->callHasNext();
	this->cache_is_ok = true;
	return this->cached_has_next;
}
