package com.steelypip.appginger.codegen;

import java.io.PrintWriter;

public class FullMixedClassGenerator extends DataClassGenerator {
	
	private final String[] fieldNames;
	private MethodOptions options;

	public FullMixedClassGenerator( final LogCreated sysconsts, String className, MethodOptions options, String... fieldNames ) {
		super( sysconsts, className );
		this.options = options;
		this.fieldNames = fieldNames;
	}

	public void generate( final PrintWriter cpp, final PrintWriter hpp ) {
		for ( String aname : this.allKeyRoots() ) {
			if ( options.isGenConstructor() ) this.generateConstructor( aname, cpp, hpp );
		}
		if ( options.isGenRecogniser() ) this.generateRecogniser( cpp, hpp );
		for ( int i = 0; i < this.fieldNames.length; i++ ) {
			if ( options.isGenFields() ) this.generateField( i, cpp, hpp );
		}
		if ( options.isGenIndexer() ) this.generateAccessor( cpp, hpp ); 
	}

	private void generateField( final int n, final PrintWriter cpp, final PrintWriter hpp ) {
		final String field = this.fieldNames[ n ];
		final String cap_field = capitalise( field );
		final String accessorName = "sys" + cap_field + "Field";
		cpp.format( "Ref * %s( Ref * pc, class MachineClass * vm ) {\n", accessorName );
		cpp.format( "    if ( vm->count == 1 ) {\n" );
		cpp.format( "        Ref x = vm->fastPeek();\n" );
		cpp.format( "        if ( %s( x ) ) {\n", this.isName() );
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
		this.addSysConst( field, 1, 1, accessorName );

	}

	private void generateAccessor( PrintWriter cpp, PrintWriter hpp ) {		
		cpp.format( "Ref * %s( Ref * pc, MachineClass * vm ) {\n", this.indexName() );
		cpp.format( "   if ( vm->count == 2 ) {\n" );
		cpp.format( "       Ref n = vm->fastPop();\n" );
		cpp.format( "       Ref v = vm->fastPeek();\n" );
		cpp.format( "       Ref * p = RefToPtr4( v );\n" );
		cpp.format( "       if ( IsObj( v ) && *RefToPtr4( v ) == %s ) {\n", this.keyName() );
		cpp.format( "           if ( IsSmall( n ) && LongToSmall( 1 ) <= n && n <= p[ -1 ] ) {\n" );
		cpp.format( "               vm->fastPeek() = p[ %s + SmallToLong( n ) ];\n", this.fieldNames.length );
		cpp.format( "           } else {\n" );
		cpp.format( "               throw Mishap( \"Small integer index needed\" );\n" );
		cpp.format( "           }\n" );
		cpp.format( "           return pc;" );
		cpp.format( "       } else {\n" );
		cpp.format( "           throw Mishap( \"Vector needed\" );\n" );
		cpp.format( "       }\n" );
		cpp.format( "   } else {\n" );
		cpp.format( "       throw Mishap( \"Wrong number of arguments for index\" );\n" );
		cpp.format( "   }\n" );
		cpp.format( "}\n" );
		
		hpp.format( "extern Ref * %s( Ref * pc, MachineClass * vm );\n", this.indexName() );
		this.addSysConst( "index" + this.dataKeyRoot, 2, 1, this.indexName() );
	}

	private void generateConstructor( final String altName, final PrintWriter cpp, final PrintWriter hpp ) {
		final String constructorName = this.altConsName( altName );
		final String keyName = this.altKeyName( altName );
		cpp.format( "Ref * %s( Ref * pc, class MachineClass * vm ) {\n", constructorName );
		cpp.format( "    const int N = vm->count;\n" );
		cpp.format( "    const int length = N - %s;\n", this.fieldNames.length );
		cpp.format( "    if ( length >= 0 ) {\n" );
		cpp.format( "        XfrClass xfr( vm->heap().preflight( pc, N + 2 ) );\n" );
		cpp.format( "        xfr.xfrRef( LongToSmall( length ) );\n" );
		cpp.format( "        xfr.setOrigin();\n" );
		cpp.format( "        xfr.xfrRef( %s );\n", keyName );
		cpp.format( "        xfr.xfrCopy( ++vm->vp -= N, N );\n" );
		cpp.format( "        vm->fastSet( xfr.makeRef() );\n" );
		cpp.format( "        return pc;\n" );
		cpp.format( "    } else {\n" );
		cpp.format( "        throw Mishap( \"Wrong number of arguments for %s\" );\n", constructorName  );
		cpp.format( "    }\n" );
		cpp.format( "}\n\n" );

		hpp.format( "extern Ref * %s( Ref * pc, MachineClass * vm );\n", constructorName );
		this.addSysConst( 
			"new" + altName + this.dataKeyRoot, 
			new Arity( this.fieldNames.length, true ), new Arity( 1 ), 
			constructorName 
		);
	}

	
}
