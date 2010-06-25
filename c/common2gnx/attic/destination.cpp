#include "destination.hpp"
#include "plant.hpp"
#include "mishap.hpp"

DestinationClass::DestinationClass( Plant plant ) {
	this->plant = plant;
	this->is_set = false;
	this->location = -1;
}

void DestinationClass::destinationSet() {
	int here = this->plant->codePosition();
	if ( this->is_set ) {
		this_never_happens();
	}
	this->location = here;
	this->is_set = true;
	
	//	Now update any pending insertions.
	for ( std::vector< int >::iterator it = this->pending_vector.begin(); it != this->pending_vector.end(); ++it ) {
		int there = *it;
		this->plant->codeUpdate( there, ( Ref )( here - there ) );
	}
}

void DestinationClass::destinationInsert() {
	int where = this->plant->codePosition();
	if ( this->is_set ) {
		this->plant->plantRef( ( Ref )( where - this->location ) );
	} else {
		this->pending_vector.push_back( where );
		this->plant->plantRef( (Ref)(-1) );
	}
}
