//
// Created by chenzy on 3/20/18.
//

#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <iostream>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>

#include "rvm.h"

#define COMMIT "Commit"
#define SEPERATOR "|"

static int flag = 0;  // verbose output by default
static unsigned int global_tid = 0;  // global counter for transaction ID
std::list<segment_node_t>* segment_list = new std::list<segment_node_t>;  // global segment list

static char* log_file_ext = ".log";

rvm_t rvm_init (const char* directory) {
    rvm_t newrvm;
    int dirlen;

    mkdir(directory, 0755);
    dirlen = strlen(directory);

    if(flag)
        std::cout << "RVM: " << directory << " starts creating." << std::endl;

    newrvm.directory = (char*) malloc (dirlen* sizeof(char));
    strcpy(newrvm.directory, directory);

    newrvm.segment_list = segment_list;  // global segment list

    if(flag)
        std::cout << "RVM: " << directory << " created." << std::endl;

    return newrvm;
}

// Map the file into the rvm
void* rvm_map (rvm_t rvm, const char *segname, int size_to_create) {
    int fd, logfd, size, check, segnamelen;
    char log[512];
    void* newdata;
    segment_t* seg;

    // checking if the segname is in the list
    if (flag)
        std::cout << "RVM: " << rvm.directory << " " << segname << " checking if multi-mapping." << std::endl;
    for (std::list<segment_node_t>::iterator iterator = rvm.segment_list->begin(); iterator != rvm.segment_list->end(); ++iterator) {
        if ((*iterator).segment->segname == segname){
            std::cout << "RVM: " << rvm.directory << " other segment is mapping. Abort." << std::endl;
            return NULL;
        }
    }

    if (flag)
        std::cout << "RVM: " << rvm.directory << " starts mapping." << std::endl;
    // log: dir info
    sprintf (log, "%s/%s", rvm.directory, segname);


    if (flag)
        std::cout << "RVM: " << rvm.directory << " starts creating " << segname << " on the disk." << std::endl;

    // Open or create the back files
    fd = open(log, O_RDWR | O_CREAT, 0755);
    if (fd == -1) {
        std::cout << "Error: Fail to open/create back files." << std::endl;
        return NULL;
    }

    if (flag)
        std::cout << "RVM: " << rvm.directory << " starts creating " << segname << " log on the disk." << std::endl;
    strcat (log, ".log");
    // Open or create the log for back files
    logfd = open(log, O_RDWR | O_CREAT, 0755);
    if (logfd == -1) {
        std::cout << "Error: Fail to open/create log files." << std::endl;
        return NULL;
    }

    // Get size of the current file
    if (flag)
        std::cout << "RVM: " << rvm.directory << " getting size of " << segname << "." << std::endl;

    size = lseek (fd, 0, SEEK_END);

    if (size == -1) {
        close(fd);
        std::cout << "Error: Fail to open file, " << strerror(errno) << std::endl;
		return NULL;  // -Yaohong Wu
    }

    if (size < size_to_create) {
        if (flag)
            std::cout << "RVM: " << rvm.directory << " expanding " << segname << "." << std::endl;
        // set hole
        check = lseek (fd, size_to_create - 1, SEEK_SET);

        if (check == -1) {
            close(fd);
            std::cout << " Error: Fail to expand file, " << strerror(errno) << std::endl;
            return NULL;
        }
    }

    if (flag)
        std::cout << "RVM: " << rvm.directory << " malloc " << segname << "." << std::endl;
    newdata = (void*) malloc (size_to_create* sizeof(char));

    // Copy data from disk to memory
    if (flag)
        std::cout << "RVM: " << rvm.directory << " syncing data to 0x" << std::hex << newdata << "." << std::endl;

    lseek (fd, 0, SEEK_SET);
    check = read (fd, newdata, size_to_create);  // also need to restore from log -Yaohong

    if (check == -1) {
        close(fd);
        std::cout << " Error: Fail to sync, " << strerror(errno) << std::endl;
        return NULL;
    }

    // create segment info
    if (flag)
        std::cout << "RVM: " << rvm.directory << " " << segname << " creating info." << std::endl;

    seg = (segment_t*) malloc(sizeof(segment_t));
    segnamelen = strlen(segname);

    //seg->segname = NULL;
    seg->segname = (char*) malloc (segnamelen*sizeof(char));
    strcpy(seg->segname, segname);

    seg->data = newdata;
    seg->size = size_to_create;
    seg->fd = fd;
    seg->logfd = logfd;
    //seg->isMap = 1; // 1 for map state

    seg->locked = 0;

	// restore from log -Yaohong
	char* log_path = reconstruct_log_path(rvm, seg);
	if(restore_segment_from_log(seg, log_path) == -1){
	    printf("error in restoring segment from log!\n");
	}
	
    // Add new segment to rvm
    if (flag)
        std::cout << "RVM: " << rvm.directory << " " << segname << " saving info." << std::endl;
	
	segment_node_t seg_node;
	seg_node.segment = seg;
	seg_node.tid = -1;
	
    rvm.segment_list->push_back(seg_node);

    sprintf(log, "RVM map %s to memory 0x%x, size: %d. \n", segname, newdata, size_to_create);

    if (flag)
        std::cout << "RVM: " << rvm.directory  << " " << segname << " logging." << std::endl;

    //rvm_log(seg->logfd, log);
    if (flag)
        std::cout << "RVM: " << rvm.directory  << " " << segname << " map success, name: " << segname << " start address: 0x"
                  << std::hex << newdata << std::endl;
    return newdata;
}


