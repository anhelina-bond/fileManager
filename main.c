#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include "fileManager.h"

#define LOG_FILE "log.txt"

int main(int argc, char *argv[]) {
    char input[256];
    char command[50];
    char arg1[100];
    char arg2[100];

    if (argc == 1) { 
        // Display available commands
        const char *help_msg = "Available commands:\n"
                               " createDir 'folderName'\n"
                               " createFile 'fileName'\n"
                               " listDir 'folderName'\n"
                               " listFilesByExtension 'folderName' '.txt'\n"
                               " readFile 'fileName'\n"
                               " appendToFile 'fileName' 'new content'\n"
                               " deleteFile 'fileName'\n"
                               " deleteDir 'folderName'\n"
                               " showLogs\n"
                               " exit\n>> ";
        write(STDOUT_FILENO, help_msg, strlen(help_msg));

        // Read input
        ssize_t bytes_read = read(STDIN_FILENO, input, sizeof(input) - 1);
        if (bytes_read <= 0) return 1; // Handle read error
        input[bytes_read - 1] = '\0'; // Remove newline

        // Parse command using strtok
        char *token = strtok(input, " ");
        if (token) strcpy(command, token);
        token = strtok(NULL, " ");
        if (token) strcpy(arg1, token);
        token = strtok(NULL, "\n");
        if (token) strcpy(arg2, token);
    } else { 
        strcpy(command, argv[1]);
        strcpy(arg1, argc > 2 ? argv[2] : "");
        strcpy(arg2, argc > 3 ? argv[3] : "");
    }

    // Open log file
    int logFile = open(LOG_FILE, O_RDWR | O_APPEND | O_CREAT, 0777);

    do {
        if (strcmp(command, "createDir") == 0) {
            createDir(arg1, logFile);
        } else if (strcmp(command, "createFile") == 0) {
            createFile(arg1, logFile);
        } else if (strcmp(command, "listDir") == 0) {
            listDir(arg1, logFile);
        } else if (strcmp(command, "listFilesByExtension") == 0) {
            listFilesByExtension(arg1, arg2, logFile);
        } else if (strcmp(command, "readFile") == 0) {
            readFile(arg1, logFile);
        } else if (strcmp(command, "appendToFile") == 0) {
            appendToFile(arg1, arg2, logFile);
        } else if (strcmp(command, "deleteFile") == 0) {
            deleteFile(arg1, logFile);
        } else if (strcmp(command, "deleteDir") == 0) {
            deleteDir(arg1, logFile);
        } else if (strcmp(command, "showLogs") == 0) {
            showLogs(logFile);
        } else {
            const char *error_msg = "Invalid command!\n";
            write(STDOUT_FILENO, error_msg, strlen(error_msg));
        }

        // Clear input buffers
        memset(input, 0, sizeof(input));
        memset(command, 0, sizeof(command));
        memset(arg1, 0, sizeof(arg1));
        memset(arg2, 0, sizeof(arg2));

        // Read next command
        write(STDOUT_FILENO, "\n>> ", 4);
        ssize_t bytes_read = read(STDIN_FILENO, input, sizeof(input) - 1);
        if (bytes_read <= 0) break;
        input[bytes_read - 1] = '\0';

        // Parse input
        char *token = strtok(input, " ");
        if (token) strcpy(command, token);
        token = strtok(NULL, " ");
        if (token) strcpy(arg1, token);
        token = strtok(NULL, "\n");
        if (token) strcpy(arg2, token);

    } while (strcmp(command, "exit") != 0);

    close(logFile);
    write(STDOUT_FILENO, "Exiting the program...\n", 23);
    return 0;
}
