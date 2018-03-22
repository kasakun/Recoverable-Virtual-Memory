//
// Created by chenzy on 3/20/18.
//

#ifndef RECOVERY_VIRTUAL_MEMORY_RVM_H
#define RECOVERY_VIRTUAL_MEMORY_RVM_H

//////////////////////////////////////////////// Structure ///////////////////////////////////////////////////////////
typedef struct __rvm_t {

} rvm_t;

typedef struct __trans_t {

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
void* rvm_map (rvm_t rvm, const char* directory);

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
 * begin a transaction that will modify the segments listed in segbases . If any of the specifed segments is already
 * being modifed by a transaction, then the call should fail and return (trans_t) -1
 */
trans_t rvm_begin_trans(rvm_t rvm, int numsegs, void **segbases);

/*
 * declare that the library is about to modify a specifed range of memory in the specifed segment.The segment must be
 * one of the segments specified in the call to rvm_begin_trans. Your library needs to ensure that the old memory has
 * been saved, in case an abort is executed. It is legal call rvm_about_to_modify multiple times on the same memory
 * area.
 */
void rvm_about_to_modify(trans_t tid, void *segbase, int ofset, int size);

#endif //RECOVERY_VIRTUAL_MEMORY_RVM_H
