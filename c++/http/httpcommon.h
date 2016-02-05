#ifndef HTTP_COMMON_INCLUDED
#define HTTP_COMMON_INCLUDED

#include "dynamic_object.h"
#include "trex.h"
#include <time.h>

#define HTTP_HEADER_ACCEPT "Accept"
#define HTTP_HEADER_ACCEPT_CHARSET "Accept-Charset"
#define HTTP_HEADER_ACCEPT_DATETIME "Accept-Datetime"
#define HTTP_HEADER_ACCEPT_ENCODING "Accept-Encoding"
#define HTTP_HEADER_ACCEPT_LANGUAGE "Accept-Language"
#define HTTP_HEADER_ACCEPT_RANGES "Accept-Ranges"
#define HTTP_HEADER_ACCESS_CONTROL_ALLOW_ORIGIN "Access-Control-Allow-Origin"
#define HTTP_HEADER_AGE "Age"
#define HTTP_HEADER_ALLOW "Allow"
#define HTTP_HEADER_AUTHORIZATION "Authorization"
#define HTTP_HEADER_CACHE_CONTROL "Cache-Control"
#define HTTP_HEADER_CONNECTION "Connection"
#define HTTP_HEADER_CONTENT_DISPOSITION "Content-Disposition"
#define HTTP_HEADER_CONTENT_ENCODING "Content-Encoding"
#define HTTP_HEADER_CONTENT_LANGUAGE "Content-Language"
#define HTTP_HEADER_CONTENT_LENGTH "Content-Length"
#define HTTP_HEADER_CONTENT_LOCATION "Content-Location"
#define HTTP_HEADER_CONTENT_MD5 "Content-MD5"
#define HTTP_HEADER_CONTENT_RANGE "Content-Range"
#define HTTP_HEADER_CONTENT_TYPE "Content-Type"
#define HTTP_HEADER_COOKIE "Cookie"
#define HTTP_HEADER_DATE "Date"
#define HTTP_HEADER_ETAG "ETag"
#define HTTP_HEADER_EXPECT "Expect"
#define HTTP_HEADER_EXPIRES "Expires"
#define HTTP_HEADER_FROM "From"
#define HTTP_HEADER_HOST "Host"
#define HTTP_HEADER_IF_MATCH "If-Match"
#define HTTP_HEADER_IF_MODIFIED_SINCE "If-Modified-Since"
#define HTTP_HEADER_IF_NONE_MATCH "If-None-Match"
#define HTTP_HEADER_IF_RANGE "If-Range"
#define HTTP_HEADER_IF_UNMODIFIED_SINCE "If-Unmodified-Since"
#define HTTP_HEADER_LAST_MODIFIED "Last-Modified"
#define HTTP_HEADER_LINK "Link"
#define HTTP_HEADER_LOCATION "Location"
#define HTTP_HEADER_MAX_FORWARDS "Max-Forwards"
#define HTTP_HEADER_ORIGIN "Origin"
#define HTTP_HEADER_P3P "P3P"
#define HTTP_HEADER_PRAGMA "Pragma"
#define HTTP_HEADER_PROXY_AUTHENTICATE "Proxy-Authenticate"
#define HTTP_HEADER_PROXY_AUTHORIZATION "Proxy-Authorization"
#define HTTP_HEADER_RANGE "Range"
#define HTTP_HEADER_REFERER "Referer"
#define HTTP_HEADER_REFRESH "Refresh"
#define HTTP_HEADER_RETRY_AFTER "Retry-After"
#define HTTP_HEADER_SERVER "Server"
#define HTTP_HEADER_SET_COOKIE "Set-Cookie"
#define HTTP_HEADER_STATUS "Status"
#define HTTP_HEADER_STRICT_TRANSPORT_SECURITY "Strict-Transport-Security"
#define HTTP_HEADER_TE "TE"
#define HTTP_HEADER_TRAILER "Trailer"
#define HTTP_HEADER_TRANSFER_ENCODING "Transfer-Encoding"
#define HTTP_HEADER_UPGRADE "Upgrade"
#define HTTP_HEADER_USER_AGENT "User-Agent"
#define HTTP_HEADER_VARY "Vary"
#define HTTP_HEADER_VIA "Via"
#define HTTP_HEADER_WARNING "Warning"
#define HTTP_HEADER_WWW_AUTHENTICATE "WWW-Authenticate"

typedef struct HTTP_ERROR_CODE
{
	int code;
	char *header;
	char *body;
}HTTP_ERROR_CODE;

typedef struct HTTP_MIME_MAP
{
	char *fileExt;
	char *mime;
}HTTP_MIME_MAP;

extern TRex *g_rex_url;
extern TRex *g_rex_reqhdr;
extern TRex *g_rex_reshdr;
extern TRex *g_rex_httphdr;
extern TRex *g_rex_url_encoded;

// need at least trible buffer size 
int http_encode_url(char* src, int len, char* out);

// need at least the same buffer size
int http_decode_url(char* src, int len, char* out);

int http_parse_urlencoded(dynamic_object* parameter, char* buf);
time_t http_parse_time(char* str);
int http_format_time(time_t tick, char* str);

void http_init();

HTTP_ERROR_CODE* http_find_code(int code);
HTTP_MIME_MAP* http_find_mime(char* fileext);

#endif
