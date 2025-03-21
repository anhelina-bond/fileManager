#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <dirent.h>
#include <time.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <stdarg.h>
#include "fileManager.h"

void createDir(const char* folderName, const int logFile){
    if (mkdir(folderName, 0777) == -1) {  
        if (errno == EEXIST) {
            printf("Error: Directory '%s' already exists.", folderName);
            saveLogs("Error: Directory '%s' already exists.", logFile, folderName);
        } else {
            printf("Error: Creating directory '%s' failed.", folderName);
            saveLogs("Error: Creating directory '%s' failed.", logFile, folderName);
        }
    } else {
        printf("Directory '%s' created successfuly", folderName);
        saveLogs("Directory '%s' created successfuly", logFile, folderName);
    }
}

void createFile(const char* fileName, const int logFile){
    int fd = open(fileName, O_CREAT | O_EXCL, 0777);
    if (fd == -1) {
        if (errno == EEXIST) {
            printf("Error: File '%s' already exists.", fileName);
            saveLogs("Error: File '%s' already exists.", logFile, fileName);
        } else {
            printf("Error: Creating File '%s' failed.", fileName);
            saveLogs("Error: Creating File '%s' failed.", logFile, fileName);
        }
    } else {
        printf("File '%s' created successfuly", fileName);
        saveLogs("File '%s' created successfuly", logFile, fileName);
    }
    close(fd);
}

void listDir(const char *folderName ,const int logFile) {
    pid_t pid = fork(); // Create a new process

    if (pid < 0) {
        printf("Fork failed %s", "");
        saveLogs("Fork failed %s", logFile, "");
        exit(1);
    }
    else if (pid == 0) { // Child process
        DIR *dir = opendir(folderName);
        struct dirent *entry;

        if (dir == NULL) {
            printf("Error: Directory '%s' not found.", folderName);
            saveLogs("Error: Directory '%s' not found.", logFile, folderName);
            exit(1);
        }

        printf("Listing directory: %s\n", folderName);
        while ((entry = readdir(dir)) != NULL) {
            printf("%s\n", entry->d_name);
        }

        closedir(dir);
        exit(0); // Child process exits after listing
    }
    else { // Parent process
        wait(NULL); // Wait for the child process to complete
    }
}

void listFilesByExtension(const char* folderName, const char* extension, const int logFile){
    
    pid_t pid = fork(); // Create a new process
    int found = 0;
    if (pid < 0) {
        printf("Fork failed %s", "");
        saveLogs("Fork failed %s", logFile, "");
        exit(1);
    }
    else if (pid == 0) { // Child process
        struct dirent *entry;
        DIR *dir = opendir(folderName);
        if (dir == NULL) {
            printf("Error: Directory '%s' not found.", folderName);
            saveLogs("Error: Directory '%s' not found.", logFile, folderName);
            exit(1);
        }

        while ((entry = readdir(dir)) != NULL) {
            if(strstr(entry->d_name, extension) != NULL){
               printf("%s\n", entry->d_name);  // Print each file/folder name
                ++found;
            }
        }

        if (found == 0) {
            printf("No files with extention '%s' found in '%s'", extension, folderName);
            saveLogs("No files with extention '%s' found in '%s'", logFile, extension, folderName);
    }

        closedir(dir);
        exit(0); // Child process exits after listing
    }
    else { // Parent process
        wait(NULL); // Wait for the child process to complete
    }
}

void readFile(const char* fileName, const int logFile){
    int fd = open(fileName, O_RDONLY);

    if (fd == -1) {
        printf("Error: File '%s' not found.", fileName);
        saveLogs("Error: File '%s' not found.", logFile, fileName);
        return;
    }

    char buff[256];
    ssize_t bytes;

    while((bytes = read(fd, buff, sizeof(buff)-1)) > 0) {
        buff[bytes] = '\0';
        printf("%s", buff);
    }

    if (bytes == -1) {
        printf("Error reading file '%s'.", fileName);
        saveLogs("Error reading file '%s'.", logFile, fileName);
    } else {
        saveLogs("File '%s' read successfuly.", logFile, fileName);
    }

    close(fd); // Close file descriptor
}

