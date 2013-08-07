#include <iostream>

#include "transdouble.hpp"

std::ostream& operator<<( std::ostream& os, const TransDouble& obj )
{
	os << obj.asDouble();
	return os;
}

std::istream& operator>>( std::istream& is, TransDouble& obj )
{
	double d;
	is >> d;
	obj = d;
	return is;
}