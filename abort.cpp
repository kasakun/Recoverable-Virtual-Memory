//
// Created by chenzy on 3/20/18.
//
#include <cstdio>
#include "rvm.h"

#define FLAG 1

int main(int argc, char* argv[]) {
    rvm_t rvm;
    trans_t tid;
	
    char* segs[1];
    char* test_string1 = "hello";
	char* test_string2 = "world";
	char* test_string3 = "Yaohong Wu";
	
    int offset1 = 0, offset2 = 25, offset3 = 60;
	int size1 = 10, size2 = 10, size3 = 15;
	
	
    // verbose switch
    rvm_verbose(FLAG);

    // create back repo
    rvm = rvm_init("RVMrepo");

    rvm_destroy(rvm, "seg0");

    // create and map seg
    segs[0] = (char*)rvm_map(rvm, "seg0", 100);
    
	tid = rvm_begin_trans(rvm, 1, (void**)segs);
	
	rvm_about_to_modify(tid, segs[0], offset1, size1);
	sprintf(segs[0] + offset1, test_string1);
	
	rvm_about_to_modify(tid, segs[0], offset2, size2);
    sprintf(segs[0] + offset2, test_string2);
	
	rvm_about_to_modify(tid, segs[0], offset3, size3);
	sprintf(segs[0] + offset3, test_string3);
	
	rvm_commit_trans(tid);
	
    // unmap, delete segment in memory
    rvm_unmap(rvm, segs[0]);

	// map again
	segs[0] = (char*)rvm_map(rvm, "seg0", 100);
	printf("first string is %s\n", segs[0] + offset1);
	printf("second string is %s\n", segs[0] + offset2);
	printf("third string is %s\n", segs[0] + offset3);

    // Destroy back file
    //rvm_destroy(rvm, "seg0");
    return 0;
}