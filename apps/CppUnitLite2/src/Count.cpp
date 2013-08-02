#include "Count.h"

void Count::incrTotal() 
{
    this->num_total += 1;
}

void Count::incrFailed() 
{
    this->num_failed += 1;
}

int Count::total() const 
{
    return this->num_total;
}

int Count::failed() const 
{
    return this->num_failed;
}
