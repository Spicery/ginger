package com.steelypip.appginger.codegen;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.List;

public class SimpleKeyGenerator {
	
	static enum Kind {
		KEYLESS_KIND,
		RECORD_KIND,
		VECTOR_KIND,
		STRING_KIND,
		PAIR_KIND,
		MAP_KIND,
		UNUSED_KIND,
		OTHER_KIND
	};
	
	static class SimpleKey {
		String			name;
		int				id;
		int				nfields;
		Kind			kind;
		
		public SimpleKey( String name, int id, int nfields, Kind kind ) {
			super();
			this.name = name;
			this.id = id;
			this.nfields = nfields;
			this.kind = kind;
		}
		
	};
	
	private void add( String s, int id, int n, Kind k ) {
		this.keys.add( new SimpleKey( s, id, n, k ) );
	}
			
	final List< SimpleKey > keys = new ArrayList< SimpleKey >();
	{
		add( "Absent", 			0, 	0, 	Kind.KEYLESS_KIND );
		add( "Bool", 			1, 	0, 	Kind.KEYLESS_KIND );
		add( "Key",				2, 	4, 	Kind.RECORD_KIND );
		add( "Termin",			3, 	0, 	Kind.KEYLESS_KIND );
		add( "Nil", 			4, 	0, 	Kind.KEYLESS_KIND );
		add( "Pair", 			5, 	2, 	Kind.PAIR_KIND );
		add( "Vector",			6, 	0, 	Kind.VECTOR_KIND );
		add( "String", 			7, 	0, 	Kind.STRING_KIND );
		add( "Symbol", 			8, 	1, 	Kind.KEYLESS_KIND );
		add( "Small", 			9, 	0, 	Kind.KEYLESS_KIND );
		add( "Float", 			10, 0, 	Kind.KEYLESS_KIND );
		add( "Unicode", 		11, 0, 	Kind.KEYLESS_KIND );
		add( "Char", 			12, 0, 	Kind.KEYLESS_KIND );
		add( "Maplet",  		13, 2, 	Kind.RECORD_KIND );
		add( "Indirection", 	14, 1, 	Kind.RECORD_KIND );
		add( "Assoc", 			15, 3, 	Kind.RECORD_KIND );
		
		//	Do NOT move references with changing IsRefSimpleKey
		//	Must be on a multiple of 4 boundary.
		add( "HardRef", 		16, 1, 	Kind.RECORD_KIND );
		add( "SoftRef", 		17, 1, 	Kind.RECORD_KIND );
		add( "WeakRef", 		18, 1, 	Kind.RECORD_KIND );
		add( "ReservedRef", 	19, 1, 	Kind.RECORD_KIND );
		
		//	Do NOT move references with changing IsMapSimpleKey
		//	Must be on a multiple of 4 boundary.
		add( "CacheEqMap",		20, 3, 	Kind.MAP_KIND );
		add( "HardEqMap", 		21, 3,  Kind.MAP_KIND );
		add( "HardIdMap",		22, 3,  Kind.MAP_KIND );
		add( "WeakIdMap",		23, 3,  Kind.MAP_KIND );
		
		//	More arbitrary keys.
		add( "HashMapData", 	24, 0,	Kind.VECTOR_KIND );
	}
	
	public void generate( final File outputFolder ) throws IOException {
		this.generateHPP( outputFolder );
		this.generateCPP( outputFolder );
	}
	
	public void generateHPP( final File outputFolder ) throws IOException {
		final File hppfile = new File( outputFolder, "simplekey.hpp.auto" );
		final PrintWriter hpp = new PrintWriter( new FileWriter( hppfile ) );
		
		//	Generate the Kinds #define's.
		int n = 0;
		for ( Kind k : Kind.values() ) {
			hpp.format( "#define %s %s\n", k.toString(), n++ );
		}
		
		//	Generate the SimpleKey #define's.
		for ( SimpleKey skey : keys ) {
			hpp.format(  "#define %sID %s\n", skey.name, skey.id );
			hpp.format(  "#define sys%sKey MAKE_KEY( %s, %s, %s )\n", skey.name, skey.id, skey.nfields, skey.kind );
		}
		
		hpp.close();
	}
		
	public void generateCPP( final File outputFolder ) throws IOException {
	
		final File cppfile = new File( outputFolder, "simplekey.cpp.auto" );
		final PrintWriter cpp = new PrintWriter( new FileWriter( cppfile ) );

		
		//	Generate the SimpleKey #define's.
		for ( SimpleKey skey : keys ) {
			cpp.format(  "case %s: return \"%s\";\n", skey.id, skey.name );
		}

		/*
		switch ( SimpleKeyID( key ) ) {
		case 0: return "Absent";
		case 1: return "Bool";
		case 2: return "Key";
		case 3: return "Termin";
		case 4: return "Nil";
		case 5: return "Pair";
		case 6: return "Vector";
		case 7: return "String";
		case 8: return "Symbol";
		case 9: return "Small";
		case 10: return "Float";
		case 11: return "Unicode";
		case 12: return "Char";
		case 13: return "Maplet";
		case 14: return "Map";
		case 15: return "Assoc";
		case 16: return "Indirection";
		case 17: return "Ref";
		case 18: return "WeakRef";
		default: return "<Unknown>";
		*/

		cpp.close();
	}

	
	
}
