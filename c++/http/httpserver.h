#include "socket.h"
#include "dynamic_object.h"
#include "httpcommon.h"

typedef struct HTTP_CONFIG
{
	char serverName[80];
	char documentRoot[260];
	char defaultPage[80];
}HTTP_CONFIG;

extern HTTP_CONFIG g_http_config;

int http_server_init(socket_addr* addr, int addrnum);
int http_send_response(socket_conn* conn, int code, dynamic_object* reqhdrs);
int http_read_request(socket_conn* conn, dynamic_object* reqhdrs);
int http_send_file(socket_conn* conn, dynamic_object * req);
HTTP_ERROR_CODE* http_find_code(int code);
HTTP_MIME_MAP* http_find_mime(char* fileext);