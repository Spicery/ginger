#include <iostream>

#include "transdouble.hpp"

std::ostream& operator<<( std::ostream& os, const TransDouble& obj )
{
	os << obj.asDouble();
	return os;
}