void rvm_log(int logfd, char* log) {
    int bytes = write(logfd, log, strlen(log));

    if (bytes == -1) {
        close(logfd);
        std::cout << "Error: Fail to write to log, " << strerror(errno) << "." << std::endl;
        return;
    }

    fsync(logfd);
}

void rvm_unmap(rvm_t rvm, void *segbase){
    for (std::list<segment_node_t>::iterator iterator = rvm.segment_list->begin(); iterator != rvm.segment_list->end(); ++iterator) {
        if ((*iterator).segment->data == segbase){

            //std::cout << "segment list size = " << rvm.segment_list->size() << std::endl;
            if (flag)
                std::cout << "RVM: " << rvm.directory << " " << (*iterator).segment->segname << " " << "0x" << std::hex
                          << segbase <<" starts unmapping." << std::endl;
            char log[1024];
            sprintf(log, "RVM: %s unmap %s from memory 0x%x", rvm.directory, (*iterator).segment->segname, segbase);
            //rvm_log((*iterator).segment->logfd, log);

            if (flag)
                std::cout << "RVM: " << rvm.directory << " closing " << (*iterator).segment->segname << "." << std::endl;
            close((*iterator).segment->fd);
            close((*iterator).segment->logfd);

            if (flag)
                std::cout << "RVM: " << rvm.directory << " free memory." << std::endl;
            free((*iterator).segment->segname);
            free(segbase);

            rvm.segment_list->erase(iterator);
            if (flag)
                std::cout << "RVM: " << rvm.directory << " unmap success." << std::endl;

            break;
        }
    }
}

void rvm_destroy(rvm_t rvm, const char *segname){
    char log[1024];

    // check if the seg is in mapping
    if (flag)
        std::cout << "RVM: " << rvm.directory << " " << segname << " checking if in mapping." << std::endl;
    for (std::list<segment_node_t>::iterator iterator = rvm.segment_list->begin(); iterator != rvm.segment_list->end(); ++iterator) {
        if ((*iterator).segment->segname == segname){
            std::cout << "RVM: " << rvm.directory << " " << segname << " is mapping. Fail to destroy." << std::endl;
            return;
        }
    }

    if (flag)
        std::cout << "RVM: " << rvm.directory << " removing " << segname << "." << std::endl;
    sprintf(log, "%s/%s", rvm.directory, segname);
    remove(log);

    if (flag)
        std::cout << "RVM: " << rvm.directory << " removing " << segname << ".log." << std::endl;
    sprintf(log, "%s/%s.log", rvm.directory, segname);
    remove(log);
}

void rvm_verbose(int FLAG) {
    if (FLAG != 0 && FLAG != 1) {
        std::cout << "Wrong flag!" << std::endl;
    }
    flag = FLAG;
    return;
}

trans_t rvm_begin_trans(rvm_t rvm, int numsegs, void **segbases){
	if(flag)
		printf("RVM: %s, begin transaction on %d segments\n", rvm.directory, numsegs);
	
	for(int i = 0; i < numsegs; ++i){
	    for(std::list<segment_node_t>::iterator it = rvm.segment_list->begin(); it != rvm.segment_list->end(); ++it){
		    
			if((*it).segment->data == segbases[i]){  
			    if((*it).tid != -1){ // current segment is being modified by another transaction
				    printf("current segment is being modified by another transaction\n");
					return (trans_t)-1;
				}
			}
		}
	}
	
	trans_t tid = global_tid++;
	
	for(int i = 0; i < numsegs; ++i){
	    for(std::list<segment_node_t>::iterator it = segment_list->begin(); it != segment_list->end(); ++it){
			if((*it).segment->data == segbases[i]){  
			    (*it).tid = tid;
				//printf("(*it).tid = %d\n", (*it).tid);
			}
		}
	}
	
    return tid;
}





