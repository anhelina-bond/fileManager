#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <dirent.h>
#include <time.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <stdarg.h>
#include "fileManager.h"

#define STDOUT 1
#define STDERR 2

void write_message(int fd, const char *message) {
    write(fd, message, strlen(message));
}

void createDir(const char* folderName, const int logFile) {
    if (mkdir(folderName, 0777) == -1) {  
        if (errno == EEXIST) {
            write_message(STDOUT, "Error: Directory already exists \n");
            saveLogs("Error: Directory already exists: ", logFile, folderName, NULL);
        } else {
            write_message(STDOUT, "Error: Creating directory failed \n");
            saveLogs("Error: Creating directory failed: ", logFile, folderName, NULL);
        }
    } else {
        write_message(STDOUT, "Directory created successfully \n");
        saveLogs("Directory created successfully: ", logFile, folderName, NULL);
    }
}

void createFile(const char* fileName, const int logFile) {
    int fd = open(fileName, O_CREAT | O_EXCL | O_WRONLY, 0777);
    if (fd == -1) {
        if (errno == EEXIST) {
            write_message(STDOUT, "Error: File already exists \n");
            saveLogs("Error: File already exists: ", logFile, fileName, NULL);
        } else {
            write_message(STDOUT, "Error: Creating file failed \n");
            saveLogs("Error: Creating file failed: ", logFile, fileName, NULL);
        }
    } else {
        write_message(STDOUT, "File created successfully \n");
        saveLogs("File created successfully: ", logFile, fileName, NULL);
    }
    close(fd);
}

void listDir(const char *folderName, const int logFile) {
    pid_t pid = fork();
    if (pid < 0) {
        write_message(STDOUT, "Fork failed \n");
        saveLogs("Fork failed ", logFile, NULL, NULL);
        exit(1);
    } else if (pid == 0) { 
        DIR *dir = opendir(folderName);
        struct dirent *entry;
        if (dir == NULL) {
            write_message(STDOUT, "Error: Directory not found \n");
            saveLogs("Error: Directory not found: ", logFile, folderName, NULL);
            exit(1);
        }
        while ((entry = readdir(dir)) != NULL) {
            write(STDOUT, entry->d_name, strlen(entry->d_name));
            write(STDOUT, "\n", 1);
        }
        closedir(dir);
        exit(0);
    } else {
        wait(NULL);
    }
}

void listFilesByExtension(const char* folderName, const char* extension, const int logFile) {
    pid_t pid = fork();
    int found = 0;
    if (pid < 0) {
        write_message(STDOUT, "Fork failed \n");
        saveLogs("Fork failed", logFile, NULL, NULL);
        exit(1);
    } else if (pid == 0) { 
        struct dirent *entry;
        DIR *dir = opendir(folderName);
        if (dir == NULL) {
            write_message(STDOUT, "Error: Directory not found \n");
            saveLogs("Error: Directory  not found: ", logFile, folderName, NULL);
            exit(1);
        }
        
        size_t ext_len = strlen(extension);

        while ((entry = readdir(dir)) != NULL) {
            char *filename = entry->d_name;
            size_t len = strlen(filename);
            if (len > ext_len && strcmp(filename + (len - ext_len), extension) == 0) {
                write(STDOUT, filename, len);
                write(STDOUT, "\n", 1);
                ++found;
            }
        }
        if (found == 0) {
            write_message(STDOUT, "No matching files found \n");
            saveLogs("No files with extension found in  directory", logFile, extension, folderName);
        }
        closedir(dir);
        exit(0);
    } else {
        wait(NULL);
    }
}

void readFile(const char* fileName, const int logFile) {
    int fd = open(fileName, O_RDONLY);
    if (fd == -1) {
        write_message(STDOUT, "Error: File not found \n");
        saveLogs("Error: File not found: ", logFile, fileName, NULL);
        return;
    }

    char buff[256];
    ssize_t bytes;
    while ((bytes = read(fd, buff, sizeof(buff) - 1)) > 0) {
        write(STDOUT, buff, bytes);
    }

    if (bytes == -1) {
        write_message(STDOUT, "Error reading file \n");
        saveLogs("Error reading file: ", logFile, fileName, NULL);
    } else {
        saveLogs("File read successfully: ", logFile, fileName, NULL);
    }

    close(fd);
}

