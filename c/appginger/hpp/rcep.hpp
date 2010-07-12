#ifndef RCEP_HPP
#define RCEP_HPP

#include <istream>

#include "machine.hpp"

//	Does not trap mishap.
bool unsafe_read_comp_exec_print( Machine vm, std::istream & input );

//	Traps mishap.
bool read_comp_exec_print( Machine vm, std::istream & input );

#endif
