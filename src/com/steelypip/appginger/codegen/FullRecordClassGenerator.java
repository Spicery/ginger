package com.steelypip.appginger.codegen;

import java.io.PrintWriter;

public class FullRecordClassGenerator extends DataClassGenerator {
	
	private final String[] fieldNames;


	public FullRecordClassGenerator( final LogCreated sysconsts, String className, String... fieldNames ) {
		super( sysconsts, className );
		this.fieldNames = fieldNames;
	}


	public void generate( final PrintWriter cpp, final PrintWriter hpp ) {
		this.generateConstructor( cpp, hpp );
		this.generateRecogniser( cpp, hpp );
		for ( int i = 0; i < this.fieldNames.length; i++ ) {
			this.generateField( i, cpp, hpp );
		}
	}

	private void generateField( final int n, final PrintWriter cpp, final PrintWriter hpp ) {
		final String field = this.fieldNames[ n ];
		final String cap_field = capitalise( field );
		final String keyName = "sys" + this.dataKeyRoot + "Key";
		final String accessorName = "sys" + cap_field + "Field";
		cpp.format( "Ref * %s( Ref * pc, class MachineClass * vm ) {\n", accessorName );
		cpp.format( "    if ( vm->count == 1 ) {\n" );
		cpp.format( "        Ref x = vm->fastPeek();\n" );
		cpp.format( "        if ( refKey( x ) == %s ) {\n", keyName );
		cpp.format( "            vm->fastPeek() = RefToPtr4( x )[ %s ];\n", n + 1 );
		cpp.format( "        } else {\n" );
		cpp.format( "            throw Mishap( \"Trying to take the %s of non-%s\" );\n", field, dataKeyRoot );
		cpp.format( "        }\n" );
		cpp.format( "        return pc;\n" );
		cpp.format( "    } else {\n" );
		cpp.format( "        throw Mishap( \"Wrong number of arguments\" );\n" );
		cpp.format( "    }\n" );
		cpp.format( "}\n\n" );
		
		hpp.format( "extern Ref * %s( Ref * pc, MachineClass * vm );\n", accessorName );
		this.addSysConst( field, 1, accessorName );

	}

	static boolean memcpy = false;

	private void generateConstructor( final PrintWriter cpp, final PrintWriter hpp ) {
		final String constructorName = this.consName();
		final String keyName = this.keyName();
		cpp.format( "Ref * %s( Ref * pc, class MachineClass * vm ) {\n", constructorName );
		cpp.format( "    if ( vm->count == 2 ) {\n" );
		cpp.format( "        XfrClass xfr( vm->heap().preflight( pc, %s ) );\n", this.fieldNames.length + 1 );
		cpp.format( "        xfr.setOrigin();\n" );
		cpp.format( "        xfr.xfrRef( %s );\n", keyName );
		
		//	Not sure which is better.
		if ( memcpy ) {
			cpp.format( "        xfr.xfrCopy( ++vm->vp -= %s, %s );\n", this.fieldNames.length );
			cpp.format( "        vm->fastSet( xfr.makeRef() );\n" );
		} else {
			for ( int i = this.fieldNames.length - 1; i >= 0; i-- ) {
				cpp.format( "        Ref f%s = vm->fastPop();\n", i );
			}
			for ( int i = 0; i < this.fieldNames.length; i++ ) {
				cpp.format( "        xfr.xfrRef( f%s );\n", i );
			}
			cpp.format( "        vm->fastPush( xfr.makeRef() );\n" );
			cpp.format( "        return pc;\n" );
		}
		
		cpp.format( "    } else {\n" );
		cpp.format( "        throw Mishap( \"Wrong number of arguments for %s\" );\n", constructorName  );
		cpp.format( "    }\n" );
		cpp.format( "}\n\n" );

		hpp.format( "extern Ref * %s( Ref * pc, MachineClass * vm );\n", constructorName );
		this.addSysConst( "new" + this.dataKeyRoot, this.fieldNames.length, constructorName );
	}
	
	
	
	
}
