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

void writeMessage(const char *msg) {
    write(STDOUT_FILENO, msg, strlen(msg));
}

void createDir(const char* folderName, const int logFile) {
    if (mkdir(folderName, 0777) == -1) {  
        if (errno == EEXIST) {
            dprintf(STDOUT_FILENO, "Error: Directory '%s' already exists.\n", folderName);
            saveLogs("Error: Directory '%s' already exists.", logFile, folderName);
        } else {
            dprintf(STDOUT_FILENO, "Error: Creating directory '%s' failed.\n", folderName);
            saveLogs("Error: Creating directory '%s' failed.", logFile, folderName);
        }
    } else {
        dprintf(STDOUT_FILENO, "Directory '%s' created successfully\n", folderName);
        saveLogs("Directory '%s' created successfully", logFile, folderName);
    }
}

void createFile(const char* fileName, const int logFile) {
    int fd = open(fileName, O_CREAT | O_EXCL | O_WRONLY, 0777);
    if (fd == -1) {
        if (errno == EEXIST) {
            dprintf(STDOUT_FILENO, "Error: File '%s' already exists.\n", fileName);
            saveLogs("Error: File '%s' already exists.", logFile, fileName);
        } else {
            dprintf(STDOUT_FILENO, "Error: Creating File '%s' failed.\n", fileName);
            saveLogs("Error: Creating File '%s' failed.", logFile, fileName);
        }
    } else {
        dprintf(STDOUT_FILENO, "File '%s' created successfully\n", fileName);
        saveLogs("File '%s' created successfully", logFile, fileName);
    }
    close(fd);
}

void listDir(const char *folderName ,const int logFile) {
    pid_t pid = fork();

    if (pid < 0) {
        writeMessage("Fork failed\n");
        saveLogs("Fork failed", logFile);
        exit(1);
    }
    else if (pid == 0) { 
        DIR *dir = opendir(folderName);
        struct dirent *entry;

        if (dir == NULL) {
            dprintf(STDOUT_FILENO, "Error: Directory '%s' not found.\n", folderName);
            saveLogs("Error: Directory '%s' not found.", logFile, folderName);
            exit(1);
        }

        dprintf(STDOUT_FILENO, "Listing directory: %s\n", folderName);
        while ((entry = readdir(dir)) != NULL) {
            dprintf(STDOUT_FILENO, "%s\n", entry->d_name);
        }

        closedir(dir);
        exit(0);
    }
    else {
        wait(NULL);
    }
}

void readFile(const char* fileName, const int logFile) {
    int fd = open(fileName, O_RDONLY);

    if (fd == -1) {
        dprintf(STDOUT_FILENO, "Error: File '%s' not found.\n", fileName);
        saveLogs("Error: File '%s' not found.", logFile, fileName);
        return;
    }

    char buff[256];
    ssize_t bytes;

    while((bytes = read(fd, buff, sizeof(buff) - 1)) > 0) {
        buff[bytes] = '\0';
        write(STDOUT_FILENO, buff, bytes);
    }

    if (bytes == -1) {
        dprintf(STDOUT_FILENO, "Error reading file '%s'.\n", fileName);
        saveLogs("Error reading file '%s'.", logFile, fileName);
    } else {
        saveLogs("File '%s' read successfully.", logFile, fileName);
    }

    close(fd);
}

void appendToFile(const char* fileName, const char* content, const int logFile) {
    int fd = open(fileName, O_WRONLY | O_APPEND);

    if (fd == -1) { 
        dprintf(STDOUT_FILENO, "Error: File '%s' not found.\n", fileName);
        saveLogs("Error: File '%s' not found.", logFile, fileName);
        return;
    }

    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0; 

    if (fcntl(fd, F_SETLK, &lock) == -1) { 
        dprintf(STDOUT_FILENO, "Error: Cannot write to '%s'. File is locked or read-only.\n", fileName);
        saveLogs("Error: Cannot write to '%s'. File is locked or read-only.", logFile, fileName);
        close(fd);
        return;
    }

    if (write(fd, content, strlen(content)) == -1) {
        dprintf(STDOUT_FILENO, "Error writing to file '%s'.\n", fileName);
        saveLogs("Error writing to file '%s'.", logFile, fileName);
    } else {
        dprintf(STDOUT_FILENO, "Content appended successfully to '%s'.\n", fileName);
        saveLogs("Content appended successfully to '%s'.", logFile, fileName);
    }

    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);
    close(fd);
}

