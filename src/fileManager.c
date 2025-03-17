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

#include "fileManager.h"

void createDir(const char* folderName, const int logFile){
    if (mkdir(folderName) == -1) {  // 0777 = full permissions
        if (errno == EEXIST) {
            printf("Error: Directory '%s' already exists.\n", folderName);
        } else {
            perror("mkdir error");
        }
    } else {
        printf("directory created");
    }
}

void createFile(const char* fileName, const int logFile){
    int fd = open(fileName, O_CREAT | O_EXCL, 0777);
    if (fd == -1) {
        if (errno == EEXIST) {
            printf("Error: File '%s' already exists.\n", fileName);
        } else {
            perror("open error");
        }
    }
    close(fd);
}

void listDir(const char* folderName, const int logFile){
    struct dirent *entry;
    DIR *dir = opendir(folderName);
    if(dir==NULL) {
        printf("Error: Directory '%s' not found", folderName);
    }

    while ((entry = readdir(dir)) != NULL) {
        printf("%s\n", entry->d_name);  // Print each file/folder name
    }

    closedir(dir);
}

void listFilesByExtension(const char* folderName, const char* extension, const int logFile){
    struct dirent *entry;
    DIR *dir = opendir(folderName);
    char *p;
    if(dir==NULL) {
        printf("Error: Directory '%s' not found", folderName);
    }

    while ((entry = readdir(dir)) != NULL) {
        p = strstr(entry->d_name, extension);
        printf("%s\n", entry->d_name);  // Print each file/folder name
    }

    closedir(dir);
}

void readFile(const char* fileName, const int logFile){
    int fd = open(fileName, O_RDONLY);

    if (fd == -1) {
        printf("Error: File '%s' not found.\n", fileName);
        return;
    }

    char buff[256];
    ssize_t bytes;

    while((bytes = read(fd, buff, sizeof(buff)-1)) > 0) {
        buff[bytes] = '\0';
        printf("%s", buff);
    }

    if (bytes == -1) {
        perror("Error reading file");
    }

    close(fd); // Close file descriptor
}

void appendToFile(const char* fileName, const char* content, const int logFile){
    int fd = open(fileName, O_WRONLY | O_APPEND);

    if (fd == -1) { 
        printf("Error: Cannot write to '%s'. File does not exist or is read-only.\n", fileName);
        return;
    }

    //Setting up file lock
    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;  // Lock the whole file

    if (fcntl(fd, F_SETLK, &lock) == -1) { // Try locking
        printf("Error: Cannot write to \"%s\". File is locked or read-only.\n", fileName);
        close(fd);
        return;
    }

    // Write new content to the file
    if (write(fd, content, strlen(content)) == -1) {
        perror("Error writing to file");
    } else {
        printf("Content appended successfully to \"%s\".\n", fileName);
    }

    // Unlock file
    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);

    close(fd); // Close file descriptor
}

void deleteFile(const char* fileName, const int logFile){
    pid_t pid = fork();

    if(pid < 0) {
        perror("Fork failed");
        return;
    }

    if (pid == 0 ) {
        if (access(fileName, F_OK)  == -1) {
            printf("Error: File \"%s\" not found.\n", fileName);
            exit(EXIT_FAILURE);
        }

        // Delete the file
        if (remove(fileName) == 0) {
            printf("File \"%s\" deleted successfully.\n", fileName);
        } else {
            perror("Error deleting file");
        }
        exit(EXIT_SUCCESS);
    } else {
        wait (EXIT_SUCCESS);
    }
}

void deleteDir(const char* folderName, const int logFile) {
    pid_t pid = fork();

    if (pid < 0) {
        perror("Fork failed");
        return;
    }

    if (pid == 0) { // Child process
        // Check if the directory exists
        DIR* dir = opendir(folderName);
        if (dir == NULL) {
            perror("Error opening directory");
            exit(EXIT_FAILURE);
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
            printf("Error: Directory \"%s\" is not empty.\n", folderName);
            exit(EXIT_FAILURE);
        }

        // Delete the directory
        if (rmdir(folderName) == 0) {
            printf("Directory \"%s\" deleted successfully.\n", folderName);
        } else {
            perror("Error deleting directory");
        }
        exit(EXIT_SUCCESS);
    } else {
        wait(NULL); // Parent process waits for child
    }
}

void showLogs(const int logFile){
    char buff[1024];
    ssize_t bytes;

    while((bytes = read(logFile, buff, sizeof(buff)-1)) > 0) {
        buff[bytes] = '\0';
        printf("%s", buff);
    }
}

void saveLogs(const char* message, const int logFile) {

    // Get current timestamp
    char timestamp[25];      
    getCurrentTimestamp(timestamp, sizeof(timestamp));
    char result[100];
    strncat(result, timestamp, sizeof(timestamp));
    strncat(result, message, sizeof(message));
    write(logFile, result, strlen(result));
}


void getCurrentTimestamp(char *buffer, size_t size) {
    time_t now = time(NULL);       // Get current time
    struct tm *tm_info = localtime(&now);  // Convert to local time

    strftime(buffer, size, "[%Y-%m-%d %H:%M:%S]", tm_info);  // Format timestamp
}

// void listDir(const char *path) {
//     pid_t pid = fork(); // Create a new process

//     if (pid < 0) {
//         perror("Fork failed");
//         exit(1);
//     }
//     else if (pid == 0) { // Child process
//         DIR *dir = opendir(path);
//         struct dirent *entry;

//         if (dir == NULL) {
//             perror("opendir failed");
//             exit(1);
//         }

//         printf("Listing directory: %s\n", path);
//         while ((entry = readdir(dir)) != NULL) {
//             printf("%s\n", entry->d_name);
//         }

//         closedir(dir);
//         exit(0); // Child process exits after listing
//     }
//     else { // Parent process
//         wait(NULL); // Wait for the child process to complete
//     }
// }