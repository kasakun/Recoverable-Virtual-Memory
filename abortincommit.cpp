#include "rvm.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINE_MAX 10000
#define TEST_STRING1 "hello, world"
#define VIRUS "FQHFADA21341QOIFHANFOH2AF121RFCQWFAEG24GQ3AGDFVVSZD1ASF"
#define OFFSET2 1000
#define OFFSET3 2000

int main(int argc, char **argv)
{
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
    sprintf(seg+OFFSET3, VIRUS);

    rvm_commit_trans(trans); // Will abort while writing virus code



    /* Check log file */

    FILE* log_file;
    char line[LINE_MAX];
    log_file = fopen ("./rvm_segments/testseg.log", "r");
    while(fgets(line, LINE_MAX, log_file) != NULL) {
        printf("%s", line);
    }

    rvm_unmap(rvm, seg);
    printf("OK\n");
    exit(0);
}
