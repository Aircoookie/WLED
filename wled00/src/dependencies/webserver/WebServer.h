/*
  WebServer.h - Dead simple web-server.
  Supports only one simultaneous client, knows how to handle GET and POST.

  Copyright (c) 2014 Ivan Grokhotkov. All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
  Modified 8 May 2015 by Hristo Gochkov (proper post and file upload handling)
*/


#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <functional>
#ifdef ESP8266
#define WebServer ESP8266WebServer
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#define write_P   write
#endif

enum HTTPMethod { HTTP_ANY, HTTP_GET, HTTP_POST, HTTP_PUT, HTTP_PATCH, HTTP_DELETE, HTTP_OPTIONS };
enum HTTPUploadStatus { UPLOAD_FILE_START, UPLOAD_FILE_WRITE, UPLOAD_FILE_END,
                        UPLOAD_FILE_ABORTED };
enum HTTPClientStatus { HC_NONE, HC_WAIT_READ, HC_WAIT_CLOSE };

#define HTTP_DOWNLOAD_UNIT_SIZE 1460

#ifndef HTTP_UPLOAD_BUFLEN
#define HTTP_UPLOAD_BUFLEN 2048
#endif

#define HTTP_MAX_DATA_WAIT 1000 //ms to wait for the client to send the request
#define HTTP_MAX_POST_WAIT 1000 //ms to wait for POST data to arrive
#define HTTP_MAX_SEND_WAIT 5000 //ms to wait for data chunk to be ACKed
#define HTTP_MAX_CLOSE_WAIT 2000 //ms to wait for the client to close the connection

#define CONTENT_LENGTH_UNKNOWN ((size_t) -1)
#define CONTENT_LENGTH_NOT_SET ((size_t) -2)

class WebServer;

typedef struct {
  HTTPUploadStatus status;
  String  filename;
  String  name;
  String  type;
  size_t  totalSize;    // file size
  size_t  currentSize;  // size of data currently in buf
  uint8_t buf[HTTP_UPLOAD_BUFLEN];
} HTTPUpload;

#include "detail/RequestHandler.h"

namespace fs {
class FS;
}

class WebServer
{
public:
  WebServer(IPAddress addr, int port = 80);
  WebServer(int port = 80);
  ~WebServer();

  void begin();
  void handleClient();

  void close();
  void stop();

  bool authenticate(const char * username, const char * password);
  void requestAuthentication();

  typedef std::function<void(void)> THandlerFunction;
  void on(const String &uri, THandlerFunction handler);
  void on(const String &uri, HTTPMethod method, THandlerFunction fn);
  void on(const String &uri, HTTPMethod method, THandlerFunction fn, THandlerFunction ufn);
  void addHandler(RequestHandler* handler);
  void serveStatic(const char* uri, fs::FS& fs, const char* path, const char* cache_header = NULL );
  void onNotFound(THandlerFunction fn);  //called when handler is not assigned
  void onFileUpload(THandlerFunction fn); //handle file uploads

  String uri() { return _currentUri; }
  HTTPMethod method() { return _currentMethod; }
  WiFiClient client() { return _currentClient; }
  HTTPUpload& upload() { return _currentUpload; }

  String arg(String name);        // get request argument value by name
  String arg(int i);              // get request argument value by number
  String argName(int i);          // get request argument name by number
  int args();                     // get arguments count
  bool hasArg(String name);       // check if argument exists
  void collectHeaders(const char* headerKeys[], const size_t headerKeysCount); // set the request headers to collect
  String header(String name);      // get request header value by name
  String header(int i);              // get request header value by number
  String headerName(int i);          // get request header name by number
  int headers();                     // get header count
  bool hasHeader(String name);       // check if header exists

  String hostHeader();            // get request host header if available or empty String if not

