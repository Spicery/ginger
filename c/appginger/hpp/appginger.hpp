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

#ifndef APPGINGER_HPP
#define APPGINGER_HPP

#define VERSION "0.4 (alpha)"


class AppGinger {
private:
	enum Mode {
		InteractiveMode,
		BatchMode,
		CGIMode
	} mode;
	int machine_impl_num;
	bool dbg_show_code;
	bool is_trapping_mishap;
	
public:
	void setInteractiveMode() { this->mode = InteractiveMode; }
	void setBatchMode() { this->mode = BatchMode; }
	void setCgiMode() { this->mode = CGIMode; }
	bool isInteractiveMode() { return this->mode == InteractiveMode; }
	bool isBatchMode() { return this->mode == BatchMode; }
	bool isCgiMode() { return this->mode == CGIMode; }
	void setTrappingMishap( bool t ) { this->is_trapping_mishap = t; }
	bool isTrappingMishap() { return this->is_trapping_mishap; }
	void setMachineImplNum( const int n ) { this->machine_impl_num = n; }
	int getMachineImplNum() { return this->machine_impl_num; }
	const char * version() { return VERSION; }
	void setShowCode() { this->dbg_show_code = true; }
	bool getShowCode() { return this->dbg_show_code; }

public:
	void printGPL( const char * start, const char * end );
	MachineClass * newMachine();


public:
	AppGinger() :
		mode( InteractiveMode ),
		machine_impl_num( 1 ),
		dbg_show_code( false ),
		is_trapping_mishap( true )
	{
	}

};

#endif

