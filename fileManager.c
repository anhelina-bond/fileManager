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
            write_message(STDOUT, "Error: Directory already exists.\n");
            saveLogs("Error: Directory '%s' already exists.", logFile, folderName);
        } else {
            write_message(STDOUT, "Error: Creating directory failed.\n");
            saveLogs("Error: Creating directory '%s' failed.", logFile, folderName);
        }
    } else {
        write_message(STDOUT, "Directory created successfully.\n");
        saveLogs("Directory '%s' created successfully.", logFile, folderName);
    }
}

void createFile(const char* fileName, const int logFile) {
    int fd = open(fileName, O_CREAT | O_EXCL | O_WRONLY, 0777);
    if (fd == -1) {
        if (errno == EEXIST) {
            write_message(STDOUT, "Error: File already exists.\n");
            saveLogs("Error: File '%s' already exists.", logFile, fileName);
        } else {
            write_message(STDOUT, "Error: Creating file failed.\n");
            saveLogs("Error: Creating file '%s' failed.", logFile, fileName);
        }
    } else {
        write_message(STDOUT, "File created successfully.\n");
        saveLogs("File '%s' created successfully.", logFile, fileName);
    }
    close(fd);
}

void listDir(const char *folderName, const int logFile) {
    pid_t pid = fork();
    if (pid < 0) {
        write_message(STDOUT, "Fork failed.\n");
        saveLogs("Fork failed.", logFile);
        exit(1);
    } else if (pid == 0) { 
        DIR *dir = opendir(folderName);
        struct dirent *entry;
        if (dir == NULL) {
            write_message(STDOUT, "Error: Directory not found.\n");
            saveLogs("Error: Directory '%s' not found.", logFile, folderName);
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
        write_message(STDOUT, "Fork failed.\n");
        saveLogs("Fork failed.", logFile);
        exit(1);
    } else if (pid == 0) { 
        struct dirent *entry;
        DIR *dir = opendir(folderName);
        if (dir == NULL) {
            write_message(STDOUT, "Error: Directory not found.\n");
            saveLogs("Error: Directory '%s' not found.", logFile, folderName);
            exit(1);
        }
        while ((entry = readdir(dir)) != NULL) {
            if (strstr(entry->d_name, extension) != NULL) {
                write(STDOUT, entry->d_name, strlen(entry->d_name));
                write(STDOUT, "\n", 1);
                ++found;
            }
        }
        if (found == 0) {
            write_message(STDOUT, "No matching files found.\n");
            saveLogs("No files with extension '%s' found in '%s'", logFile, extension, folderName);
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
        write_message(STDOUT, "Error: File not found.\n");
        saveLogs("Error: File '%s' not found.", logFile, fileName);
        return;
    }

    char buff[256];
    ssize_t bytes;
    while ((bytes = read(fd, buff, sizeof(buff) - 1)) > 0) {
        write(STDOUT, buff, bytes);
    }

    if (bytes == -1) {
        write_message(STDOUT, "Error reading file.\n");
        saveLogs("Error reading file '%s'.", logFile, fileName);
    } else {
        saveLogs("File '%s' read successfully.", logFile, fileName);
    }

    close(fd);
}

void deleteFile(const char* fileName, const int logFile) {
    pid_t pid = fork();
    if (pid < 0) {
        write_message(STDOUT, "Fork failed.\n");
        saveLogs("Fork failed.", logFile);
        return;
    }
    if (pid == 0) {
        if (unlink(fileName) == 0) {
            write_message(STDOUT, "File deleted successfully.\n");
            saveLogs("File '%s' deleted successfully.", logFile, fileName);
        } else {
            write_message(STDOUT, "Error deleting file.\n");
            saveLogs("Error deleting file '%s'.", logFile, fileName);
        }
        exit(0);
    } else {
        wait(NULL);
    }
}

void deleteDir(const char* folderName, const int logFile) {
    pid_t pid = fork();
    if (pid < 0) {
        write_message(STDOUT, "Fork failed.\n");
        saveLogs("Fork failed.", logFile);
        return;
    }
    if (pid == 0) {
        if (rmdir(folderName) == 0) {
            write_message(STDOUT, "Directory deleted successfully.\n");
            saveLogs("Directory '%s' deleted successfully.", logFile, folderName);
        } else {
            write_message(STDOUT, "Error deleting directory.\n");
            saveLogs("Error deleting directory '%s'.", logFile, folderName);
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
void saveLogs(const char* format, const int logFile, const char* arg1 = NULL, const char* arg2 = NULL) {
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
