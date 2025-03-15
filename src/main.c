#include <stdio.h>
#include <sys/stat.h>
#include <string.h>

#include "fileManager.h"

int main(int argc, char *argv[] ) {
    char command[50];  // Allocate memory for command
    char arg1[100];    // Allocate memory for arguments
    char arg2[100];
    if (argc == 1) {
        printf("Available commands: \n createDir 'folderName' \n createFile 'fileName' \n listDir 'folderName' \n listFilesByExtension 'folderName' '.txt' \n readFile 'fileName'\n appendToFile 'fileName' 'new content' \n deleteFile 'fileName'\n deleteDir 'folderName'\n showLogs\n exit\n>> ");
        scanf("%49s %99s %99[^\n]", command, arg1, arg2);

    } else{
        strcpy(command, argv[1]);
        strcpy(arg1, argc > 2 ? argv[2] : "");
        strcpy(arg2, argc > 3 ? argv[3] : "");
    }

    do
    {
        // Command execution
        if (strcmp(command, "createDir") == 0) {
            createDir(arg1);
        } else if (strcmp(command, "createFile") == 0) {
            createFile(arg1);
        } else if (strcmp(command, "listDir") == 0) {
            listDir(arg1);
        } else if (strcmp(command, "listFilesByExtension") == 0) {
            listFilesByExtension(arg1, arg2);
        } else if (strcmp(command, "readFile") == 0) {
            readFile(arg1);
        } else if (strcmp(command, "appendToFile") == 0) {
            appendToFile(arg1, arg2);
        } else if (strcmp(command, "deleteFile") == 0) {
            deleteFile(arg1);
        } else if (strcmp(command, "deleteDir") == 0) {
            deleteDir(arg1);
        } else if (strcmp(command, "showLogs") == 0) {
            showLogs();
        } else {
            printf("Invalid command!\n");
        }

        //clear the arguments
        arg1[0] = '\0';
        arg2[0] = '\0';

        printf(">>");
        scanf("%49s %99s %99[^\n]", command, arg1, arg2);
    } while (strcmp(command, "exit") != 0);

    printf("Exiting the program...");
    

    


    return 0;
}