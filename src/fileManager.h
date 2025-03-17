#ifndef FILE_MANAGER
#define FILE_MANAGER

void createDir(const char* folderName , const int logFile);
void createFile(const char* fileName, const int logFile);
void listDir(const char* folderName, const int logFile);
void listFilesByExtension(const char* folderName, const char* extension, const int logFile);
void readFile(const char* fileName, const int logFile);
void appendToFile(const char* fileName, const char* content, const int logFile);
void deleteFile(const char* fileName, const int logFile);
void deleteDir(const char* folderName, const int logFile);
void showLogs(const int logFile);
void saveLogs(const char* message, const int logFile);

#endif
