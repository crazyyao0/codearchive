#include "cpio.h"
#include "http.h"
extern "C"
{
	#include "trex.h"
}
#include <stdio.h>

#define snprintf _snprintf
#define _atoi64 atoll

HTTP_CONFIG g_http_config = {8192, 
	"my_http_server", 
	"C:\\Program Files\\Apache Software Foundation\\Apache2.2\\htdocs",
	{
		{100, "HTTP/1.1 100 Continue",						""},
		{101, "HTTP/1.1 101 Switching Protocols",			""},
		{200, "HTTP/1.1 200 OK",							""},
		{201, "HTTP/1.1 201 Created",						""},
		{202, "HTTP/1.1 202 Accepted",						""},
		{203, "HTTP/1.1 203 Non-Authoritative Information",	""},
		{204, "HTTP/1.1 204 No Content",					""},
		{205, "HTTP/1.1 205 Reset Content",					""},
		{206, "HTTP/1.1 206 Partial Content",				""},
		{300, "HTTP/1.1 300 Multiple Choices",				""},
		{301, "HTTP/1.1 301 Moved Permanently",				""},
		{302, "HTTP/1.1 302 Found",							""},
		{303, "HTTP/1.1 303 See Other",						""},
		{304, "HTTP/1.1 304 Not Modified",					""},
		{305, "HTTP/1.1 305 Use Proxy",						""},
		{307, "HTTP/1.1 307 Temporary Redirect",			""},
		{400, "HTTP/1.1 400 Bad Request",					"<html><body>Bad Request</body></html>"},
		{401, "HTTP/1.1 401 Unauthorized",					"<html><body>Unauthorized</body></html>"},
		{402, "HTTP/1.1 402 Payment Required",				"<html><body>Payment Required</body></html>"},
		{403, "HTTP/1.1 403 Forbidden",						"<html><body>Forbidden</body></html>"},
		{404, "HTTP/1.1 404 Not Found",						"<html><body>Not Found</body></html>"},
		{405, "HTTP/1.1 405 Method Not Allowed",			"<html><body>Method Not Allowed</body></html>"},
		{406, "HTTP/1.1 406 Not Acceptable",				"<html><body>Not Acceptable</body></html>"},
		{407, "HTTP/1.1 407 Proxy Authentication Required",	"<html><body>Proxy Authentication Required</body></html>"},
		{408, "HTTP/1.1 408 Request Time-out",				"<html><body>Request Time-out</body></html>"},
		{409, "HTTP/1.1 409 Conflict",						"<html><body>Conflict</body></html>"},
		{410, "HTTP/1.1 410 Gone",							"<html><body>Gone</body></html>"},
		{411, "HTTP/1.1 411 Length Required",				"<html><body>Length Required</body></html>"},
		{412, "HTTP/1.1 412 Precondition Failed",			"<html><body>Precondition Failed</body></html>"},
		{413, "HTTP/1.1 413 Request Entity Too Large",		"<html><body>Request Entity Too Large</body></html>"},
		{414, "HTTP/1.1 414 Request-URI Too Large",			"<html><body>Request-URI Too Large</body></html>"},
		{415, "HTTP/1.1 415 Unsupported Media Type",		"<html><body>Unsupported Media Type</body></html>"},
		{416, "HTTP/1.1 416 Requested range not satisfiable","<html><body>Requested range not satisfiable</body></html>"},
		{417, "HTTP/1.1 417 Expectation Failed",			"<html><body>Expectation Failed</body></html>"},
		{500, "HTTP/1.1 500 Internal Server Error",			"<html><body>Internal Server Error</body></html>"},
		{501, "HTTP/1.1 501 Not Implemented",				"<html><body>Not Implemented</body></html>"},
		{502, "HTTP/1.1 502 Bad Gateway",					"<html><body>Bad Gateway</body></html>"},
		{503, "HTTP/1.1 503 Service Unavailable",			"<html><body>Service Unavailable</body></html>"},
		{504, "HTTP/1.1 504 Gateway Time-out",				"<html><body>Gateway Time-out</body></html>"},
		{505, "HTTP/1.1 505 HTTP Version not supported",	"<html><body>HTTP Version not supported</body></html>"},
	},
	40,
};

#define HTTPSTATE_WAITHEADER 0
#define HTTPSTATE_READPOSTDATA 1
#define HTTPSTATE_GETRESPONSE 2
#define HTTPSTATE_SENTHEADER 3
#define HTTPSTATE_SENTBODY 4

TRex *tRexURL;
TRex *tRexHeader;
dynamic_object* cgi;

