//
// Created by chenzy on 3/20/18.
//

#ifndef RECOVERY_VIRTUAL_MEMORY_RVM_H
#define RECOVERY_VIRTUAL_MEMORY_RVM_H

#include <list>
//////////////////////////////////////////////// Structure ///////////////////////////////////////////////////////////

/*
 * segment structure
 */
typedef struct __segment {
    char* segname;
    int fd;
    int logfd;
    int size;
    int locked;
    int isMap;

    void* data;

} segment;

/*
 * store history data form the log
 */
typedef struct __region {
    int offset;
    int size;

    void* origindata;
} region;

/*
 * structure for one log file
 */
typedef struct __rvm_t {
    char* directory;   // backing directory
    std::list<segment*>* rvm_seg;
} rvm_t;

/*
 * structure for one transaction
 */
typedef struct __trans_t {
    unsigned int transID;
    unsigned int numseges;
    rvm_t rvm;
    segment** segs;
} trans_t;


//////////////////////////////////////////////// Declare /////////////////////////////////////////////////////////////
/*
 * Initialize the library with the specified directory as backing store
 */
rvm_t rvm_init (const char* directory);

/*
 *  map a segment from disk into memory.
 *  If the segment does not already exist, then create it and give it size size_to_create . If the segment exists but is
 *  shorter than size_to_create , then extend it until it is long enough.
 *  It is an error to try to map the same segment twice.
 */
void* rvm_map (rvm_t rvm, const char *segname, int size_to_create);

/*
 * unmap a segment from memory
 */
void rvm_unmap (rvm_t rvm, void* segbase);

/*
 * destroy a segment completely, erasing its backing store. This function should not be called on a segment that is
 * currently mapped.
 */
void rvm_destroy(rvm_t rvm, const char *segname);

/*
 * begin a transaction that will modify the segments listed in segbases . If any of the specified segments is already
 * being modified by a transaction, then the call should fail and return (trans_t) -1
 */
trans_t rvm_begin_trans(rvm_t rvm, int numsegs, void **segbases);

/*
 * declare that the library is about to modify a specified range of memory in the specified segment.The segment must be
 * one of the segments specified in the call to rvm_begin_trans. Your library needs to ensure that the old memory has
 * been saved, in case an abort is executed. It is legal call rvm_about_to_modify multiple times on the same memory
 * area.
 */
void rvm_about_to_modify(trans_t tid, void *segbase, int offset, int size);

/*
 * commit all changes that have been made within the specified transaction. When the call returns, then enough
 * information should have been saved to disk so that, even if the program crashes, the changes will be seen by the
 * program when it restarts.
 */
void rvm_commit_trans(trans_t tid);

/*
 * undo all changes that have happened within the specified transaction.
 */
void rvm_abort_trans(trans_t tid);

/*
 * play through any committed or aborted items in the log files and shrink the log files as much as possible.
 */
void rvm_truncate_log(rvm_t rvm);

/*
 * If the value passed to this function is nonzero, then verbose output from your library is enabled. If the value
 * passed to this function is zero, then verbose output from your library is disabled. When verbose output is enabled,
 * your library should print information about what it is doing.
 */
void rvm_verbose(int enable_flag);

/*
 * LOG file writing
 */

void rvm_log(int logfd, char* log);

#endif //RECOVERY_VIRTUAL_MEMORY_RVM_H
