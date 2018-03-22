//
// Created by chenzy on 3/20/18.
//
#include <cstdio>
#include "rvm.h"

#define FLAG 1

int  main(int argc, char* argv[]) {
    rvm_t rvm;
    trans_t tr;
    char* seg;
    char* segarr[10];

    // verbose switch
    rvm_verbose(FLAG);

    // create back repo
    rvm = rvm_init("RVMrepo");
    // create and map seg
    seg = (char*) rvm_map(rvm, "seg0", 100);
    sprintf(seg, "HELLO");

    // unmap, delete segment in memory
    rvm_unmap(rvm, seg);

    // Destroy back file
    rvm_destroy(rvm, "seg0");
    return 0;
}