void http_init()
{
	cpio_init(sizeof(HTTP_CONTEXT));
	SocketAddress addr;
	addr.family = AF_INET;
	addr.addrv4.sin_port = htons(80);
	addr.addrv4.sin_addr.s_addr = INADDR_ANY;
	cpio_init_socket(&addr);

	const char *error = NULL;
	tRexURL = trex_compile("^([^ ]+) (/[^?# ]*)(\\?[^# ]*)?(#[^ ]*)? HTTP/([^ ]+)$", &error);
	tRexHeader = trex_compile("^([^ ]+): (.+)$", &error);

	cgi = dynamic_create();
}

void http_init_context(HTTP_CONTEXT* context)
{
	context->header_buf = (char*)malloc(g_http_config.maxHeaderSize);
	context->header_buf_len = 0;
	context->post_data = 0;
	context->post_len = 0;
	context->return_data = 0;
	context->return_len = 0;
	context->http_state = HTTPSTATE_WAITHEADER;
	context->req_hdr = dynamic_create();
	context->res_hdr = dynamic_create();
}

void http_close_context(HTTP_CONTEXT* context)
{
	if(context->header_buf)
	{
		free(context->header_buf);
		context->header_buf = 0;
	}

	if(context->post_data)
	{
		free(context->post_data);
		context->post_data = 0;
	}

	if(context->return_data)
	{
		free(context->return_data);
		context->return_data = 0;
	}

	if(context->req_hdr)
	{
		dynamic_delete(context->req_hdr);
		context->req_hdr = 0;
	}

	if(context->res_hdr)
	{
		dynamic_delete(context->res_hdr);
		context->res_hdr = 0;
	}
}

void http_send_response(HTTP_CONTEXT* context, int code)
{
	int count = 0;
	int r;
	dynamic_object* h = context->res_hdr;

	do
	{
		dynamic_hash_entry* entry = dynamic_map_first(context->res_hdr);
		HTTP_ERROR_CODE * strcode = g_http_config.codes + 34;
		for(r = 0;r<g_http_config.codes_len;r++)
		{
			if(g_http_config.codes[r].code == code)
			{
				strcode = g_http_config.codes + r;
				break;
			}
		}

		if(strcode->code >=400 && context->return_data==0)
		{
			context->return_data = (char*)malloc(128);
			context->return_content_type = "text/html";
			context->return_len = sprintf(context->return_data, strcode->body);
		}

		r=snprintf(context->header_buf + count, g_http_config.maxHeaderSize - count, "%s\r\n", strcode->header);
		if(r<0)	break;
		count += r;

		r = snprintf(context->header_buf + count, g_http_config.maxHeaderSize - count, "Server: %s\r\n", g_http_config.serverName);
		if(r<0)	break;
		count += r;

		if(context->return_len)
		{
			r = snprintf(context->header_buf + count, g_http_config.maxHeaderSize - count, "Content-Type: %s\r\n", context->return_content_type);
			if(r<0)	break;
			count += r;

			r = snprintf(context->header_buf + count, g_http_config.maxHeaderSize - count, "Content-Length: %d\r\n", context->return_len);
			if(r<0)	break;
			count += r;
		}

		while(entry)
		{
			r = snprintf(context->header_buf + count, g_http_config.maxHeaderSize - count, "%s: %s\r\n", entry->key, dynamic_get_string(&entry->value));
			if(r<0)	break;
			count += r;
			entry = dynamic_map_next(context->res_hdr, entry);
		}
		if(r<0)	break;

		r = snprintf(context->header_buf + count, g_http_config.maxHeaderSize - count, "Connection: close\r\n\r\n");
		if(r<0)	break;
		count += r;

	}while(0);

	if(r<0)
	{
		// response header is too large
		dynamic_empty_map(context->res_hdr);
		if(context->return_data)
		{
			free(context->return_data);
			context->return_data = 0;
			context->return_len = 0;
		}
		http_send_response(context, 500);
		return ;
	}

	context->header_buf_len = count;
	context->header_buf_sent = 0;
	context->http_state = HTTPSTATE_SENTHEADER;

	cpio_send(context->socket, context->header_buf, context->header_buf_len);
}

int http_parse_url(HTTP_CONTEXT* context, char*line, char* end)
{
	const char *a,*b;
	dynamic_object* h = context->req_hdr;
	*end = 0;
	
	if(trex_search(tRexURL, line, &a, &b))
	{
		TRexMatch match;
		trex_getsubexp(tRexURL, 1, &match);
		dynamic_set_nstring(dynamic_map_insert(h, "METHOD"), (char*)match.begin, match.len);

		trex_getsubexp(tRexURL, 2, &match);
		dynamic_set_nstring(dynamic_map_insert(h, "PATH"), (char*)match.begin, match.len);

		trex_getsubexp(tRexURL, 3, &match);
		if(match.len > 1)
			dynamic_set_nstring(dynamic_map_insert(h, "QUERY_STRING"), (char*)match.begin + 1, match.len - 1);
		
		trex_getsubexp(tRexURL, 4, &match);
		if(match.len > 1)
			dynamic_set_nstring(dynamic_map_insert(h, "FRAGMENT_ID"), (char*)match.begin + 1, match.len - 1);

		trex_getsubexp(tRexURL, 5, &match);
		dynamic_set_nstring(dynamic_map_insert(h, "HTTP_VERSION"), (char*)match.begin, match.len);

		return 1;
	}else
	{
		return 0;
	}
}

