#include <FS.h>
#include <SPIFFS.h>
#include "FreeRTOS.h"
#include "NekosFS.h"
namespace nekos {
    namespace fs {
        // Initialize filesystem
        bool initFS(bool formatOnFail = true) {
            if (!SPIFFS.begin(formatOnFail)) {
                Serial.println("[FS] Mount Failed");
                return false;
            }
            return true;
        }

        // Write string to file
        bool writeFile(const char* path, const String& content) {
            File file = SPIFFS.open(path, FILE_WRITE);
            if (!file) {
                Serial.printf("[FS] Failed to open file for writing: %s\n", path);
                return false;
            }
            file.print(content);
            file.close();
            return true;
        }

        bool touchFile(const char* path) {
            // If file exists, just return true
            if (SPIFFS.exists(path)) {
                return true;
            }
            // Create empty file
            File file = SPIFFS.open(path, FILE_WRITE);
            if (!file) {
                Serial.printf("[FS] Failed to create file: %s\n", path);
                return false;
            }
            file.close();
            return true;
        }

        bool appendFile(const char* path, const String& content) {
            // Ensure file exists
            if (!SPIFFS.exists(path)) {
                if (!touchFile(path)) {
                    return false;
                }
            }
            File file = SPIFFS.open(path, FILE_APPEND);
            if (!file) {
                Serial.printf("[FS] Failed to open file for appending: %s\n", path);
                return false;
            }
            file.print(content);
            file.close();
            return true;
        }

        // Read file content into String
        String readFile(const char* path) {
            File file = SPIFFS.open(path, FILE_READ);
            if (!file) {
                Serial.printf("[FS] Failed to open file for reading: %s\n", path);
                return "";
            }
            String content;
            while (file.available()) {
                content += (char)file.read();
            }
            file.close();
            return content;
        }

        // Delete file
        bool deleteFile(const char* path) {
            if (!SPIFFS.exists(path)) {
                Serial.printf("[FS] File not found: %s\n", path);
                return false;
            }
            return SPIFFS.remove(path);
        }

        // List all files in a directory
        void listDir(const char* dirname, uint8_t levels) {
            File root = SPIFFS.open(dirname);
            if (!root || !root.isDirectory()) {
                Serial.println("[FS] Failed to open directory");
                return;
            }
            File file = root.openNextFile();
            while (file) {
                if (file.isDirectory()) {
                    Serial.printf("  DIR : %s\n", file.name());
                    if (levels) {
                        listDir(file.name(), levels - 1);
                    }
                } else {
                    Serial.printf("  FILE: %s  SIZE: %u\n", file.name(), (unsigned int)file.size());
                }
                file = root.openNextFile();
            }
        }

        bool makeDir(const char* path) {
            if (!path || strlen(path) == 0) {
                Serial.println("[FS] mkdir: invalid path");
                return false;
            }
            if (SPIFFS.exists(path)) {
                Serial.printf("[FS] mkdir: already exists: %s\n", path);
                return false;
            }
            if (SPIFFS.mkdir(path)) {
                Serial.printf("[FS] mkdir: created: %s\n", path);
                return true;
            } else {
                Serial.printf("[FS] mkdir: failed: %s\n", path);
                return false;
            }
        }

        // Initialize SPIFFS, optionally format on failure
        bool init(bool formatOnFail) {
            if (!SPIFFS.begin(true)) {
                if (formatOnFail) {
                    SPIFFS.format();
                    return SPIFFS.begin();
                }
                return false;
            }
            return true;
        }

        // Format the SPIFFS filesystem
        bool format() {
            return SPIFFS.format();
        }

        // Copy file from srcPath to destPath
        bool copyFile(const char* srcPath, const char* destPath) {
            File src = SPIFFS.open(srcPath, FILE_READ);
            if (!src) return false;

            File dest = SPIFFS.open(destPath, FILE_WRITE);
            if (!dest) {
                src.close();
                return false;
            }

            constexpr size_t bufSize = 256;
            uint8_t buf[bufSize];
            size_t len;

            while ((len = src.read(buf, bufSize)) > 0) {
                if (dest.write(buf, len) != len) {
                    src.close();
                    dest.close();
                    return false;
                }
            }
            src.close();
            dest.close();
            return true;
        }

        // Move (rename) file from oldPath to newPath
        bool moveFile(const char* oldPath, const char* newPath) {
            if (!copyFile(oldPath, newPath)) return false;
            return SPIFFS.remove(oldPath);
        }

        // Check if a file or directory exists
        bool fileExists(const char* path) {
            return SPIFFS.exists(path);
        }

        // Get size of file in bytes, or 0 if not found
        size_t getFileSize(const char* path) {
            File file = SPIFFS.open(path, FILE_READ);
            if (!file) return 0;
            size_t size = file.size();
            file.close();
            return size;
        }

        // Recursively find files matching pattern (substring match) and add to outFiles
        void findFiles(const char* dirname, const char* pattern, std::vector<String>& outFiles, uint8_t levels) {
            File dir = SPIFFS.open(dirname);
            if (!dir || !dir.isDirectory()) return;

            File file = dir.openNextFile();
            while (file) {
                String name = file.name();
                if (file.isDirectory()) {
                    if (levels > 0) findFiles(name.c_str(), pattern, outFiles, levels - 1);
                } else {
                    if (name.indexOf(pattern) >= 0) {
                        outFiles.push_back(name);
                    }
                }
                file = dir.openNextFile();
            }
        }

        // Return free space in bytes on SPIFFS
        size_t getFreeSpace() {
            return SPIFFS.totalBytes() - SPIFFS.usedBytes();
        }

        // Return total space in bytes on SPIFFS
        size_t getTotalSpace() {
            return SPIFFS.totalBytes();
        }
        
        void joinPath(const char* cwd, const char* path, char* outPath, size_t outLen) {
            if (!path || path[0] == '/') {
                // absolute path or empty, just copy
                strncpy(outPath, path ? path : "/", outLen);
                outPath[outLen - 1] = '\0';
            } else if (cwd && cwd[0] != '\0') {
                snprintf(outPath, outLen, "%s/%s", cwd, path);
            } else {
                snprintf(outPath, outLen, "%s", path);
            }
        }
        
        bool isDir(const char* path) {
            if (!path || path[0] == '\0') return false;
            File file = SPIFFS.open(path);
            if (!file) {
                return false;
            }
            bool result = file.isDirectory();
            file.close();
            return result;
        }
    }
}

