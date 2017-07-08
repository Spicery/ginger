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

#ifndef MACHINE_HPP
#define MACHINE_HPP

#include <memory>
#include <iostream>
#include <vector>

//#include "dict.hpp"
#include "codegen.hpp"
#include "instruction_set.hpp"
#include "heap.hpp"
#include "appcontext.hpp"
#include "package.hpp"
#include "registers.hpp"    //  To be deleted.

namespace Ginger {

class CodeGenClass;
typedef CodeGenClass * CodeGen;

class Pressure {
private:
    float pressure;
    
public:
    void setUnderPressure() {
        this->pressure = 2.0;
    }
    
    void clearUnderPressure() {
        this->pressure = 0.0;
    }

    bool isUnderPressure() {
        //std::cout << "test pressure (" << ( this->pressure > 1.0 ? "yes" : "no" ) << ")" << std::endl;
        return this->pressure > 1.0;
    }
    
    void increasePressure() {
        this->pressure += 1.0;
        //std::cout << "pressure(up) " << this->pressure << std::endl;
    }
    
    void decreasePressure() {
        this->pressure *= 0.5;
        //std::cout << "pressure(down) " << this->pressure << std::endl;
    }
    
public:
    Pressure() : pressure( 0.0 ) {}
};

class ExternalTracker {
private:
    std::list< Ref * > external_object_list;
public:
    void add( Ref * exobj ) {
        this->external_object_list.push_back( exobj );
    }

    void cleanUpAfterGarbageCollection();
};

class RootCell;

class MachineClass : public External {
friend class GarbageCollect;
friend class RootCell;

private:
    Ginger::AppContext *            appg;
    std::unique_ptr<CodeGenClass>   codegen_aptr;
    std::unique_ptr<HeapClass>      heap_aptr;
    Pressure                        pressure;
    std::vector< Ref >              queue;

public:
    ExternalTracker                 external_objects;
    Registers                       registers;
    std::vector< Cell >             spare_registers;
    bool                            sigint_flag;

    
private:
    std::unique_ptr<PackageManager> package_mgr_aptr;
    
public:
    Package *       getPackage( std::string name );
    Package *       getInteractivePackage();
    Pressure &      getPressure();
    
public:
    //  Volatile! Only cached when a garbage collection
    //  might be triggered.
    Ref *           program_counter;
    Ref *           func_of_program_counter;

public:
    void            check_call_stack_integrity();   //  debug
        
public:
    long            count;          //  Args count
    Ref *           link;           //  Return address
    Ref *           func_of_link;
    Ref *           sp;
    Ref *           sp_base;
    Ref *           sp_end;
    Ref *           vp;
    Ref *           vp_base;
    Ref *           vp_end; 
    
public:
    Ref &           fastPeek() { return *vp; }
    void            fastPoke( Cell c ) { *vp = c.asRef(); }
    Ref &           fastPeek( int n ) { return *( vp - n ); } 
    void            fastPoke( int n, Cell c ) { *( vp - n ) = c.asRef(); } 
    Ref             fastPop() { return *vp--; }
    Cell            fastPopAsCell() { return *vp--; }
    Ref             fastSet( Ref r ) { return *vp = r; }
    Ref             fastSet( Cell c ) { return *vp = c.asRef(); }
    void            fastPush( Ref r ) { *++vp = r; }
    void            fastPush( Cell c ) { *++vp = c.asRef(); }
    void            fastPushLong( const long n ) { fastPush( Cell( n ) ); }
    Ref             fastSubscr( int n ) { return *( vp - n ); }
    void            fastDrop( int n ) { vp -= n; }
    ptrdiff_t       stackLength() { return this->vp - this->vp_base; }
    void            checkStackRoom( long n );

public:
    void            pushAbsent();
    void            pushNil();
    void            pushTermin();
    void            pushUndef();
    void            pushCharacter( char c );
    void            pushSmall( long n );
    void            pushBool( bool b );

public:
    void            pushSimple( Cell c );
    void            sysCall( SysCall & s );
    
public:
    void            printResults( std::ostream & out, float time_taken );
    void            printResults( float time_taken );

    void            printfn( Ref x );
    void            printfn( std::ostream & out, Ref x );
    Ref*            instructionShow( Ref * pc );
    Ref*            instructionShow( std::ostream & out, Ref * pc );
    void            resetMachine();
    CodeGenClass *  codegen();      
    HeapClass &     heap();
    AppContext &    getAppContext();
    
public:
    virtual Ref sysFastListIterator();
    virtual Ref sysFastVectorIterator();
    virtual Ref sysFastMixedIterator();
    virtual Ref sysFastStringIterator();
    
public:
    virtual Ref * setUpPC( Ref r, const bool clear_stack );
    virtual void execute( Ref r, const bool clear_stack = true ) = 0;
    virtual const InstructionSet & instructionSet() = 0;
    void clearStack();
    void clearCallStack();

public:
    void executeQueue( const bool clear_stack = true );
    void addToQueue( Ref r );

public:
    bool getShowCode();
    bool isGCTrace();
    
private:
    int veto_count;
    
public:
    void gcLiftAllVetoes();             //  Makes decision to grow/shrink, level = 0.
    void gcVeto();                      //  Inhibit moving, bump +1 for nesting.
    void gcLiftVeto();                  //  bump -1, moving allowed if level = 0.
    bool gcMoveEnabled();
    void log( std::string msg ) {}  
    void trackExternalObject( Ref * exobj ) {
        this->external_objects.add( exobj );
    }
    void postGCManageExternalObjects() {
        this->external_objects.cleanUpAfterGarbageCollection();
    }

public:
    //  Part of the External interface.
    void print( std::ostream & out );
    Ref * sysApply( Ref * pc, MachineClass * vm );


public:
    MachineClass( AppContext * appg );
    virtual ~MachineClass();
};

typedef class MachineClass *Machine;

Machine machine_new( int n );
void machine_reset( Machine vm );

} // namespace Ginger

#endif

