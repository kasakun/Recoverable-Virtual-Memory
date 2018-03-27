//
// Created by chenzy on 3/20/18.
//

#ifndef RECOVERY_VIRTUAL_MEMORY_RVM_H
#define RECOVERY_VIRTUAL_MEMORY_RVM_H

#include <list>
//////////////////////////////////////////////// Structures ///////////////////////////////////////////////////////////

/*
 * segment structure
 */
typedef struct __segment {
    char* segname;  // name of segment = name of the backup file for this segment 
    int fd;  // file descriptor for external data segment, i.e. the backup file
    int logfd;  // file descriptor for log segment
    int size;
    //int locked;
    //int isMap;

    void* data;
} segment_t;

/*
 * region structure
 */
typedef struct __region {
    int offset;  // with regard to the memory base of a segment 
    int size;

    void* old_data;  // store old data of a memory region into another memory region
} region_t;


/* 
 * transaction structure
 */
typedef int trans_t;

/* 
 * segment node structure
 */
typedef struct __segment_node{
	segment_t* segment;
	trans_t tid;  // a segment can only be modified by a transaction at a time
	std::list<region_t> regions;  // multiple regions of this segment may be modified by a transaction
} segment_node_t;


/*
 * structure for rvm
 */
typedef struct __rvm {
    char* directory;   // backing directory
    std::list<segment_node_t>* segment_list;
} rvm_t;


//////////////////////////////////////////////// Library APIs /////////////////////////////////////////////////////////////
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



//////////////////////////////////////////////// Utility Functions /////////////////////////////////////////////////////////////

/*
 * LOG file writing
 */
void rvm_log(int logfd, char* log);


/*
 * write new data (of specific regions of a segment) into the (redo) log file associated with this segment
 */
int write_segment_to_log(segment_node_t seg_node);

/*
 * read the (redo) log file associated with this segment
 */
int restore_segment_from_log(void* data, char* log_path);


/*
 * reconstruct log path from rvm (directory) and segment (segment name) 
 */
char* reconstruct_log_path(rvm_t rvm, segment_t* seg);


/*
 * reset a segment node's transaction ID to -1 if its original transaction ID is tid
 */
void reset_segment_tid(trans_t tid);


/*
 * print information of the global segment list
 */
void print_segment_list();


/*
 * check if a file is a log file 
 */
int is_log_file(char* file_path);


/*
 * generate a path to the specified file under a specified directory
 */
char* concat_dir_file(char* dir, char* file);


int get_segment_size(rvm_t rvm, char* seg_name);

#endif //RECOVERY_VIRTUAL_MEMORY_RVM_H
