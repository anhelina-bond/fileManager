#include <stdio.h>
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

    if (argc == 1) {        //if no arguments provided
        printf("Available commands: \n createDir 'folderName' \n createFile 'fileName' \n listDir 'folderName' \n listFilesByExtension 'folderName' '.txt' \n readFile 'fileName'\n appendToFile 'fileName' 'new content' \n deleteFile 'fileName'\n deleteDir 'folderName'\n showLogs\n exit\n>> ");
        
        fgets(input, sizeof(input), stdin);  // Read full input
        input[strcspn(input, "\n")] = 0;     // Remove newline

        // Clear previous arguments
        command[0] = arg1[0] = arg2[0] = '\0';

        // Parse input safely
        sscanf(input, "%49s %99s %99[^\n]", command, arg1, arg2);
    } else {        // arguments provided
        strcpy(command, argv[1]);
        strcpy(arg1, argc > 2 ? argv[2] : "");
        strcpy(arg2, argc > 3 ? argv[3] : "");
    }

    // create log file
    int logFile = open(LOG_FILE, O_RDWR | O_APPEND | O_CREAT, 0777);

    // run command
    do {
        // Command execution
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
            printf("Invalid command!\n");
        }

        // Clear the arguments
        arg1[0] = '\0';
        arg2[0] = '\0';

        printf("\n>> ");
        fgets(input, sizeof(input), stdin);  // Read next command
        input[strcspn(input, "\n")] = 0;     // Remove newline

        // Clear previous command/arguments before parsing again
        command[0] = arg1[0] = arg2[0] = '\0';
        sscanf(input, "%49s %99s %99[^\n]", command, arg1, arg2);

    } while (strcmp(command, "exit") != 0);
    
    close(logFile);

    printf("Exiting the program...\n");
    return 0;
}