void appendToFile(const char* fileName, const char* content, const int logFile){
    int fd = open(fileName, O_WRONLY | O_APPEND);

    if (fd == -1) { 
        printf("Error: File '%s' not found.", fileName);
        saveLogs("Error: File '%s' not found.", logFile, fileName);
        return;
    }

    //Setting up file lock
    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;  // Lock the whole file

    if (fcntl(fd, F_SETLK, &lock) == -1) { // Try locking
        printf("Error: Cannot write to '%s'. File is locked or read-only.", fileName);
        saveLogs("Error: Cannot write to '%s'. File is locked or read-only.", logFile, fileName);
        close(fd);
        return;
    }

    // Write new content to the file
    if (write(fd, content, strlen(content)) == -1) {
        printf("Error writing to a file '%s'.", fileName);
        saveLogs("Error writing to a file '%s'.", logFile, fileName);
    } else {
        printf("Content appended successfully to '%s'.", fileName);
        saveLogs("Content appended successfully to '%s'.", logFile, fileName);
    }

    // Unlock file
    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);

    close(fd); // Close file descriptor
}

void deleteFile(const char* fileName, const int logFile){
    pid_t pid = fork();

    if(pid < 0) {
        printf("Fork failed %s", "");
        saveLogs("Fork failed %s", logFile, "");
        return;
    }

    if (pid == 0 ) {
        if (access(fileName, F_OK)  == -1) {
            printf("Error: File '%s' not found.", fileName);
            saveLogs("Error: File '%s' not found.", logFile, fileName);
            exit(1);
        }

        // Delete the file
        if (remove(fileName) == 0) {
            printf("File '%s' deleted successfully.", fileName);
            saveLogs("File '%s' deleted successfully.", logFile, fileName);
        } else {
            printf("Error deleting a file '%s'.", fileName);
            saveLogs("Error deleting a file '%s'.", logFile, fileName);
        }
        exit(0);
    } else {
        wait (NULL);
    }
}

void deleteDir(const char* folderName, const int logFile) {
    pid_t pid = fork();

    if (pid < 0) {
        printf("Fork failed %s", "");
        saveLogs("Fork failed %s", logFile, "");
        return;
    }

    if (pid == 0) { // Child process
        // Check if the directory exists
        DIR* dir = opendir(folderName);
        if (dir == NULL) {
            printf("Error: Directory '%s' not found.", folderName);
            saveLogs("Error: Directory '%s' not found.", logFile, folderName);
            exit(1);
        }

        // Check if directory is empty
        struct dirent* entry;
        int isEmpty = 1;
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_name[0] != '.') { // Ignore "." and ".."
                isEmpty = 0;
                break;
            }
        }
        closedir(dir);

        if (!isEmpty) {
            printf("Error: Directory '%s' is not empty.", folderName);
            saveLogs("Error: Directory '%s' is not empty.", logFile, folderName);
            exit(1);
        }

        // Delete the directory
        if (rmdir(folderName) == 0) {
            printf("Directory '%s' deleted successfully.", folderName);
            saveLogs("Directory '%s' deleted successfully.", logFile, folderName);
        } else {
            printf("Error deleting a Directory '%s'.", folderName);
            saveLogs("Error deleting a Directory '%s'.", logFile, folderName);
        }
        exit(0);
    } else {
        wait(NULL); // Parent process waits for child
    }
}

void showLogs(const int logFile){
    char buff[1024];
    ssize_t bytes;

    // move to begining of the file
    lseek(logFile, 0, SEEK_SET);

    while((bytes = read(logFile, buff, sizeof(buff)-1)) > 0) {
        buff[bytes] = '\0';
        printf("%s", buff);
        if(bytes == -1) {
            perror("Error reading log file");
        }
    }
}


/* Helper functions */

void saveLogs(const char* format, const int logFile, ...) {

    // get current timestamp
    char timestamp[25];      
    getCurrentTimestamp(timestamp, sizeof(timestamp));

    // compose the result
    char message[256];
    va_list args;
    va_start(args, logFile);

    vsnprintf(message, sizeof(message), format, args);
    va_end(args);

    char result[300];
    snprintf(result, sizeof(result), "%s %s\n", timestamp, message);

    write(logFile, result, strlen(result));
}


void getCurrentTimestamp(char *buffer, size_t size) {
    time_t now = time(NULL);       // Get current time
    struct tm *tm_info = localtime(&now);  // Convert to local time

    strftime(buffer, size, "[%Y-%m-%d %H:%M:%S]", tm_info);  // Format timestamp
}