void rvm_about_to_modify(trans_t tid, void* segbase, int offset, int size){
    if(flag)
		printf("RVM: about to modify segment 0x%x with offset %d and size %d for transaction %d\n", segbase, offset, size, tid);
	
	for(std::list<segment_node_t>::iterator it = segment_list->begin(); it != segment_list->end(); ++it){
	    
		if((*it).tid == tid && (*it).segment->data == segbase){
			// store old data in somewhere else in memory
			void* old_data = malloc(sizeof(char) * size);
			memcpy(old_data, segbase + offset, size);
			
			// create a region object and insert it into the region list of the segment
			region_t region;
			region.offset = offset;
			region.size = size;
			region.old_data = old_data;
			
			(*it).regions.push_back(region);
		}
	}
}

void rvm_commit_trans(trans_t tid){
    if(flag)
		printf("RVM: commit transaction with ID %d\n", tid);
	
	for(std::list<segment_node_t>::iterator it = segment_list->begin(); it != segment_list->end(); ++it){
		if((*it).tid == tid){
			// write segment data to log segment in disk
			if(write_segment_to_log((*it)) == -1){
			    printf("error in writing segment to log\n");
			}
			
			// throw away old data
			for(std::list<region_t>::iterator it_region = (*it).regions.begin(); it_region != (*it).regions.end(); ++it_region){
			    free((*it_region).old_data);
			}
		}
	}
		
	// reset segment's tid to -1 if it is committed to disk
	reset_segment_tid(tid);
}


int write_segment_to_log(segment_node_t seg_node){
    segment_t* seg = seg_node.segment;
	int logfd = seg->logfd;
	
	char buf[4096] = COMMIT;  // hard code
	for(std::list<region_t>::iterator it = seg_node.regions.begin(); it != seg_node.regions.end(); ++it){
	    strcat(buf, SEPERATOR);
		region_t r = (*it);
		char tmp_buf[256];  // hard code
		sprintf(tmp_buf, "%d-%d-", r.offset, r.size);
		memcpy(tmp_buf + strlen(tmp_buf), seg->data + r.offset, r.size);
		strcat(buf, tmp_buf);
	}
	
	int bytes = write(logfd, buf, strlen(buf));  // append?
	if(bytes == -1){
	    printf("error in writing segment to log\n");
		return -1;
	}
	
	fsync(logfd);
	
	if(flag){
	    printf("Commit transaction %d: finish writing segment %s to log\n", seg_node.tid, seg->segname);
	}
	
	return 0;
}


char* reconstruct_log_path(rvm_t rvm, segment_t* seg){
    char* log_path = (char*)malloc(strlen(rvm.directory) + 1 + strlen(seg->segname) + strlen(log_file_ext));
	strcpy(log_path, rvm.directory);
	strcat(log_path, "/");
	strcat(log_path, seg->segname);
	strcat(log_path, log_file_ext);
	
	printf("log path = %s\n", log_path);
	
    return log_path;
}


int restore_segment_from_log(segment_t* seg, char* log_path){
	FILE* logfile = fopen(log_path, "r");
	
	if(logfile == NULL){
	    printf("error in reading log file!\n");
		return -1;
	}
	
	char line[LINE_MAX];
	char *rest1, *token1;
	char *rest2, *token2;
	
	while(fgets(line, LINE_MAX, logfile) != NULL){
	    token1 = strtok_r(line, SEPERATOR, &rest1);
		
		if(strcmp(token1, COMMIT) != 0){
		    printf("this is not a committed log entry\n");
			return -1;
		}
		
		token1 = strtok_r(rest1, SEPERATOR, &rest1);
		while(token1 != NULL){
		    printf("token1 = %s\n", token1);
			token2 = strtok_r(token1, "-", &rest2);
            int offset = atoi(token2);
            printf("offset is %d\n", offset);
            token2 = strtok_r(rest2, "-", &rest2);  // skip 
            token2 = strtok_r(rest2, "-", &rest2);	// actual data
            printf("size of actual data is %d\n", strlen(token2));
			strcpy((char*)seg->data + offset, token2);  // copy data into segment
			
			token1 = strtok_r(rest1, SEPERATOR, &rest1);
		}
	}
	
	fclose(logfile);
	printf("finish restoring from log\n");
}


void reset_segment_tid(trans_t tid){
	for(std::list<segment_node_t>::iterator it = segment_list->begin(); it != segment_list->end(); ++it){
		if((*it).tid == tid){
			(*it).tid = -1;
		}
	}
}

void print_segment_list(){
    for(std::list<segment_node_t>::iterator it = segment_list->begin(); it != segment_list->end(); ++it){
	    printf("segment name = %s, tid = %d\n", (*it).segment->segname, (*it).tid);
	}
}