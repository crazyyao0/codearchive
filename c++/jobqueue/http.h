#include "cpio.h"
extern "C"
{
	#include "dynamic_object.h"
}

typedef struct HTTP_CONTEXT
{
	SOCKET socket;
	SocketAddress local;
	SocketAddress remote;

	int http_state;
	char* header_buf;
	int header_buf_len;
	int header_buf_sent;

	char* post_data;
	int post_len;
	int post_len_copied;

	char* return_data;
	char* return_content_type;
	int return_len;
	int return_len_sent;

	dynamic_object* req_hdr;
	dynamic_object* res_hdr;
}HTTP_CONTEXT;

typedef struct HTTP_ERROR_CODE
{
	int code;
	char *header;
	char *body;
}HTTP_ERROR_CODE;

typedef struct HTTP_MIME_TYPE
{
	char *fileext;
	char *type;
}HTTP_MIME_TYPE;

struct HTTP_CONFIG
{
	int maxHeaderSize;
	char serverName[80];
	char documentRoot[260];
	struct HTTP_ERROR_CODE codes[40];
	int codes_len;
};

typedef int (*CGI_FUNC)(HTTP_CONTEXT* context);

typedef struct HTTP_CGI
{
	char path[260];
	CGI_FUNC fn;
}HTTP_CGI;

void http_init_context(HTTP_CONTEXT* context);
void http_close_context(HTTP_CONTEXT* context);
void http_data_arrived(HTTP_CONTEXT* context, char* buffer, int len);
void http_data_sent(HTTP_CONTEXT* context, int len);
void http_precess_response(HTTP_CONTEXT* context);

void http_register_cgi(char *path, CGI_FUNC func);
int http_encode_url(char* src, int len, char* out);
int http_decode_url(char* src, int len, char* out);