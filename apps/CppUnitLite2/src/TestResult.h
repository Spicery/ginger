
#ifndef TESTRESULT_H
#define TESTRESULT_H

#include <map>
#include <string>

#include "Count.h"

class Failure;
class Test;



class TestResult
{
public:
    typedef std::map< std::string, Count >::iterator iterator;

public:
    TestResult ();
    virtual ~TestResult();

    virtual void TestWasRun ( const Test& test );
    virtual void StartTests ();
    virtual void AddFailure (const Failure & failure);
    virtual void EndTests ();

    int FailedCount() const;
    int TestCount() const;

protected:
    Count m_count;
    std::map< std::string, Count > m_group_count;
    long int m_startTime;
    float m_secondsElapsed;
};

#endif
