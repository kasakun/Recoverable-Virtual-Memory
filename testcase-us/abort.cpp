//
// Created by chenzy on 3/20/18.
//
#include <stdio.h>
#include "rvm.h"

#define FLAG 1


int main(int argc, char* argv[]) {
    rvm_t rvm;
    trans_t tid1, tid2;

    char* segs[1];
    char* test_string1 = "hello";
	char* test_string2 = "world";
	char* test_string3 = "Yaohong Wu";

	char* test_string4 = "HELLO";  // different from test_string1
	char* test_string5 = "WORLD";  // different from test_string2

    int offset1 = 0, offset2 = 25, offset3 = 60;
	int size1 = 10, size2 = 10, size3 = 15;


    // verbose switch
    rvm_verbose(FLAG);

    // create back repo
    rvm = rvm_init("RVM_abort");

    rvm_destroy(rvm, "testseg");

    // create and map seg
    segs[0] = (char*)rvm_map(rvm, "testseg", 100);

	// first transaction, end using commit
	tid1 = rvm_begin_trans(rvm, 1, (void**)segs);

	rvm_about_to_modify(tid1, segs[0], offset1, size1);
	sprintf(segs[0] + offset1, test_string1);

	rvm_about_to_modify(tid1, segs[0], offset2, size2);
    sprintf(segs[0] + offset2, test_string2);

	rvm_about_to_modify(tid1, segs[0], offset3, size3);
	sprintf(segs[0] + offset3, test_string3);

	rvm_commit_trans(tid1);

	// second transaction, end usng abort
	tid2 = rvm_begin_trans(rvm, 1, (void**)segs);

	rvm_about_to_modify(tid2, segs[0], offset1, size1);
	sprintf(segs[0] + offset1, test_string4);

	rvm_about_to_modify(tid2, segs[0], offset2, size2);
    sprintf(segs[0] + offset2, test_string5);

	rvm_abort_trans(tid2);

	// unmap, delete segment in memory
    // rvm_unmap(rvm, segs[0]);

	// map again
	// segs[0] = (char*)rvm_map(rvm, "testseg", 100);

    // check changes
	printf("first string is %s\n", segs[0] + offset1);
	printf("second string is %s\n", segs[0] + offset2);
	printf("third string is %s\n", segs[0] + offset3);

	// unmap, delete segment in memory
    rvm_unmap(rvm, segs[0]);

    // Destroy back file
    // rvm_destroy(rvm, "seg0");
    return 0;
}