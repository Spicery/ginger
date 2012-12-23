#include <iostream>
#include <cppunit/TestAssert.h>

#include "simplekeytest.hpp"

#include "common.hpp"
#include "key.hpp"
#include "vectorlayout.hpp"

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
		default:
			return KEYLESS_LAYOUT;
	}
}


void SimpleKeyTest::testOK() {
	CPPUNIT_ASSERT( inferredLayout( sysAbsentKey ) == LayoutOfSimpleKey( sysAbsentKey ) );
	CPPUNIT_ASSERT( inferredLayout( sysBoolKey ) == LayoutOfSimpleKey( sysBoolKey ) );
	CPPUNIT_ASSERT( inferredLayout( sysClassKey ) == LayoutOfSimpleKey( sysClassKey ) );
	CPPUNIT_ASSERT( inferredLayout( sysTerminKey ) == LayoutOfSimpleKey( sysTerminKey ) );
	CPPUNIT_ASSERT( inferredLayout( sysNilKey ) == LayoutOfSimpleKey( sysNilKey ) );
	CPPUNIT_ASSERT( inferredLayout( sysPairKey ) == LayoutOfSimpleKey( sysPairKey ) );
	CPPUNIT_ASSERT( inferredLayout( sysVectorKey ) == LayoutOfSimpleKey( sysVectorKey ) );
	CPPUNIT_ASSERT( inferredLayout( sysStringKey ) == LayoutOfSimpleKey( sysStringKey ) );
	CPPUNIT_ASSERT( inferredLayout( sysSymbolKey ) == LayoutOfSimpleKey( sysSymbolKey ) );
	CPPUNIT_ASSERT( inferredLayout( sysSmallKey ) == LayoutOfSimpleKey( sysSmallKey ) );
	CPPUNIT_ASSERT( inferredLayout( sysFloatKey ) == LayoutOfSimpleKey( sysFloatKey ) );
	CPPUNIT_ASSERT( inferredLayout( sysUnicodeKey ) == LayoutOfSimpleKey( sysUnicodeKey ) );
	CPPUNIT_ASSERT( inferredLayout( sysCharKey ) == LayoutOfSimpleKey( sysCharKey ) );
	CPPUNIT_ASSERT( inferredLayout( sysMapletKey ) == LayoutOfSimpleKey( sysMapletKey ) );
	CPPUNIT_ASSERT( inferredLayout( sysIndirectionKey ) == LayoutOfSimpleKey( sysIndirectionKey ) );
	CPPUNIT_ASSERT( inferredLayout( sysAssocKey ) == LayoutOfSimpleKey( sysAssocKey ) );
	CPPUNIT_ASSERT( inferredLayout( sysHardRefKey ) == LayoutOfSimpleKey( sysHardRefKey ) );
	CPPUNIT_ASSERT( inferredLayout( sysSoftRefKey ) == LayoutOfSimpleKey( sysSoftRefKey ) );
	CPPUNIT_ASSERT( inferredLayout( sysWeakRefKey ) == LayoutOfSimpleKey( sysWeakRefKey ) );
	CPPUNIT_ASSERT( inferredLayout( sysReservedRefKey ) == LayoutOfSimpleKey( sysReservedRefKey ) );
	CPPUNIT_ASSERT( inferredLayout( sysCacheEqMapKey ) == LayoutOfSimpleKey( sysCacheEqMapKey ) );
	CPPUNIT_ASSERT( inferredLayout( sysHardEqMapKey ) == LayoutOfSimpleKey( sysHardEqMapKey ) );
	CPPUNIT_ASSERT( inferredLayout( sysHardIdMapKey ) == LayoutOfSimpleKey( sysHardIdMapKey ) );
	CPPUNIT_ASSERT( inferredLayout( sysWeakIdMapKey ) == LayoutOfSimpleKey( sysWeakIdMapKey ) );
	CPPUNIT_ASSERT( inferredLayout( sysHashMapDataKey ) == LayoutOfSimpleKey( sysHashMapDataKey ) );
	CPPUNIT_ASSERT( inferredLayout( sysUndefinedKey ) == LayoutOfSimpleKey( sysUndefinedKey ) );
	CPPUNIT_ASSERT( inferredLayout( sysElementKey ) == LayoutOfSimpleKey( sysElementKey ) );
	CPPUNIT_ASSERT( inferredLayout( sysAttrMapKey ) == LayoutOfSimpleKey( sysAttrMapKey ) );
}

CPPUNIT_TEST_SUITE_REGISTRATION( SimpleKeyTest );
