#include <stdlib.h>
#include <stdio.h>

#include "rvm.h"


int main(int argc, char** argv){
    rvm_t rvm;
    trans_t tid;

	char* segs[1];

	char* test_string1 = "hello";
	int offset1 = 0;
	int size1 = 10;

	rvm_verbose(1);

	rvm = rvm_init("RVM_trunc");  // directory name cannot be "RVM_truncate"!
	rvm_destroy(rvm, "testseg");

	segs[0] = (char*)rvm_map(rvm, "testseg", 4096);

	tid = rvm_begin_trans(rvm, 1, (void**)segs);

	rvm_about_to_modify(tid, segs[0], offset1, size1);
	sprintf(segs[0] + offset1, test_string1);

	rvm_commit_trans(tid);
	
	rvm_truncate_log(rvm);

	rvm_unmap(rvm, segs[0]);

	return 0;
}