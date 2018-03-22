//
// Created by chenzy on 3/20/18.
//

#include <sys/stat.h>
#include <string.h>
#include <malloc.h>
#include <iostream>
#include <fcntl.h>
#include <zconf.h>
#include <errno.h>

#include "rvm.h"
extern int flag = 0;  // verbose output by default

rvm_t rvm_init (const char* directory) {
    rvm_t newrvm;
    int dirlen;

    mkdir(directory, 0755);
    dirlen = strlen(directory);

    if(flag)
        std::cout << "RVM: " << directory << " starts creating." << std::endl;

    newrvm.directory = (char*) malloc (dirlen* sizeof(char));
    strcpy(newrvm.directory, directory);

    newrvm.rvm_seg = new std::list<segment*>;

    if(flag)
        std::cout << "RVM: " << directory << " created." << std::endl;

    return newrvm;
}

// Map the file into the rvm
void* rvm_map (rvm_t rvm, const char *segname, int size_to_create) {
    int fd, logfd, size, check, segnamelen;
    char log[512];
    void* newdata;
    segment* seg;

    // checking if the segname is in the list
    if (flag)
        std::cout << "RVM: " << rvm.directory << " " << segname << " checking if multi-mapping." << std::endl;
    for (std::list<segment*>::iterator iterator = rvm.rvm_seg->begin(); iterator != rvm.rvm_seg->end(); ++iterator) {
        if ((*iterator)->segname == segname){
            std::cout << "RVM: " << rvm.directory << " other segment is mapping. Abort." << std::endl;
            return NULL;
        }
    }

    if (flag)
        std::cout << "RVM: " << rvm.directory << " starts mapping." << std::endl;
    // log: dir info
    sprintf (log, "%s/%s", rvm.directory, segname);


    if (flag)
        std::cout << "RVM: " << rvm.directory << " starts creating" << segname << " on the disk." << std::endl;

    // Open or create the back files
    fd = open(log, O_RDWR | O_CREAT, 0755);
    if (fd == -1) {
        std::cout << "Error: Fail to open/create back files." << std::endl;
        return NULL;
    }

    if (flag)
        std::cout << "RVM: " << rvm.directory << " starts creating" << segname << " log on the disk." << std::endl;
    strcat (log, ".log");
    // Open or create the log for back files
    logfd = open(log, O_RDWR | O_CREAT, 0755);
    if (logfd == -1) {
        std::cout << "Error: Fail to open/create log files." << std::endl;
        return NULL;
    }

    // Get size of the current file
    if (flag)
        std::cout << "RVM: " << rvm.directory << " getting size of" << segname << "." << std::endl;

    size = lseek (fd, 0, SEEK_END);

    if (size == -1) {
        close(fd);
        std::cout << "Error: Fail to open file, " << strerror(errno) << std::endl;
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
        std::cout << "RVM: " << rvm.directory << "Malloc " << segname << "." << std::endl;
    newdata = (void*) malloc (size_to_create* sizeof(char));

    // Copy data from disk to memory
    if (flag)
        std::cout << "RVM: " << rvm.directory << " syncing data to 0x" << std::hex << newdata << "." << std::endl;

    lseek (fd, 0, SEEK_SET);
    check = read (fd, newdata, size_to_create);

    if (check == -1) {
        close(fd);
        std::cout << " Error: Fail to sync, " << strerror(errno) << std::endl;
        return NULL;
    }

    // create segment info
    if (flag)
        std::cout << "RVM: " << rvm.directory << " " << segname << " creating info." << std::endl;

    seg = (segment*) malloc(sizeof(segment));
    segnamelen = strlen(segname);

    seg->segname = NULL;
    seg->segname = (char*) malloc (segnamelen*sizeof(char));
    strcpy(seg->segname, segname);

    seg->data = newdata;
    seg->size = size_to_create;
    seg->fd = fd;
    seg->logfd = logfd;
    seg->isMap = 1; // 1 for map state

    seg->locked = 0;

    // Add new segment to rvm
    if (flag)
        std::cout << "RVM: " << rvm.directory  << segname << " saving info." << std::endl;
    rvm.rvm_seg->push_back(seg);

    sprintf(log, "RVM map %s to memory 0x%x, size: %d. \n", segname, newdata, size_to_create);

    if (flag)
        std::cout << "RVM: " << rvm.directory  << segname << " logging." << std::endl;

    rvm_log(seg->logfd, log);
    if (flag)
        std::cout << "RVM: " << rvm.directory  << segname << " map success, name: " << segname << " start address: 0x"
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
    for (std::list<segment*>::iterator iterator = rvm.rvm_seg->begin(); iterator != rvm.rvm_seg->end(); ++iterator) {
        if ((*iterator)->data == segbase){

            std::cout << rvm.rvm_seg->size() << std::endl;
            if (flag)
                std::cout << "RVM: " << rvm.directory << " " << (*iterator)->segname << " " << "0x" << std::hex
                          << segbase <<" starts unmapping." << std::endl;
            char log[1024];
            sprintf(log, "RVM: %s unmap %s from memory 0x%x", rvm.directory, (*iterator)->segname, segbase);
            rvm_log((*iterator)->logfd, log);

            if (flag)
                std::cout << "RVM: " << rvm.directory << " closing" << (*iterator)->segname << "." << std::endl;
            close((*iterator)->fd);
            close((*iterator)->logfd);

            if (flag)
                std::cout << "RVM: " << rvm.directory << " free memory." << std::endl;
            free((*iterator)->segname);
            free(segbase);
            free(*iterator);

            rvm.rvm_seg->erase(iterator);
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
    for (std::list<segment*>::iterator iterator = rvm.rvm_seg->begin(); iterator != rvm.rvm_seg->end(); ++iterator) {
        if ((*iterator)->segname == segname){
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