void appendToFile(const char* fileName, const char* content, const int logFile) {
    int fd = open(fileName, O_WRONLY | O_APPEND);
    if (fd == -1) { 
        write_message(STDOUT, "Error: File not found\n");
        saveLogs("Error: File not found: ", logFile, fileName, NULL);
        return;
    }

    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;

    if (fcntl(fd, F_SETLK, &lock) == -1) {
        write_message(STDOUT, "Error: Cannot write. File is locked\n");
        saveLogs("Error: Cannot write to file. File is locked: ", logFile, fileName, NULL);
        close(fd);
        return;
    }

    if (write(fd, content, strlen(content)) == -1) {
        write_message(STDOUT, "Error writing to file.\n");
        saveLogs("Error writing to file: ", logFile, fileName, NULL);
    } else {
        write_message(STDOUT, "Content appended successfully.\n");
        saveLogs("Content appended successfully to file: ", logFile, fileName, NULL);
    }

    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);
    close(fd);
}

void deleteFile(const char* fileName, const int logFile) {
    pid_t pid = fork();
    if (pid < 0) {
        write_message(STDOUT, "Fork failed \n");
        saveLogs("Fork failed ", logFile, NULL, NULL);
        return;
    }
    if (pid == 0) {
        if (unlink(fileName) == 0) {
            write_message(STDOUT, "File deleted successfully \n");
            saveLogs("File deleted successfully: ", logFile, fileName, NULL);
        } else {
            write_message(STDOUT, "Error deleting file \n");
            saveLogs("Error deleting file: ", logFile, fileName, NULL);
        }
        exit(0);
    } else {
        wait(NULL);
    }
}

void deleteDir(const char* folderName, const int logFile) {
    int dirFD = open(folderName, O_DIRECTORY);
    if (dirFD < 0) {
        write_message(STDOUT, "Error opening directory.\n");
        saveLogs("Error opening directory: ", logFile, folderName, NULL);
        return;
    }

    DIR* dir = fdopendir(dirFD);
    if (dir == NULL) {
        write_message(STDOUT, "Error accessing directory.\n");
        saveLogs("Error accessing directory:", logFile, folderName, NULL);
        close(dirFD);
        return;
    }

    struct dirent* entry;
    int isEmpty = 1; // Assume empty

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] != '.') { // Ignore `.` and `..`
            isEmpty = 0; // Directory is not empty
            break;
        }
    }
    closedir(dir);

    if (!isEmpty) {
        write_message(STDOUT, "Directory is not empty.\n");
        saveLogs("Directory '%s' is not empty.", logFile, folderName, NULL);
        return;
    }
    pid_t pid = fork();
    if (pid < 0) {
        write_message(STDOUT, "Fork failed \n");
        saveLogs("Fork failed ", logFile, NULL, NULL);
        return;
    }
    if (pid == 0) {
        if (rmdir(folderName) == 0) {
            write_message(STDOUT, "Directory deleted successfully \n");
            saveLogs("Directory deleted successfully: ", logFile, folderName, NULL);
        } else {
            write_message(STDOUT, "Error deleting directory \n");
            saveLogs("Error deleting directory: ", logFile, folderName, NULL);
        }
        exit(0);
    } else {
        wait(NULL);
    }
}

void showLogs(const int logFile) {
    char buff[1024];
    ssize_t bytes;
    lseek(logFile, 0, SEEK_SET);
    while ((bytes = read(logFile, buff, sizeof(buff) - 1)) > 0) {
        write(STDOUT, buff, bytes);
    }
}

/* Helper Functions */
void saveLogs(const char* format, const int logFile, const char* arg1, const char* arg2) {
    char timestamp[25];
    getCurrentTimestamp(timestamp, sizeof(timestamp));

    char message[256];
    
    // Manually create the log message
    strcpy(message, timestamp);
    strcat(message, " ");
    strcat(message, format);
    if (arg1 != NULL) {
        strcat(message, " ");
        strcat(message, arg1);
    }
    if (arg2 != NULL) {
        strcat(message, " ");
        strcat(message, arg2);
    }
    strcat(message, "\n");

    write(logFile, message, strlen(message));
}


void getCurrentTimestamp(char *buffer, size_t size) {
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    strftime(buffer, size, "[%Y-%m-%d %H:%M:%S]", tm_info);
}
