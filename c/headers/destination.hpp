#ifndef DESTINATION_HPP
#define DESTINATION_HPP

#include <vector>

class PlantClass;

class DestinationClass {
private:
	std::vector< int >	pending_vector;
	
public:
	PlantClass *		plant;
	bool				is_set;
	int					location;
	
public:
	DestinationClass( PlantClass * plant );
	
public:
	void destinationSet();
	void destinationInsert();
		
};

typedef DestinationClass * Destination;

#endif
