#include "rvm.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wait.h>

#define LINE_MAX 10000
#define TEST_STRING1 "hello, world"
#define TEST_STRING2 "blablablabla"
#define VIRUS "Fgdsg3ADA21341QOIdfggagaeNFOH2AF12agrg31FCQWhsdf"
#define OFFSET2 1000
#define OFFSET3 2000
#define OFFSET4 3000
void proc1() {
    rvm_t rvm;
    char *seg;
    char *segs[3];
    trans_t trans;


    rvm = rvm_init("rvm_segments");

    rvm_destroy(rvm, "testseg");

    segs[0] = (char *) rvm_map(rvm, "testseg", 10000);
    seg = segs[0];

    /* write some data and commit it */
    trans = rvm_begin_trans(rvm, 1, (void**) segs);
    rvm_about_to_modify(trans, seg, 0, 100);
    sprintf(seg, TEST_STRING1);

    rvm_about_to_modify(trans, seg, OFFSET2, 100);
    sprintf(seg+OFFSET2, TEST_STRING1);


    rvm_commit_trans(trans); // Will abort while writing virus code

    trans = rvm_begin_trans(rvm, 1, (void**) segs);
    rvm_about_to_modify(trans, seg, OFFSET3, 100);
    sprintf(seg + OFFSET3, TEST_STRING2);

    rvm_about_to_modify(trans, seg, OFFSET4, 100);
    sprintf(seg+OFFSET4,  VIRUS);

    rvm_commit_trans(trans); // Will abort while writing virus code
    rvm_unmap(rvm, seg);
    exit(0);
}

void proc2() {
    /* Check log file */
    FILE* log_file;
    rvm_t rvm;
    char* newseg;
    char line[LINE_MAX];
    log_file = fopen ("./rvm_segments/testseg.log", "r");

    rvm = rvm_init("rvm_segments");

    //
    printf("Go back to last committed state.\n");

    printf("Log file before mapping. \n");
    printf("==================================\n");
    while(fgets(line, LINE_MAX, log_file) != NULL) {
        printf("%s", line);
    }
    printf("\n", line);

    newseg = (char *) rvm_map(rvm, "testseg", 10000);


    if(strcmp(newseg+OFFSET2, TEST_STRING1)) {
        printf("ERROR: 2 is incorrect (%s)\n",
               newseg+OFFSET2);
        exit(2);
    }

    if(strcmp(newseg, TEST_STRING1)) {
        printf("ERROR: first hello is incorrect (%s)\n",
               newseg);
        exit(2);
    }
    rvm_unmap(rvm, newseg);

    printf("==================================\n");
    printf("The complete lines are correct.\n");

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