  // send response to the client
  // code - HTTP response code, can be 200 or 404
  // content_type - HTTP content type, like "text/plain" or "image/png"
  // content - actual content body
  void send(int code, const char* content_type = NULL, const String& content = String(""));
  void send(int code, char* content_type, const String& content);
  void send(int code, const String& content_type, const String& content);
  void send_P(int code, PGM_P content_type, PGM_P content);
  void send_P(int code, PGM_P content_type, PGM_P content, size_t contentLength);

  void setContentLength(size_t contentLength);
  void sendHeader(const String& name, const String& value, bool first = false);
  void sendContent(const String& content);
  void sendContent_P(PGM_P content);
  void sendContent_P(PGM_P content, size_t size);

  static String urlDecode(const String& text);

#ifdef ESP8266
template<typename T> size_t streamFile(T &file, const String& contentType){
  setContentLength(file.size());
  if (String(file.name()).endsWith(".gz") &&
      contentType != "application/x-gzip" &&
      contentType != "application/octet-stream"){
    sendHeader("Content-Encoding", "gzip");
  }
  send(200, contentType, "");
  return _currentClient.write(file);
}
#else
template<typename T> size_t streamFile(T &file, const String& contentType){
#define STREAMFILE_BUFSIZE 2*1460
  setContentLength(file.size());
  if (String(file.name()).endsWith(".gz") &&
      contentType != "application/x-gzip" &&
      contentType != "application/octet-stream") {
    sendHeader("Content-Encoding", "gzip");
  }
  send(200, contentType, "");
  uint8_t *buf = (uint8_t *)malloc(STREAMFILE_BUFSIZE);
  if (buf == NULL) {
    //DBG_OUTPUT_PORT.printf("streamFile malloc failed");
    return 0;
  }
  size_t totalBytesOut = 0;
  while (client().connected() && (file.available() > 0)) {
    int bytesOut;
    int bytesIn = file.read(buf, STREAMFILE_BUFSIZE);
    if (bytesIn <= 0) break;
    while (1) {
      bytesOut = 0;
      if (!client().connected()) break;
      bytesOut = client().write(buf, bytesIn);
      if (bytesIn == bytesOut) break;

      //DBG_OUTPUT_PORT.printf("bytesIn %d != bytesOut %d\r\n",
      //bytesIn, bytesOut);
      delay(1);
    }
    totalBytesOut += bytesOut;
    yield();
  }
  if (totalBytesOut != file.size()) {
    //DBG_OUTPUT_PORT.printf("file size %d bytes out %d\r\n",
    //    file.size(), totalBytesOut);
  }
  free(buf);
  return totalBytesOut;
}
#endif

protected:
  void _addRequestHandler(RequestHandler* handler);
  void _handleRequest();
  bool _parseRequest(WiFiClient& client);
  void _parseArguments(String data);
  static String _responseCodeToString(int code);
  bool _parseForm(WiFiClient& client, String boundary, uint32_t len);
  bool _parseFormUploadAborted();
  void _uploadWriteByte(uint8_t b);
  uint8_t _uploadReadByte(WiFiClient& client);
  void _prepareHeader(String& response, int code, const char* content_type, size_t contentLength);
  bool _collectHeader(const char* headerName, const char* headerValue);

  struct RequestArgument {
    String key;
    String value;
  };

  WiFiServer  _server;

  WiFiClient  _currentClient;
  HTTPMethod  _currentMethod;
  String      _currentUri;
  uint8_t     _currentVersion;
  HTTPClientStatus _currentStatus;
  unsigned long _statusChange;

  RequestHandler*  _currentHandler;
  RequestHandler*  _firstHandler;
  RequestHandler*  _lastHandler;
  THandlerFunction _notFoundHandler;
  THandlerFunction _fileUploadHandler;

  int              _currentArgCount;
  RequestArgument* _currentArgs;
  HTTPUpload       _currentUpload;

  int              _headerKeysCount;
  RequestArgument* _currentHeaders;
  size_t           _contentLength;
  String           _responseHeaders;

  String           _hostHeader;
  bool             _chunked;

};


#endif //WEBSERVER_H