int http_parse_header(HTTP_CONTEXT* context, char*line, char* end)
{
	const char *a,*b;
	dynamic_object* h = context->req_hdr;
	*end = 0;
	
	if(trex_search(tRexHeader, line, &a, &b))
	{
		char headername[32];
		TRexMatch name;
		TRexMatch value;
		trex_getsubexp(tRexHeader, 1, &name);
		trex_getsubexp(tRexHeader, 2, &value);

		if(name.len>31)
			return 0;

		memcpy(headername, name.begin, name.len);
		headername[name.len]=0;
		dynamic_set_nstring(dynamic_map_insert(h, headername), (char*)value.begin, value.len);
		return 1;
	}else
	{
		return 0;
	}
}

void http_precess_response(HTTP_CONTEXT* context)
{
	dynamic_object * obj = dynamic_map_find(cgi, dynamic_get_string(dynamic_map_find(context->req_hdr, "PATH")));
	if(obj && dynamic_is_bytes(obj))
	{
		HTTP_CGI *c = (HTTP_CGI*)dynamic_as_bytes(obj);
		http_send_response(context, c->fn(context));
	}else
	{
		http_send_response(context, 404);
	}
}

void http_data_arrived(HTTP_CONTEXT* context, char* buffer, int len)
{
	char *start;
	char *end;
	switch(context->http_state)
	{
	case HTTPSTATE_WAITHEADER:
		if(context->header_buf_len + len > g_http_config.maxHeaderSize)
		{
			// exceed the max head size
			http_send_response(context, 400);
			return;
		}
		memcpy(context->header_buf + context->header_buf_len, buffer, len);
		if(context->header_buf_len > 4)
			start = context->header_buf + context->header_buf_len - 4;
		else
			start = context->header_buf;
		context->header_buf_len += len;
		end = strstr(start, "\r\n\r\n");
		if(end)
		{
			*(end + 2) = 0;
			char* line = context->header_buf;
			char* lineend = strstr(line, "\r\n");
			if(lineend == 0 || http_parse_url(context, line, lineend) == 0)
			{
				// wrong header format
				http_send_response(context, 400);
				return;
			}
			*lineend = 0;
			printf("%s\n", line);
			line = lineend + 2;
			while(line < end)
			{
				lineend = strstr(line, "\r\n");
				if(lineend == 0 || http_parse_header(context, line, lineend) == 0)
				{
					// wrong header format
					http_send_response(context, 400);
					return;
				}
				line = lineend + 2;
			}
			dynamic_object* content_type = dynamic_map_find(context->req_hdr, "Content-Type");
			if(content_type)
			{
				dynamic_object* content_length = dynamic_map_find(context->req_hdr, "Content-Length");
				if(content_length == 0)
				{
					http_send_response(context, 400);
					return;
				}
				context->post_len = atoi(dynamic_get_string(content_length));
				start = end + 4;
				context->http_state = HTTPSTATE_READPOSTDATA;

				if(context->post_len < 65536)
				{
					context->post_len_copied = len - (start - context->header_buf);
					context->post_data = (char*)malloc(context->post_len + 1);
					if(context->post_len_copied)
						memcpy(context->post_data, start, context->post_len_copied);

					if(context->post_len_copied == context->post_len)
					{
						context->http_state = HTTPSTATE_GETRESPONSE;
						http_precess_response(context);
					}
				}else
				{
					http_send_response(context, 500);
					// to large post data
				}
			}else
			{
				context->http_state = HTTPSTATE_GETRESPONSE;
				http_precess_response(context);
			}
		}
		break;
	case HTTPSTATE_READPOSTDATA:
		memcpy(context->post_data + context->post_len_copied, buffer, len);
		context->post_len_copied += len;
		if(context->post_len_copied == context->post_len)
		{
			context->http_state = HTTPSTATE_GETRESPONSE;
			http_precess_response(context);
		}
		break;
	}
}

void http_data_sent(HTTP_CONTEXT* context, int len)
{
	switch(context->http_state)
	{
	case HTTPSTATE_SENTHEADER:
		context->header_buf_sent += len;
		if(context->header_buf_sent == context->header_buf_len)
		{
			if(context->return_len)
			{
				context->http_state = HTTPSTATE_SENTBODY;
				context->return_len_sent = 0;
				cpio_send(context->socket, context->return_data, context->return_len);
			}
			else
			{
				context->http_state = HTTPSTATE_WAITHEADER;
			}
		}else
		{
			cpio_send(context->socket, context->header_buf + context->header_buf_sent, context->header_buf_len - context->header_buf_sent);
		}
		break;
	case HTTPSTATE_SENTBODY:
		context->return_len_sent += len;
		if(context->return_len_sent == context->return_len)
		{
			cpio_close_socket(context->socket);
		}else
		{
			cpio_send(context->socket, context->return_data + context->return_len_sent, context->return_len - context->return_len_sent);
		}
		break;
	}
}

