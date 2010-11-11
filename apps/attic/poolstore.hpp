#ifndef POOLSTORE_H
#define POOLSTORE_H

#include "pool.hpp"

class PoolStoreClass {
public:
        Pool    permanent;              // shared between tasks
        Pool    main;           // lifetime of the task
        Pool    scratch;                // transient
};

typedef PoolStoreClass  *PoolStore;

#endif
