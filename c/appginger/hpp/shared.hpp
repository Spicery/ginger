#ifndef SHARED_HPP
#define SHARED_HPP

#define BOOST
#ifdef BOOST
	#include <boost/shared_ptr.hpp>
	#define shared boost::shared_ptr
#else
	#include <tr1/memory>
	#define shared std::tr1::shared_ptr
#endif


#endif
