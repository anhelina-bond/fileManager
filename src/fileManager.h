#ifndef FILE_MANAGER
#define FILE_MANAGER

void createDir(const char* folderName);
void createFile(const char* fileName);
void listDir(const char* folderName);
void listFilesByExtension(const char* folderName, const char* extension);
void readFile(const char* fileName);
void appendToFile(const char* fileName, const char* content);
void deleteFile(const char* fileName);
void deleteDir(const char* folderName);
void showLogs();

#endif
