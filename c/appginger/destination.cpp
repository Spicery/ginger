#include <iostream>
using namespace std;

#include "destination.hpp"
#include "plant.hpp"
#include "mishap.hpp"

#define DBG_DESTINATION

DestinationClass::DestinationClass( Plant plant ) {
	this->plant = plant;
	this->is_set = false;
	this->location = -1;
}

void DestinationClass::destinationSet() {
	int here = this->plant->codePosition();
	#ifdef DBG_DESTINATION
		cout << "Setting destination at [" << here << "]" << endl;
	#endif
	if ( this->is_set ) {
		this_never_happens();
	}
	this->location = here;
	this->is_set = true;
	
	//	Now update any pending insertions.
	for ( std::vector< int >::iterator it = this->pending_vector.begin(); it != this->pending_vector.end(); ++it ) {
		int there = *it;
		#ifdef DBG_DESTINATION
			cout << "Pending update of [" << there << "] with jump " << ( here - there ) << endl;
		#endif
		this->plant->codeUpdate( there, ( Ref )( here - there ) );
	}
}

void DestinationClass::destinationInsert() {
	int where = this->plant->codePosition();
	if ( this->is_set ) {
		#ifdef DBG_DESTINATION
			cout << "Backward goto at [" << where << "] with jump " << where - this->location << endl;
		#endif
		this->plant->plantRef( ( Ref )( this->location - where ) );
	} else {
		#ifdef DBG_DESTINATION
		#endif
		this->pending_vector.push_back( where );
		this->plant->plantRef( (Ref)(-1) );
	}
}