void http_register_cgi(char *path, CGI_FUNC func)
{
	HTTP_CGI c;
	c.fn = func;
	strcpy(c.path, path);

	dynamic_set_bytes(dynamic_map_insert(cgi, path), (char*)&c, sizeof(HTTP_CGI));
}

int http_decode_url(char* src, int len, char* out)
{
	int outlen = 0;
	char* end = src + len;
	while(src<end)
	{
		if(*src == '%')
		{
			if(src + 2 >= end)
				return 0;

			int a = src[1];
			int b = src[2];

			if(a>='a' && a<='f')
				a = a-'a'+10;
			else if(a>='A' && a<='F')
				a = a-'A'+10;
			else if(a>='0' && a<='9')
				a = a-'0';
			else
				return 0;

			if(b>='a' && b<='f')
				b = b-'a'+10;
			else if(b>='A' && b<='F')
				b = b-'A'+10;
			else if(b>='0' && b<='9')
				b = b-'0';
			else
				return 0;
			out[outlen] = (char)(a * 16 + b);
			src += 3;
			outlen ++;
		}else if(*src == '+')
		{
			out[outlen] = ' ';
			src ++;
			outlen ++;
		}
		else
		{
			out[outlen] = *src;
			src ++;
			outlen ++;
		}
	}
	return outlen;
}

int http_encode_url(char* src, int len, char* out)
{
	int outlen = 0;
	char* end = src + len;
	while(src < end)
	{
		if(*src >= 'A' && *src<='Z' || *src>='a' && *src<='z' || *src>='0' && *src<='9' ||
			*src == '.' || *src == '~' || *src == '-' || *src == '_')
		{
			out[outlen] = *src;
			outlen ++;
			src ++;
		}else if(*src == ' ')
		{
			out[outlen] = '+';
			outlen ++;
			src ++;
		}else
		{
			int a, b;
			out[outlen] = '%';
			a = (*src >> 4) & 0xF;
			b = *src & 0xF;
			if(a>=10)
				out[outlen + 1] = a + 'A' - 10;
			else
				out[outlen + 1] = a + '0';
			
			if(b>=10)
				out[outlen + 2] = b + 'A' - 10;
			else
				out[outlen + 2] = b + '0';

			src ++;
			outlen += 3;
		}
	}
	return outlen;
}

char* http_parse_urlencoded(char* buf, char** name, char** value)
{
	*name = 0;
	*value= 0;

	while(*buf == '&' || *buf == '=')
		buf ++;

	if(*buf == 0)
		return 0;

	*name = buf;
	while(*buf && *buf != '&' && *buf != '=')
		buf ++;

	if(*buf == '&' || *buf == 0)
	{
		*buf = 0;
		return buf + 1;
	}else if(*buf == '=')
	{
		*buf = 0;
		buf ++;
		*value = buf; 

		while(*buf && *buf != '&')
			buf ++;

		if(*buf == '&')
		{
			*buf = 0;
			return buf + 1;
		}
		else
		{
			return 0;
		}
	}else
	{
		return 0;
	}
}

void http_parse_request_parameters(HTTP_CONTEXT* context)
{
	dynamic_object* hdr = context->req_hdr;
	dynamic_object* params = dynamic_map_insert(hdr, "params");
	dynamic_object* query_string = dynamic_map_find(hdr, "QUERY_STRING");
	dynamic_object* content_type = dynamic_map_find(hdr, "Content-Type");

	if(query_string)
	{
		char* str = dynamic_get_string(query_string);
		char* name;
		char* value;

		while(str)
		{
			str = http_parse_urlencoded(str, &name, &value);
			if(name)
			{
				if(value)
				{
					dynamic_set_string(dynamic_map_insert(params, name), value);
				}else
				{
					dynamic_map_insert(params, name);
				}
			}
		}

	}

	if(content_type && strncmp(dynamic_get_string(content_type), "application/x-www-form-urlencoded", 33) == 0)
	{
		char* str = context->post_data;
		char* name;
		char* value;
		
		str[context->post_len] = 0;

		while(str)
		{
			str = http_parse_urlencoded(str, &name, &value);
			if(name)
			{
				if(value)
				{
					dynamic_set_string(dynamic_map_insert(params, name), value);
				}else
				{
					dynamic_map_insert(params, name);
				}
			}
		}

	}
}