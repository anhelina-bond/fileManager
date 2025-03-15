#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>

#include "fileManager.h"

void createDir(const char* folderName){
    if (mkdir(folderName, 0777) == -1) {  // 0777 = full permissions
        if (errno == EEXIST) {
            printf("Error: Directory '%s' already exists.\n", folderName);
        } else {
            perror("mkdir error");
        }
    } else {
        printf("directory created");
    }
}

void createFile(const char* fileName){
    int fd = open(fileName, O_CREAT | O_EXCL, 0777);
    if (fd == -1) {
        if (errno == EEXIST) {
            printf("Error: File '%s' already exists.\n", fileName);
        } else {
            perror("open error");
        }
    }
}

void listDir(const char* folderName){

}

void listFilesByExtension(const char* folderName, const char* extension){

}

void readFile(const char* fileName){

}

void appendToFile(const char* fileName, const char* content){

}

void deleteFile(const char* fileName){

}

void deleteDir(const char* folderName){

}

void showLogs(){

}
