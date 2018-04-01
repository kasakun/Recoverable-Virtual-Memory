/* basic.c - test that basic persistency works */

#include "rvm.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#define TEST_STRING "hello, world"

#define CRASH_INDEX 6

/* proc1 writes some data, commits it, then exits */
void proc1() 
{
    rvm_t rvm;
    trans_t trans;
    char* segs[1];
     
    rvm = rvm_init("rvm_segments");
    rvm_destroy(rvm, "testseg");
    segs[0] = (char *) rvm_map(rvm, "testseg", 10000);

    int offset = 0;
    int delta = strlen(TEST_STRING);
    for(int i = 0; i < 10; ++i){
	    if (i == CRASH_INDEX) break;
		trans = rvm_begin_trans(rvm, 1, (void **) segs);
        rvm_about_to_modify(trans, segs[0], offset, delta);
        sprintf(segs[0] + offset, TEST_STRING);
	    rvm_commit_trans(trans);
		offset += delta;
	}
	 
    abort();
}


/* proc2 opens the segments and reads from them */
void proc2() 
{
    char* segs[1];
    rvm_t rvm;
     
    rvm = rvm_init("rvm_segments");

    segs[0] = (char *) rvm_map(rvm, "testseg", 10000);

    char* test_string = TEST_STRING;
    int delta = strlen(TEST_STRING);

    char* target = (char*)malloc(delta * CRASH_INDEX + 1);
    memcpy(target, test_string, delta);
    target[delta] = '\0';

    for(int i = 1; i < CRASH_INDEX; ++i){
	    strcat(target, TEST_STRING);
	}
    target[CRASH_INDEX * delta] = '\0';
    segs[0][CRASH_INDEX * delta] = '\0';
    printf("target string is %s\n", target);
    printf("segs[0] is %s\n", segs[0]);
    if(strcmp(segs[0], target)){
        printf("string no matched!\n");
        exit(2);
    }
     
    printf("OK\n");
    exit(0);
}


int main(int argc, char **argv)
{
    int pid;

    pid = fork();
    if(pid < 0) {
	    perror("fork");
	    exit(2);
    }
    if(pid == 0) {
	    proc1();
	    exit(0);
    }

    waitpid(pid, NULL, 0);

    proc2();

    return 0;
}
