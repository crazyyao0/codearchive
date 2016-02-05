#include "httpserver.h"
#include "trex.h"

TRex *g_rex_reqhdr;
TRex *g_rex_httphdr;

HTTP_CONFIG g_http_config;

int http_server_load_config()
{
	strcpy(g_http_config.serverName, "my_http_server");
	strcpy(g_http_config.documentRoot, "F:\\javafx\\apache-tomcat-7.0.35\\webapps\\docs");
	strcpy(g_http_config.defaultPage, "index.html");
	return 1;
}

int http_server_init(socket_addr* addr, int addrnum)
{
	http_init();
	http_server_load_config();
	return 1;
}

int http_resolve_path(char* path, char* localpath, int locallen)
{
	char* str = localpath;
	int len;

	localpath[0];

	len = strlen(g_http_config.documentRoot);
	if(locallen < len + 1)
		return 0;
	memcpy(str, g_http_config.documentRoot, len + 1);
	str += len;
	locallen -=  len;

	if(path[0] != '/')
	{
		if(locallen < 2)
			return 0;
		*str = '/';
		str++;
		locallen --;
	}

	len = strlen(path);
	if(locallen < len + 1)
		return 0;
	memcpy(str, path, len + 1);
	str += len;
	locallen -=  len;

	if(path[len-1] == '/')
	{
		len = strlen(g_http_config.defaultPage);
		if(locallen < len + 1)
			return 0;
		memcpy(str, g_http_config.defaultPage, len + 1);
		str += len;
		locallen -=  len;
	}
	return 1;
}

int http_send_file(socket_conn* conn, dynamic_object* req)
{
	char fullpath[260];
	char filebuf[8192];
	FILE* fp;
	HTTP_MIME_MAP * mime;
	dynamic_object* reshdr;
	char* connection;
	int filelen;
	int readlen;

	if(http_resolve_path(dynamic_get_string(dynamic_map_find(req, "PATH")), fullpath, 260) == 0)
	{
		return http_send_response(conn, 414, 0);
	}

	fp = fopen(fullpath, "rb");
	if(fp == 0)
	{
		return http_send_response(conn, 404, 0);
	}
	fseek(fp, 0, SEEK_END);
	filelen = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	mime = http_find_mime(fullpath);

	reshdr = dynamic_create();
	dynamic_string_printf(dynamic_map_insert(reshdr, HTTP_HEADER_CONTENT_LENGTH), "%d", filelen);
	dynamic_set_string(dynamic_map_insert(reshdr, HTTP_HEADER_CONTENT_TYPE), mime->mime);
	dynamic_string_printf(dynamic_map_insert(reshdr, HTTP_HEADER_CACHE_CONTROL), "public, max-age=86400");
	connection = dynamic_get_string(dynamic_map_find(req, HTTP_HEADER_CONNECTION));
	if(connection && strcmp(connection, "keep-alive")==0)
	{
		dynamic_set_string(dynamic_map_insert(reshdr, HTTP_HEADER_CONNECTION), "keep-alive");
	}else
	{
		dynamic_set_string(dynamic_map_insert(reshdr, HTTP_HEADER_CONNECTION), "close");
	}



	http_send_response(conn, 200, reshdr);
	while(1)
	{
		readlen = fread(filebuf, 1, 8192, fp);
		if(readlen == 0)
			break;
		if(write_socket(conn, filebuf, readlen, -1) != readlen)
			break;
	}
	dynamic_delete(reshdr);
	fclose(fp);
	return 1;
}

