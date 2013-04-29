/******************************************************************************\
	Copyright (c) 2010 Stephen Leach. AppGinger is distributed under the terms 
	of the GNU General Public License. This file is part of AppGinger.

    AppGinger is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    AppGinger is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with AppGinger.  If not, see <http://www.gnu.org/licenses/>.
\******************************************************************************/

#ifndef ENGINE_REGISTER_HPP
#define ENGINE_REGISTER_HPP

#include <vector>
#include <string>
#include "machine.hpp"
#include "generator.hpp"

namespace Ginger {

class AppContext;

class EngineFactory {
private:
	const std::string short_name;
	const std::string long_name;
	const std::string description;
public:
	EngineFactory( 
		const std::string & _short_name, 
		const std::string & _long_name, 
		const std::string & _description 
	) :
		short_name( _short_name ),
		long_name( _long_name ),
		description( _description )
	{}
	virtual ~EngineFactory() {}
public:
	bool match( const std::string & name ) {
		return this->short_name == name || this->long_name == name;
	}
	const std::string getShortName() { return this->short_name; }
	const std::string getLongName() { return this->long_name; }
	const std::string getDescription() { return this->description; }
public:
	virtual MachineClass * newEngine( AppContext * cxt ) = 0;
};

class EngineFactoryRegistration {
public:
	typedef std::vector< EngineFactory * >::iterator iterator;
	typedef Ginger::Generator< std::vector< EngineFactory * > > Generator;
public:
	static std::vector< EngineFactory * > & getRegister();
public:
	EngineFactoryRegistration( EngineFactory * engine_factory ) {
		getRegister().push_back( engine_factory );
	}
public:
	static EngineFactory * findMatch( const std::string & name );
	static Generator generator() { return Generator( getRegister() ); }
};

} // namespace Ginger

#endif