package com.steelypip.appginger.codegen;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.List;

public class SimpleKeyGenerator {
	
	/*
	 * "Kind" is a grouping of compound objects that have the same
	 * treatment by 
	 * 	1.	The garbage collector
	 * 	2.	The generic functions defined in sys.cpp, sysequals.cpp, sysprint.cpp, sysstring.cpp, sysvector.cpp
	 */
	
	//	3 bits. The "Layout" is the pattern by which the store of an object
	//	is positioned around a key. It is the level at which the garbage collector
	//	needs to understand heap contents.
	static enum Layout {
		KEYLESS_LAYOUT,
		RECORD_LAYOUT,
		VECTOR_LAYOUT,
		STRING_LAYOUT,
		MIXED_LAYOUT,
		UNUSED5_LAYOUT,
		UNUSED6_LAYOUT,
		UNUSED7_LAYOUT
	};

	//	4 bits. The "Kind" is a subdivision of layouts. Objects that are the
	//	same kind share method implementations.
	static enum Kind {
		KEYLESS_KIND( Layout.KEYLESS_LAYOUT ),
		RECORD_KIND( Layout.RECORD_LAYOUT ),
		PAIR_KIND( Layout.RECORD_LAYOUT ),
		MAP_KIND( Layout.RECORD_LAYOUT ),
		VECTOR_KIND( Layout.VECTOR_LAYOUT ),
		STRING_KIND( Layout.STRING_LAYOUT ),
		ATTR_KIND( Layout.MIXED_LAYOUT ),
		MIXED_KIND( Layout.MIXED_LAYOUT );
		
		private Layout layout;
		
		Kind( Layout layout ) {
			this.layout = layout;
		}
		
		public int kindBits() {
			return this.ordinal();
		}
		
		public int layoutBits() { return this.layout.ordinal(); }
	}

	
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
		add( "Absent", 			0, 	0, 	Kind.KEYLESS_KIND 	);
		add( "Bool", 			1, 	0, 	Kind.KEYLESS_KIND	);
		add( "Key",				2, 	4, 	Kind.RECORD_KIND	);
		add( "Termin",			3, 	0, 	Kind.KEYLESS_KIND	);
		add( "Nil", 			4, 	0, 	Kind.KEYLESS_KIND	);
		add( "Pair", 			5, 	2, 	Kind.PAIR_KIND		);
		add( "Vector",			6, 	0, 	Kind.VECTOR_KIND	);
		add( "String", 			7, 	0, 	Kind.STRING_KIND	);
		add( "Symbol", 			8, 	1, 	Kind.KEYLESS_KIND	);
		add( "Small", 			9, 	0, 	Kind.KEYLESS_KIND	);
		add( "Float", 			10, 0, 	Kind.KEYLESS_KIND	);
		add( "Unicode", 		11, 0, 	Kind.KEYLESS_KIND	);
		add( "Char", 			12, 0, 	Kind.KEYLESS_KIND	);
		add( "Maplet",  		13, 2, 	Kind.RECORD_KIND	);
		add( "Indirection", 	14, 1, 	Kind.RECORD_KIND	);
		add( "Assoc", 			15, 3, 	Kind.RECORD_KIND	);
		
		//	Do NOT move references without changing IsRefSimpleKey
		//	Must be on a multiple of 4 boundary.
		add( "HardRef", 		16, 1, 	Kind.RECORD_KIND	);
		add( "SoftRef", 		17, 1, 	Kind.RECORD_KIND	);
		add( "WeakRef", 		18, 1, 	Kind.RECORD_KIND	);
		add( "ReservedRef", 	19, 1, 	Kind.RECORD_KIND	);
		
		//	Do NOT move references without changing IsMapSimpleKey
		//	Must be on a multiple of 4 boundary.
		add( "CacheEqMap",		20, 3, 	Kind.MAP_KIND		);
		add( "HardEqMap", 		21, 3,  Kind.MAP_KIND		);
		add( "HardIdMap",		22, 3,  Kind.MAP_KIND		);
		add( "WeakIdMap",		23, 3,  Kind.MAP_KIND		);
		
		//	More arbitrary keys.
		add( "HashMapData", 	24, 0,	Kind.VECTOR_KIND	);
		add( "Unassigned", 		25, 0,	Kind.KEYLESS_KIND	);
		add( "Undefined", 		26, 0,  Kind.KEYLESS_KIND	);
		add( "Indeterminate",   27, 0,  Kind.KEYLESS_KIND	);
		add( "Element",      	28, 1, 	Kind.MIXED_KIND	);
		add( "AttrMap",			29,	1,	Kind.ATTR_KIND		);
		add( "Exception",       30, 3,  Kind.RECORD_KIND	);	//	TAG : Symbol, VALUES : List< Any >, DETAILS : List< Maplet > 
	}
	
	public void generate( final File outputFolder ) throws IOException {
		this.generateHPP( outputFolder );
		this.generateCPP( outputFolder );
	}
	
	public void generateHPP( final File outputFolder ) throws IOException {
		final File hppfile = new File( outputFolder, "simplekey.hpp.auto" );
		final PrintWriter hpp = new PrintWriter( new FileWriter( hppfile ) );
		
		//	Generate the Kinds #define's.
		for ( Kind k : Kind.values() ) {
			hpp.format( "#define %s %s\n", k.toString(), k.kindBits() );
		}
		
		//	Generate the Layout #define's.
		for ( Layout x : Layout.values() ) {
			hpp.format( "#define %s %s\n", x.toString(), x.ordinal() );
		}
		
		
		//	Generate the SimpleKey #define's.
		for ( SimpleKey skey : keys ) {
			hpp.format(  "#define %sID %s\n", skey.name, skey.id );
			hpp.format(  
				"#define sys%sKey MAKE_KEY( %s, %s, %s, %s )\n", 
				skey.name, skey.id, skey.nfields, skey.kind.kindBits(), skey.kind.layoutBits()  
			);
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
