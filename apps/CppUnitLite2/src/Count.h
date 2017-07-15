#ifndef CPPUNITLITE_COUNT_H
#define CPPUNITLITE_COUNT_H

class Count {
public:
    Count() 
        : num_total( 0 )
        , num_failed( 0 )
    {}
public:
    void    incrTotal(); 
    void    incrFailed();
    int     total() const;
    int     failed() const;
private:
    int     num_total;
    int     num_failed;
};

#endif
