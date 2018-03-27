//
// Created by chenzy on 3/20/18.
//

#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>
#include <dirent.h>
#include <assert.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <iostream>

#include "rvm.h"

#define COMMIT "Commit"
#define SEPERATOR "|"

int flag = 0;  // verbose output by default

int global_tid = 0;  // global counter for transaction ID
const char* log_file_ext = ".log";
std::list<segment_node_t>* segment_list = new std::list<segment_node_t>;  // global segment list

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
    check = read (fd, newdata, size_to_create);  // also need to restore from log? -Yaohong

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

	// restore from log -Yaohong
	char* log_path = reconstruct_log_path(rvm, seg);
	if(restore_segment_from_log(seg->data, log_path) == -1){
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
		printf("RVM: commit transaction %d\n", tid);
	
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
			// clear the region list
			(*it).regions.clear();
		}
	}
		
	// reset segment's tid to -1 if it is committed to disk
	reset_segment_tid(tid);
}


void rvm_abort_trans(trans_t tid){
    if(flag)
		printf("RVM: abort transaction %d\n", tid);
	
	// restore from in memory copy?
	// or from log segment in disk?
	
	for(std::list<segment_node_t>::iterator it = segment_list->begin(); it != segment_list->end(); ++it){
	    if((*it).tid == tid){
			
			//printf("region list size = %d\n", (*it).regions.size());
			
			// restore from in memory copy
			for(std::list<region_t>::iterator it_region = (*it).regions.begin(); it_region != (*it).regions.end(); ++it_region){
			    memcpy((*it).segment->data + (*it_region).offset, (*it_region).old_data, (*it_region).size);
			}
		}
	}
}



void rvm_truncate_log(rvm_t rvm){
    //check all logs under backing directory
    char* directory = rvm.directory;
    DIR* d;
    struct dirent *dir;    
    
	d = opendir(directory);
	if(d == NULL){
	    printf("error in opening directory!\n");
	}
	
	while((dir = readdir(d)) != NULL){
	    char* file_name = dir->d_name;
		
		if(is_log_file(file_name) == 0) continue;  // not a log file, skip
		
		char* seg_name = strtok(file_name, ".");
		printf("segment name is %s\n", seg_name);
		
		char* seg_path = concat_dir_file(directory, seg_name);
		
		int fd_seg = open(seg_path, O_RDWR);
		if(fd_seg == -1){
		    printf("error in opening segment file");
		}
		
		size_t seg_size = (size_t)get_segment_size(rvm, seg_name);
		
		void* data = mmap(NULL, seg_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_seg, 0);
        if(data == MAP_FAILED){
			printf("error in mmap with errno = %d!\n", errno);
		}
		
		char* file_path = concat_dir_file(directory, strcat(seg_name, log_file_ext));

		printf("path to log file is %s\n", file_path);
		
		restore_segment_from_log(data, file_path);
		
		if(munmap(data, seg_size) == -1){
		    printf("error in munmap!\n");
		}
		
		if(close(fd_seg) == -1){
			printf("error in closing segment file");
		}
		
		FILE* new_log_file = fopen(file_path, "w");  //erase all the contents of the log file by creating a new one
		fclose(new_log_file);
	}
	closedir(d);
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
	
	strcat(buf, "\n");
	
	int bytes = write(logfd, buf, strlen(buf));  // append to new line?
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
    char* log_path = (char*)malloc(strlen(rvm.directory) + 3 + strlen(seg->segname) + strlen(log_file_ext));
    log_path[0] = '.'; // Zeyu Chen
    log_path[1] = '/'; // Zeyu Chen
	strcpy(log_path, rvm.directory);
	strcat(log_path, "/");
	strcat(log_path, seg->segname);
	strcat(log_path, log_file_ext);
	
    return log_path;
}


int restore_segment_from_log(void* data, char* log_path){
	FILE* log_file = fopen(log_path, "r");
	
	if(log_file == NULL){
	    printf("error in reading log file!\n");
		printf("log path is %s\n", log_path);
		return -1;
	}
	
	char line[LINE_MAX];
	char *rest1, *token1;
	char *rest2, *token2;
	
	while(fgets(line, LINE_MAX, log_file) != NULL){
	    token1 = strtok_r(line, SEPERATOR, &rest1);
		
		if(strcmp(token1, COMMIT) != 0){
		    printf("this is not a committed log entry\n");
			return -1;
		}
		
		token1 = strtok_r(rest1, SEPERATOR, &rest1);
		while(token1 != NULL){
		    //printf("token1 = %s\n", token1);
			token2 = strtok_r(token1, "-", &rest2);  // offset
            int offset = atoi(token2);  
            //printf("offset is %d\n", offset);
            token2 = strtok_r(rest2, "-", &rest2);  // skip size 
            token2 = strtok_r(rest2, "-", &rest2);	// actual data
			
			//printf("bbb size of actual data is %d\n", strlen(token2));
			if(token2[strlen(token2)-1] == '\n') token2[strlen(token2)-1] = '\0';  // remove the '\n'
			//printf("aaa size of actual data is %d\n", strlen(token2));
			
			strcpy((char*)data + offset, token2);  // copy data into segment
			
			token1 = strtok_r(rest1, SEPERATOR, &rest1);
		}
	}
	
	fclose(log_file);
	//printf("finish restoring from log\n");
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

int is_log_file(char* file_path){
    int size_ext = strlen(log_file_ext);
	if(strlen(file_path) < size_ext) return 0;
	return strcmp(file_path + strlen(file_path) - size_ext, log_file_ext) == 0;	
}

char* concat_dir_file(char* dir, char* file){
    char* path = (char*)malloc(sizeof(char) * (strlen(dir) + strlen(file) + 1));
	strcat(path, dir);
	strcat(path, "/");
	strcat(path, file);
	return path;
}

int get_segment_size(rvm_t rvm, char* seg_name){
    for(std::list<segment_node_t>::iterator it = rvm.segment_list->begin(); it != rvm.segment_list->end(); ++it){
	    if(strcmp((*it).segment->segname, seg_name) == 0){
		    return (*it).segment->size;  
		}
	}
	
	return 0;
}