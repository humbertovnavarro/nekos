#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "embedded_files.h"
#include <pgmspace.h>
#include "SPIFFS.h"
#include "NekosHTTP.h"
namespace nekos {
namespace http {

WebServer server(80);
TaskHandle_t webServerHandle;

void getFiles() {
  String json = "[";
  File root = SPIFFS.open("/");
  if (!root || !root.isDirectory()) {
    server.send(500, "text/plain", "Failed to open root directory");
    return;
  }
  File file = root.openNextFile();
  bool first = true;
  while (file) {
    if (!first) json += ",";
    json += "{\"name\":\"" + String(file.name()) + "\",\"size\":" + String(file.size()) + "}";
    first = false;
    file = root.openNextFile();
  }
  json += "]";
  server.send(200, "application/json", json);
}

void deleteFile() {
  if (server.hasArg("file")) {
    String path = server.arg("file");
    if (SPIFFS.exists("/" + path)) {
      SPIFFS.remove("/" + path);
      server.send(200, "text/plain", "Deleted");
      return;
    }
  }
  server.send(404, "text/plain", "File not found");
}

void postFile() {
    if (!server.hasArg("plain")) { // "plain" contains the raw body
        server.send(400, "text/plain", "No file content received");
        return;
    }
    if(!server.hasArg("file")) {
      server.send(404, "text/plain", "No file specified");
      return;
    }
    String filename = "/" + server.arg("file"); // or get it from URL/query if you like
    File f = SPIFFS.open(filename, FILE_WRITE);
    if (!f) {
        server.send(500, "text/plain", "Failed to open file for writing");
        return;
    }
    String body = server.arg("plain"); // the raw POST body
    f.print(body); // write to SPIFFS
    f.close();
    Serial.printf("Upload complete: %s (%u bytes)\n", filename.c_str(), body.length());
    server.send(200, "text/plain", "Upload complete");
}


void getFile() {
  if(!server.hasArg("file")) {
    server.send(404, "text/plain", "File not found");
    return;
  }
  String path = server.arg("file");
  if(!SPIFFS.exists("/" + path)) {
    server.send(404, "text/plain", "File not found");
    return;
  }
  File f = SPIFFS.open("/" + path);
  server.streamFile(f, "text/plain", 200);
}

// Embedded file lookup
const EmbeddedFile* findEmbeddedFile(const String& path) {
  for (size_t i = 0; i < embeddedFileCount; ++i) {
    if (path.equals(embeddedFiles[i].path)) return &embeddedFiles[i];
  }
  return nullptr;
}

void serveStatic() {
  String path = server.uri();
  if (path == "/") path = "index.html";
  else if (path.startsWith("/")) path = path.substring(1);

  const EmbeddedFile* file = findEmbeddedFile(path);
  if (!file) {
    server.send(404, "text/plain", "Not Found");
    return;
  }

  String contentType = "text/plain";
  if (path.endsWith(".html")) contentType = "text/html";
  else if (path.endsWith(".js")) contentType = "application/javascript";
  else if (path.endsWith(".css")) contentType = "text/css";
  else if (path.endsWith(".png")) contentType = "image/png";
  else if (path.endsWith(".jpg")) contentType = "image/jpeg";
  else if (path.endsWith(".ico")) contentType = "image/x-icon";

  server.send_P(200, contentType.c_str(), (PGM_P)file->data, file->size);
}

void webServerTask(void* pvparams) {
  server.handleClient();
}


void begin() {
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS Mount Failed");
    abort();
  }
  server.on("/api/file", HTTP_DELETE, deleteFile);
  server.on("/api/file", HTTP_POST, postFile);
  server.on("/api/file", HTTP_GET, getFile);
  server.on("/api/files", HTTP_GET, getFiles);
  server.onNotFound(serveStatic);
  server.begin();
  xTaskCreate(webServerTask, "webserver", 8192, nullptr, 1, &webServerHandle);
}

}
}
