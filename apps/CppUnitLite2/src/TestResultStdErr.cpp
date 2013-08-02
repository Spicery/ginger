#include "TestResultStdErr.h"
#include "Failure.h"
#include <iostream>
#include <iomanip>

static const char * HEADER_LINE = "+-- Unit Test Results -----------------------------------------------------";
static const char * RULER       = "+- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - ";
static const char * FOOTER_LINE = "+--------------------------------------------------------------------------";
static const char * FAIL_PREFIX = "| [X] ";
static const char * OK_PREFIX = "| [ ] ";
static const char * SUMMARY_PREFIX = "| ";

void TestResultStdErr::AddFailure (const Failure & failure) 
{
    TestResult::AddFailure(failure);
    std::cerr << failure << std::endl;
}

void TestResultStdErr::EndTests () 
{
    TestResult::EndTests();

    std::cerr << HEADER_LINE << std::endl;

    bool groups = false;
    for ( TestResult::iterator it = this->m_group_count.begin(); it != this->m_group_count.end(); ++it, groups = true ) {
    	const std::string& group_name = it->first;
    	const Count& count = it->second;
    	std::cerr << ( count.failed() > 0 ? FAIL_PREFIX : OK_PREFIX );
    	std::cerr << group_name << ": ";
    	if ( count.failed() > 0 ) {
    		std::cerr << count.failed() << " of " << count.total() << " tests failed";
    	} else {
    		std::cerr << count.total() << " test" << ( count.total() != 1 ? "s" : "" ) << " passed";
    	}
    	std::cerr << std::endl;
    }

    if ( groups ) {
        std::cerr << RULER << std::endl;
    }
    std::cerr << SUMMARY_PREFIX << m_count.total() << " tests run (in ";
    std::cerr << std::setprecision(3) << m_secondsElapsed << "s)" << std::endl;
    if ( m_count.failed() > 0 ) {
        std::cerr << SUMMARY_PREFIX  << "****** There " << ( m_count.failed() == 1 ? "was" : "were" ) << " " << m_count.failed() << " failure" << ( m_count.failed() == 1 ? "" : "s" ) << "." << std::endl;
    } else {
        std::cerr << SUMMARY_PREFIX  << "There were no test failures." << std::endl;
    }
    std::cerr << FOOTER_LINE << std::endl;

}

