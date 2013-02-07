#include <iostream>
#include <string>

#include <stdlib.h>
#include <unistd.h>

namespace Ginger {

class XDGConfigFiles {
private:

	enum State {
		HomeState,
		DefaultHomeState,
		DirsState,
		DirsStateCont,
		DefaultDirsState,
		DoneState
	} state;
	int next_dir_index;

	bool cache_is_ok;
	bool cached_has_next;

private:
	std::string base_name;
	std::string config_home_env;
	std::string config_dirs_env;
	std::string config_dirs_val;
	std::string fallback_dir;
	std::string answer;

public:
	XDGConfigFiles(
		const char * base_name,
		const char * config_home_env = "XDG_CONFIG_HOME",
		const char * config_dirs_env = "XDG_CONFIG_DIRS",
		const char * fallback_dir = "/etc/xdg"
	) :
		state( HomeState ),
		next_dir_index( 0 ),
		cache_is_ok( false ),
		base_name( base_name ), 
		config_home_env( config_home_env ),
		config_dirs_env( config_dirs_env ),
		fallback_dir( fallback_dir )
	{
	}

private:
	bool tryReturnOrHasNext( const std::string & dir, const char * subdir, const State next_state );

	static bool isNullOrEmpty( const char * p ) {
		return p == NULL || p[ 0 ] == '\0';
	}

	bool lookup( const std::string & env, const char * subdir, const State fail_state, const State ok_state );

	bool callHasNext();

public:
	std::string next();

	bool hasNext();
	
};

} // namespace Ginger

