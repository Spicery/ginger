#ifndef MISHAP_HPP
#define MISHAP_HPP

#include <string>
#include <vector>
#include <utility>

class Mishap {
private:
	std::string message;
	std::vector< std::pair< std::string, std::string > > culprits;

public:
	Mishap & culprit( const std::string reason, const std::string arg );
	Mishap & culprit( const std::string arg );
	void report();
	
public:
	Mishap( const std::string & msg );
};

#endif
