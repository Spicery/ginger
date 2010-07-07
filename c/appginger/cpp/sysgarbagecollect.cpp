#include "sysgarbagecollect.hpp"

#include <iostream>
using namespace std;

void sysGarbageCollect( MachineClass * vm ) {
	vm->garbageCollect();
}

void MachineClass::garbageCollect() {
}
