#ifndef HTTP_CONST_H
#define HTTP_CONST_H

/* HTTP Status Codes */
#define HTTP_STATUS_OK 200
#define HTTP_STATUS_BAD_REQUEST 400
#define HTTP_STATUS_NOT_FOUND 404
#define HTTP_STATUS_TEAPOT 418
#define HTTP_STATUS_INTERNAL_ERROR 500
#define HTTP_STATUS_NOT_IMPL 501

/* Standard HTTP response messages */
#define HTTP_MSG_NOT_IMPL "Not Implemented"

/* Standard HTTP Headers */
#define HTTP_HDR_ACCESS_CONTROL_ALLOW_METHODS "Access-Control-Allow-Methods"
#define HTTP_HDR_ACCESS_CONTROL_ALLOW_ORIGIN "Access-Control-Allow-Origin"
#define HTTP_HDR_ACCESS_CONTROL_ALLOW_HEADERS "Access-Control-Allow-Headers"
#define HTTP_HDR_CACHE_CONTROL "Cache-Control"
#define HTTP_HDR_CONTENT_TYPE "Content-Type"
#define HTTP_HDR_CONTENT_ENCODING "Content_Encoding"
#define HTTP_HDR_ETAG "Etag"
#define HTTP_HDR_IF_NONE_MATCH "If-None-Match"
#define HTTP_HDR_HOST "Host"
#define HTTP_HDR_LOCATION "Location"

/* Common MIME types for Content-Type headers */
#define CT_APPLICATION_JSON "application/json"
#define CT_TEXT_PLAIN "text/plain"
#define CT_TEXT_HTML "text/html"
#define CT_IMAGE_XICON "image/x-icon"

#endif 