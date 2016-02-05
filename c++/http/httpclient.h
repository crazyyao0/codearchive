#include "socket.h"
#include "dynamic_object.h"
#include "httpcommon.h"
#include "zlib.h"

typedef struct http_stream
{
	socket_conn* socket;

	int content_length;
	int dynamic_content;
	int chunk_encoding;
	int compress_encoding;

	char* raw;
	int rawmax;
	int rawp;

	char* compress_in;
	int compress_len;
	z_stream* zstream;
}http_stream;

socket_conn* http_send_request(char* method, char* url, dynamic_object* reqhers);
http_stream* http_get_response_header(socket_conn* conn, dynamic_object* headers);
int http_stream_read(http_stream* stream, char* buffer, int len);
void http_close_stream(http_stream* stream);
int http_simple_get(char* url, dynamic_object* reqhers, dynamic_object* reshdrs, char** out);