int http_send_response(socket_conn* conn, int code, dynamic_object* reqhdrs)
{
	char* header = (char*)malloc(8192);
	int headlen = 0;
	int r;
	int errorlen;
	dynamic_hash_entry* entry;
	HTTP_ERROR_CODE * strcode = http_find_code(code);

	do
	{
		r = snprintf(header + headlen, 8192 - headlen, "%s\r\n", strcode->header);
		if(r<0) break;
		headlen+=r;

		if(reqhdrs)
		{
			if(dynamic_map_find(reqhdrs, HTTP_HEADER_SERVER) == 0)
			{
				r = snprintf(header + headlen, 8192 - headlen, HTTP_HEADER_SERVER": %s\r\n", g_http_config.serverName);
				if(r<0) break;
				headlen+=r;
			}

			entry = dynamic_map_first(reqhdrs);
			while(entry)
			{
				r = snprintf(header + headlen, 8192 - headlen, "%s: %s\r\n", entry->key, dynamic_get_string(&entry->value));
				if(r<0) break;
				headlen+=r;
				entry = dynamic_map_next(reqhdrs, entry);
			}
			if(r<0) break;

		}else
		{
			r = snprintf(header + headlen, 8192 - headlen, HTTP_HEADER_SERVER": %s\r\n", g_http_config.serverName);
			if(r<0) break;
			headlen+=r;
		}

		if(strcode->code >=400)
		{
			errorlen = strlen(strcode->body);
			r = snprintf(header + headlen, 8192 - headlen, HTTP_HEADER_CONTENT_TYPE": text/html\r\n");
			if(r<0) break;
			headlen+=r;
			r = snprintf(header + headlen, 8192 - headlen, HTTP_HEADER_CONTENT_LENGTH": %d\r\n", errorlen);
			if(r<0) break;
			headlen+=r;
		}

		r = snprintf(header + headlen, 8192 - headlen, "\r\n");
		if(r<0) break;
		headlen+=r;
	}while(0);

	if(r<0)
	{
		// response header is too large
		http_send_response(conn, 400, 0);
		return 0;
	}

	write_socket(conn, header, headlen, -1);
	return 0;
}

int http_read_request(socket_conn* conn, dynamic_object* reqhdrs)
{
	char* reshdr = (char*)malloc(8192);
	int r;
	TRexMatch match;

	r = readline_socket(conn, reshdr, 8191);
	if(r == 0)
	{
		free(reshdr);
		return 0;
	}
	reshdr[r] = 0;

	if(trex_search(g_rex_reqhdr, reshdr, 0, 0) == 0)
	{
		http_send_response(conn, 400, 0);
		free(reshdr);
		return 0;
	}
	trex_getsubexp(g_rex_reqhdr, 1, &match);
	dynamic_set_nstring(dynamic_map_insert(reqhdrs, "METHOD"), (char*)match.begin, match.len);

	trex_getsubexp(g_rex_reqhdr, 2, &match);
	dynamic_set_nstring(dynamic_map_insert(reqhdrs, "PATH"), (char*)match.begin, match.len);

	trex_getsubexp(g_rex_reqhdr, 3, &match);
	if(match.len > 1)
		dynamic_set_nstring(dynamic_map_insert(reqhdrs, "QUERY_STRING"), (char*)match.begin + 1, match.len - 1);
		
	trex_getsubexp(g_rex_reqhdr, 4, &match);
	if(match.len > 1)
		dynamic_set_nstring(dynamic_map_insert(reqhdrs, "FRAGMENT_ID"), (char*)match.begin + 1, match.len - 1);

	trex_getsubexp(g_rex_reqhdr, 5, &match);
	dynamic_set_nstring(dynamic_map_insert(reqhdrs, "HTTP_VERSION"), (char*)match.begin, match.len);

	do
	{
		char keystr[64];
		TRexMatch key;
		TRexMatch value;

		r = readline_socket(conn, reshdr, 8191);
		if(r == 0)
		{
			http_send_response(conn, 400, 0);
			free(reshdr);
			return 0;
		}
		reshdr[r] = 0;
		if(r == 2 && reshdr[0] == '\r' && reshdr[1] == '\n')
			break;

		if(trex_search(g_rex_httphdr, reshdr, 0, 0) == 0)
		{
			http_send_response(conn, 400, 0);
			free(reshdr);
			return 0;
		}
		trex_getsubexp(g_rex_httphdr, 1, &key);
		trex_getsubexp(g_rex_httphdr, 2, &value);
		if(key.len<64)
		{
			memcpy(keystr, key.begin, key.len);
			keystr[key.len] = 0;
			dynamic_set_nstring(dynamic_map_insert(reqhdrs, keystr), (char*)value.begin, value.len);
		}
	}while(1);

	free(reshdr);
	return 1;
}
