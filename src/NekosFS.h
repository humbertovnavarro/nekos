#include "FreeRTOS.h"
#include "Arduino.h"
#include <vector>
namespace nekos {
    namespace fs {
            // Basic File Operations
            bool writeFile(const char* path, const String& content);
            bool appendFile(const char* path, const String& content);
            String readFile(const char* path);
            bool deleteFile(const char* path);
            bool touchFile(const char* path);

            // Directory Operations
            void listDir(const char* dirname, uint8_t levels = 0);
            bool makeDir(const char* path);
            // File Management
            bool copyFile(const char* srcPath, const char* destPath);
            bool moveFile(const char* oldPath, const char* newPath);
            bool fileExists(const char* path);
            size_t getFileSize(const char* path);

            // File Search
            void findFiles(const char* dirname, const char* pattern, std::vector<String>& outFiles, uint8_t levels = 0);

            // Filesystem Info
            size_t getFreeSpace();
            size_t getTotalSpace();

            // Filesystem Initialization
            bool init(bool formatOnFail = true);
            bool format();
            void joinPath(const char* cwd, const char* path, char* outPath, size_t outLen);
            bool isDir(const char* path);

    }
}