void listFilesByExtension(const char* folderName, const char* extension, const int logFile) {
    pid_t pid = fork();
    int found = 0;

    if (pid < 0) {
        writeMessage("Fork failed\n");
        saveLogs("Fork failed", logFile);
        exit(1);
    }
    else if (pid == 0) {
        struct dirent *entry;
        DIR *dir = opendir(folderName);
        if (dir == NULL) {
            dprintf(STDOUT_FILENO, "Error: Directory '%s' not found.\n", folderName);
            saveLogs("Error: Directory '%s' not found.", logFile, folderName);
            exit(1);
        }

        while ((entry = readdir(dir)) != NULL) {
            if (strstr(entry->d_name, extension) != NULL) {
                dprintf(STDOUT_FILENO, "%s\n", entry->d_name);
                found++;
            }
        }

        if (found == 0) {
            dprintf(STDOUT_FILENO, "No files with extension '%s' found in '%s'\n", extension, folderName);
            saveLogs("No files with extension '%s' found in '%s'", logFile, extension, folderName);
        }

        closedir(dir);
        exit(0);
    }
    else {
        wait(NULL);
    }
}

void deleteDir(const char* folderName, const int logFile) {
    pid_t pid = fork();

    if (pid < 0) {
        writeMessage("Fork failed\n");
        saveLogs("Fork failed", logFile);
        return;
    }

    if (pid == 0) { 
        DIR* dir = opendir(folderName);
        if (dir == NULL) {
            dprintf(STDOUT_FILENO, "Error: Directory '%s' not found.\n", folderName);
            saveLogs("Error: Directory '%s' not found.", logFile, folderName);
            exit(1);
        }

        struct dirent* entry;
        int isEmpty = 1;
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_name[0] != '.') {
                isEmpty = 0;
                break;
            }
        }
        closedir(dir);

        if (!isEmpty) {
            dprintf(STDOUT_FILENO, "Error: Directory '%s' is not empty.\n", folderName);
            saveLogs("Error: Directory '%s' is not empty.", logFile, folderName);
            exit(1);
        }

        if (rmdir(folderName) == 0) {
            dprintf(STDOUT_FILENO, "Directory '%s' deleted successfully.\n", folderName);
            saveLogs("Directory '%s' deleted successfully.", logFile, folderName);
        } else {
            dprintf(STDOUT_FILENO, "Error deleting directory '%s'.\n", folderName);
            saveLogs("Error deleting directory '%s'.", logFile, folderName);
        }
        exit(0);
    } else {
        wait(NULL);
    }
}

void deleteFile(const char* fileName, const int logFile) {
    pid_t pid = fork();

    if(pid < 0) {
        writeMessage("Fork failed\n");
        saveLogs("Fork failed", logFile);
        return;
    }

    if (pid == 0) {
        if (access(fileName, F_OK) == -1) {
            dprintf(STDOUT_FILENO, "Error: File '%s' not found.\n", fileName);
            saveLogs("Error: File '%s' not found.", logFile, fileName);
            exit(1);
        }

        if (remove(fileName) == 0) {
            dprintf(STDOUT_FILENO, "File '%s' deleted successfully.\n", fileName);
            saveLogs("File '%s' deleted successfully.", logFile, fileName);
        } else {
            dprintf(STDOUT_FILENO, "Error deleting file '%s'.\n", fileName);
            saveLogs("Error deleting file '%s'.", logFile, fileName);
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

    while((bytes = read(logFile, buff, sizeof(buff) - 1)) > 0) {
        buff[bytes] = '\0';
        write(STDOUT_FILENO, buff, bytes);
    }
}

/* Helper functions */
void saveLogs(const char* format, const int logFile, ...) {
    char timestamp[25];      
    getCurrentTimestamp(timestamp, sizeof(timestamp));

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
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    strftime(buffer, size, "[%Y-%m-%d %H:%M:%S]", tm_info);
}
