#include "CppUnitLite2.h"

#include "common.hpp"
#include "key.hpp"
#include "vectorlayout.hpp"

using namespace Ginger;

static int inferredLayout( Ref simple_key ) {
	int kind = KindOfSimpleKey( simple_key );
	switch ( kind ) {
		case PAIR_KIND:
		case RECORD_KIND:
		case MAP_KIND:
			return RECORD_LAYOUT;
		case VECTOR_KIND:
			return VECTOR_LAYOUT;
		case STRING_KIND:
			return STRING_LAYOUT;
		case ATTR_KIND:
		case MIXED_KIND:
			return MIXED_LAYOUT;
		case WRECORD_KIND:
			return WRECORD_LAYOUT;
		default:
			return KEYLESS_LAYOUT;
	}
}

TEST( SimpleKey ) {
	CHECK_EQUAL( inferredLayout( sysAbsentKey ), LayoutOfSimpleKey( sysAbsentKey ) );
	CHECK_EQUAL( inferredLayout( sysBoolKey ), LayoutOfSimpleKey( sysBoolKey ) );
	CHECK_EQUAL( inferredLayout( sysClassKey ), LayoutOfSimpleKey( sysClassKey ) );
	CHECK_EQUAL( inferredLayout( sysTerminKey ), LayoutOfSimpleKey( sysTerminKey ) );
	CHECK_EQUAL( inferredLayout( sysNilKey ), LayoutOfSimpleKey( sysNilKey ) );
	CHECK_EQUAL( inferredLayout( sysPairKey ), LayoutOfSimpleKey( sysPairKey ) );
	CHECK_EQUAL( inferredLayout( sysVectorKey ), LayoutOfSimpleKey( sysVectorKey ) );
	CHECK_EQUAL( inferredLayout( sysStringKey ), LayoutOfSimpleKey( sysStringKey ) );
	CHECK_EQUAL( inferredLayout( sysSymbolKey ), LayoutOfSimpleKey( sysSymbolKey ) );
	CHECK_EQUAL( inferredLayout( sysSmallKey ), LayoutOfSimpleKey( sysSmallKey ) );
	CHECK_EQUAL( inferredLayout( sysDoubleKey ), LayoutOfSimpleKey( sysDoubleKey ) );
	//CHECK_EQUAL( inferredLayout( sysUnicodeKey ), LayoutOfSimpleKey( sysUnicodeKey ) );
	CHECK_EQUAL( inferredLayout( sysCharKey ), LayoutOfSimpleKey( sysCharKey ) );
	CHECK_EQUAL( inferredLayout( sysMapletKey ), LayoutOfSimpleKey( sysMapletKey ) );
	CHECK_EQUAL( inferredLayout( sysIndirectionKey ), LayoutOfSimpleKey( sysIndirectionKey ) );
	CHECK_EQUAL( inferredLayout( sysAssocKey ), LayoutOfSimpleKey( sysAssocKey ) );
	CHECK_EQUAL( inferredLayout( sysHardRefKey ), LayoutOfSimpleKey( sysHardRefKey ) );
	CHECK_EQUAL( inferredLayout( sysSoftRefKey ), LayoutOfSimpleKey( sysSoftRefKey ) );
	CHECK_EQUAL( inferredLayout( sysWeakRefKey ), LayoutOfSimpleKey( sysWeakRefKey ) );
	CHECK_EQUAL( inferredLayout( sysReservedRefKey ), LayoutOfSimpleKey( sysReservedRefKey ) );
	CHECK_EQUAL( inferredLayout( sysCacheEqMapKey ), LayoutOfSimpleKey( sysCacheEqMapKey ) );
	CHECK_EQUAL( inferredLayout( sysHardEqMapKey ), LayoutOfSimpleKey( sysHardEqMapKey ) );
	CHECK_EQUAL( inferredLayout( sysHardIdMapKey ), LayoutOfSimpleKey( sysHardIdMapKey ) );
	CHECK_EQUAL( inferredLayout( sysWeakIdMapKey ), LayoutOfSimpleKey( sysWeakIdMapKey ) );
	CHECK_EQUAL( inferredLayout( sysHashMapDataKey ), LayoutOfSimpleKey( sysHashMapDataKey ) );
	CHECK_EQUAL( inferredLayout( sysUndefinedKey ), LayoutOfSimpleKey( sysUndefinedKey ) );
	CHECK_EQUAL( inferredLayout( sysElementKey ), LayoutOfSimpleKey( sysElementKey ) );
	CHECK_EQUAL( inferredLayout( sysAttrMapKey ), LayoutOfSimpleKey( sysAttrMapKey ) );
}

