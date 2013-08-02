#include "TestResult.h"
#include "Failure.h"
#include "Test.h"
#include <ctime>

TestResult::TestResult() 
    : m_secondsElapsed( 0.0f )
{
    m_startTime = clock();
}

TestResult::~TestResult()
{
}

#define GROUP_SEP "__"

static const char * getGroup( const char * test_name ) {
	std::string name( test_name );
    size_t pos = name.find( GROUP_SEP );
    if ( pos != std::string::npos ) {
    	return test_name + pos + sizeof( GROUP_SEP ) - 1;
    } else {
    	return NULL;
    }
}

typedef std::map< std::string, Count > GroupCount;

static void bumpTotal( const char * test_name, GroupCount& gc ) {
	const char * g = getGroup( test_name );
    if ( g != NULL ) {
   		gc[ std::string( g ) ].incrTotal();
    }
}

static void bumpFailed( const char * test_name, GroupCount& gc ) {
	const char * g = getGroup( test_name );
    if ( g != NULL ) {
   		gc[ std::string( g ) ].incrFailed();
    }
}
 
void TestResult::TestWasRun( const Test& test )
{
    m_count.incrTotal();
    bumpTotal( test.TestName(), this->m_group_count );
}

void TestResult::StartTests () 
{
}

void TestResult::AddFailure( const Failure & failure ) 
{
    m_count.incrFailed();
    bumpFailed( failure.TestName(), this->m_group_count );
}

void TestResult::EndTests () 
{
    m_secondsElapsed = (clock() - m_startTime)/float(CLOCKS_PER_SEC);
}

int TestResult::FailedCount() const 
{ 
    return this->m_count.failed(); 
}

int TestResult::TestCount() const 
{ 
    return this->m_count.total(); 
}

