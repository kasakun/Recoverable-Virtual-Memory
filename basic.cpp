
// Created by Yaohong Wu on 3/26/18.


// #include <unistd.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <sys/wait.h>

// #include "rvm.h"


// #define DIRECTORY "RVM_basic"

// #define TEST_STRING "CP3 MVP"
// #define OFFSET1 0
// #define OFFSET2 100

// /* proc1 writes some data, commits it, then exits */
// void proc1()
// {
    // rvm_t rvm;
    // trans_t trans;
    // char* segs[1];
    
    // rvm = rvm_init(DIRECTORY);

    // rvm_destroy(rvm, "testseg");
  
    // segs[0] = (char *)rvm_map(rvm, "testseg", 1024);

    // trans = rvm_begin_trans(rvm, 1, (void**)segs);
     
    // rvm_about_to_modify(trans, segs[0], OFFSET1, 100);
    // sprintf(segs[0] + OFFSET1, TEST_STRING);
     
    // rvm_about_to_modify(trans, segs[0], OFFSET2, 100);
    // sprintf(segs[0] + OFFSET2, TEST_STRING);
     
    // rvm_commit_trans(trans);

	// exit(0);
// }


// /* proc2 opens the segments and reads from them */
// void proc2() 
// {
    // char* segs[1];
    // rvm_t rvm;
     
    // rvm = rvm_init(DIRECTORY);

    // segs[0] = (char *)rvm_map(rvm, "testseg", 1024);
    // if(strcmp(segs[0] + OFFSET1, TEST_STRING)) {
        // printf("ERROR: first test string not present\n");
		// exit(-1);
	// }
    // if(strcmp(segs[0] + OFFSET2, TEST_STRING)) {
	    // printf("ERROR: second test string not present\n");
	    // exit(-1);
    // }

    // printf("I read the committed data!\n");
    // exit(0);
// }


// int main(int argc, char **argv)
// {
    // int pid;
    // pid = fork();
    // if(pid < 0) {
	    // perror("fork");
	    // exit(-1);
    // }
    
	// if(pid == 0) {
	    // proc1();
    // }

    // waitpid(pid, NULL, 0);  // wait for its child to change states (i.e. child terminated in this case)

    // proc2();

    // return 0;
// }