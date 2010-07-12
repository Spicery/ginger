package com.steelypip.appginger.codegen;

import java.io.PrintWriter;


public class FullVectorClassGenerator extends DataClassGenerator {

	public FullVectorClassGenerator( final LogCreated sysconsts, String className ) {
		super( sysconsts, className );
	}

	@Override
	public void generate( final PrintWriter cpp, final PrintWriter hpp ) {
		this.generateConstructor( cpp, hpp );
		this.generateRecogniser( cpp, hpp );
		this.generateAccessor( cpp, hpp );
	}

	private void generateAccessor( PrintWriter cpp, PrintWriter hpp ) {		
		cpp.format( "Ref * %s( Ref * pc, MachineClass * vm ) {\n", this.indexName() );
		cpp.format( "   if ( vm->count == 2 ) {\n" );
		cpp.format( "       Ref n = vm->fastPop();\n" );
		cpp.format( "       Ref v = vm->fastPeek();\n" );
		cpp.format( "       Ref * p = RefToPtr4( v );\n" );
		cpp.format( "       if ( IsPtr4( v ) && *RefToPtr4( v ) == %s ) {\n", this.keyName() );
		cpp.format( "           if ( IsSmall( n ) && LongToSmall( 1 ) <= n && n <= p[ -1 ] ) {\n" );
		cpp.format( "               vm->fastPeek() = p[ SmallToLong( n ) ];\n" );
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
		cpp.format( "\n" );		
		
		hpp.format( "extern Ref * %s( Ref * pc, MachineClass * vm );\n", this.indexName() );
		this.addSysConst( "index" + this.dataKeyRoot, 2, this.indexName() );
	}

	private void generateConstructor( PrintWriter cpp, PrintWriter hpp ) {
		cpp.format( "Ref * %s( Ref * pc, MachineClass * vm ) {\n", this.consName() );
		cpp.format( "    int n = vm->count;\n" );
		cpp.format( "    XfrClass xfr( vm->heap().preflight( pc, 2 + n ) );\n" );
		cpp.format( "    xfr.xfrRef( LongToSmall( n ) );\n" );
		cpp.format( "    xfr.setOrigin();\n" );
		cpp.format( "    xfr.xfrRef( %s );\n", this.keyName() );
		cpp.format( "    xfr.xfrCopy( ++vm->vp -= n, n );\n" );
		cpp.format( "    vm->fastSet( xfr.make() );\n" );
		cpp.format( "    return pc;\n" );
		cpp.format( "}\n" );

		hpp.format( "extern Ref * %s( Ref * pc, MachineClass * vm );\n", this.consName() );
		this.addSysConst( "new" + this.dataKeyRoot, Arity.MANY, this.consName() );

	}
	
}
