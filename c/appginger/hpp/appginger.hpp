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
	
public:
	void setInteractiveMode() { this->mode = InteractiveMode; }
	void setBatchMode() { this->mode = BatchMode; }
	void setCgiMode() { this->mode = CGIMode; }
	bool isInteractiveMode() { return this->mode == InteractiveMode; }
	bool isBatchMode() { return this->mode == BatchMode; }
	bool isCgiMode() { return this->mode == CGIMode; }
	void setMachineImplNum( const int n ) { this->machine_impl_num = n; }
	int getMachineImplNum() { return this->machine_impl_num; }
	const char * version() { return VERSION; }
	void setShowCode() { this->dbg_show_code = true; }
	bool getShowCode() { return this->dbg_show_code; }

public:
	MachineClass * newMachine();


public:
	AppGinger() :
		mode( InteractiveMode ),
		machine_impl_num( 1 ),
		dbg_show_code( false )
	{
	}

};

#endif
