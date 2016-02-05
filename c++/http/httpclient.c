#include "httpclient.h"
#include "trex.h"

#define HTTP_COMPRESS_ENCODING_GZIP 1
#define HTTP_COMPRESS_ENCODING_DEFLATE 2

socket_conn* http_send_request(char* method, char* url, dynamic_object* headers)
{
	char* header;
	char service[32];
	char host[128];
	char port[32];
	char path[2048];

	int portnum = 80;
	socket_addr addr;
	TRexMatch match;
	int headlen;
	int sent;
	int ishttps = 0;
	socket_conn* conn;

	if(trex_search(g_rex_url, url, 0, 0) == 0)
		return 0;

	service[0] = 0;
	if(trex_getsubexp(g_rex_url, 1, &match) && match.len > 0 && match.len < 32)
	{
		memcpy(service, match.begin, match.len - 3);
		service[match.len - 3] = 0;
	}

	if(trex_getsubexp(g_rex_url, 2, &match) && match.len > 0 && match.len < 128)
	{
		memcpy(host, match.begin, match.len);
		host[match.len] = 0;
	}
	else
		return 0;

	port[0] = 0;
	if(trex_getsubexp(g_rex_url, 3, &match) && match.len > 0 && match.len < 32)
	{
		memcpy(port, match.begin + 1, match.len - 1);
		port[match.len - 1] = 0;
	}

	if(trex_getsubexp(g_rex_url, 4, &match) && match.len > 0 && match.len < 2048)
	{
		memcpy(path, match.begin, match.len);
		path[match.len] = 0;
	}
	else
	{
		memcpy(path, "/", 2);
	}

	if(service[0] && strncasecmp(service, "https", 5) == 0)
	{
		portnum = 443;
		ishttps = 1;
	}else
	{
		portnum = 80;
	}
	if(port[0])
	{
		portnum = atol(port);
	}

	if(get_socket_addr(&addr, host, portnum) == 0)
		return 0;

	header = (char*)malloc(8192);
	headlen = 0;
	headlen += snprintf(header + headlen, 8192 - headlen, "%s %s HTTP/1.1\r\n", method, path);
	headlen += snprintf(header + headlen, 8192 - headlen, HTTP_HEADER_HOST": %s\r\n", host);

	if(headers)
	{
		dynamic_hash_entry* entry = dynamic_map_first(headers);
		while(entry)
		{
			headlen += snprintf(header + headlen, 8192 - headlen, "%s: %s\r\n", entry->key, dynamic_get_string(&entry->value));
			entry = dynamic_map_next(headers, entry);
		}

		if(dynamic_map_find(headers, HTTP_HEADER_ACCEPT) == 0)
			headlen += snprintf(header + headlen, 8192 - headlen, HTTP_HEADER_ACCEPT": */*\r\n");
		if(dynamic_map_find(headers, HTTP_HEADER_ACCEPT_ENCODING) == 0)
			headlen += snprintf(header + headlen, 8192 - headlen, HTTP_HEADER_ACCEPT_ENCODING": gzip,deflate\r\n");
		if(dynamic_map_find(headers, HTTP_HEADER_USER_AGENT) == 0)
			headlen += snprintf(header + headlen, 8192 - headlen, HTTP_HEADER_USER_AGENT": myAgent\r\n");
		if(dynamic_map_find(headers, HTTP_HEADER_CONNECTION) == 0)
			headlen += snprintf(header + headlen, 8192 - headlen, HTTP_HEADER_CONNECTION": close\r\n");
	}else
	{
		headlen += snprintf(header + headlen, 8192 - headlen, HTTP_HEADER_ACCEPT": */*\r\n");
		headlen += snprintf(header + headlen, 8192 - headlen, HTTP_HEADER_ACCEPT_ENCODING": gzip,deflate\r\n");
		headlen += snprintf(header + headlen, 8192 - headlen, HTTP_HEADER_USER_AGENT": myAgent\r\n");
		headlen += snprintf(header + headlen, 8192 - headlen, HTTP_HEADER_CONNECTION": close\r\n");
	}

	headlen += snprintf(header + headlen, 8192 - headlen, "\r\n");
	if(headlen >= 8192)
	{
		free(header);
		return 0;
	}
	conn = connect_socket(&addr, ishttps);
	if(conn == 0)
	{
		free(header);
		return 0;
	}

	sent = 0;
	while(sent != headlen)
	{
		int r = write_socket(conn, header, headlen, -1);
		if(r == 0)
		{
			free(header);
			close_socket(conn);
			return 0;
		}
		sent += r;
	}

	free(header);
	return conn;
}

http_stream* http_get_response_header(socket_conn* conn, dynamic_object* headers)
{
	char* reshdr;
	int r;
	const char* a;
	const char* b;

	TRexMatch match;
	dynamic_object* content_length;
	dynamic_object* content_encoding;
	dynamic_object* transfer_encoding;
	http_stream* stream;

	if(conn == 0)
		return 0;

	reshdr = (char*)malloc(4096);
	r = readline_socket(conn, reshdr, 4095);
	if(r == 0)
	{
		free(reshdr);
		return 0;
	}
	reshdr[r] = 0;

	if(trex_search(g_rex_reshdr, reshdr, &a, &b) == 0)
	{
		free(reshdr);
		return 0;
	}

	trex_getsubexp(g_rex_reshdr, 1, &match);
	dynamic_set_nstring(dynamic_map_insert(headers, "code"), (char*)match.begin, match.len);

	trex_getsubexp(g_rex_reshdr, 2, &match);
	dynamic_set_nstring(dynamic_map_insert(headers, "code_string"), (char*)match.begin, match.len);

	do
	{
		char keystr[64];
		TRexMatch key;
		TRexMatch value;

		r = readline_socket(conn, reshdr, 8191);
		if(r == 0)
		{
			free(reshdr);
			return 0;
		}
		reshdr[r] = 0;
		if(r == 2 && reshdr[0] == '\r' && reshdr[1] == '\n')
			break;

		if(trex_search(g_rex_httphdr, reshdr, 0, 0) == 0)
		{
			free(reshdr);
			return 0;
		}
		trex_getsubexp(g_rex_httphdr, 1, &key);
		trex_getsubexp(g_rex_httphdr, 2, &value);
		if(key.len<64)
		{
			memcpy(keystr, key.begin, key.len);
			keystr[key.len] = 0;
			dynamic_set_nstring(dynamic_map_insert(headers, keystr), (char*)value.begin, value.len);
		}
	}while(1);

	stream = (http_stream*)malloc(sizeof(http_stream));
	memset(stream, 0, sizeof(http_stream));
	stream->socket = conn;
	free(reshdr);

	content_length = dynamic_map_find(headers, HTTP_HEADER_CONTENT_LENGTH);
	if(content_length)
	{
		stream->content_length = atol(dynamic_get_string(content_length));
	}

	content_encoding = dynamic_map_find(headers, HTTP_HEADER_CONTENT_ENCODING);
	if(content_encoding)
	{
		char* method = dynamic_get_string(content_encoding);
		if(strcmp(method, "gzip") == 0)
		{
			stream->compress_encoding = HTTP_COMPRESS_ENCODING_GZIP;
		}else if(strcmp(method, "deflate") == 0)
		{
			stream->compress_encoding = HTTP_COMPRESS_ENCODING_DEFLATE;
		}else
		{
			stream->compress_encoding = 0;
		}
		stream->content_length = 0;
	}

	transfer_encoding = dynamic_map_find(headers, HTTP_HEADER_TRANSFER_ENCODING);
	if(transfer_encoding && strcmp(dynamic_get_string(transfer_encoding), "chunked")==0)
	{
		stream->chunk_encoding = 1;
		stream->content_length = 0;
	}

	return stream;
}

int http_stream_read(http_stream* stream, char* buffer, int len)
{
	int size;
	int r;

	if(stream->chunk_encoding == 0 && stream->compress_encoding == 0)
	{
		return read_socket(stream->socket, buffer, len, -1);
	}

	if(stream->raw == 0)
	{
		if(stream->zstream && stream->zstream->avail_in > 0)
		{
			stream->rawp = 0;
			stream->raw = (char*)malloc(65536);
			stream->zstream->next_out = (unsigned char*)stream->raw;
			stream->zstream->avail_out = 65536;
			r = inflate(stream->zstream, Z_NO_FLUSH);
			stream->rawmax = 65536 - stream->zstream->avail_out;
			if(r != Z_OK && r != Z_STREAM_END)
			{
				return 0;
			}
		}else
		{
			if(stream->chunk_encoding)
			{
				char linebuf[16];
				int read = 0;
				r = readline_socket(stream->socket, linebuf, 15);
				linebuf[r] = 0;
				stream->rawmax = atol(linebuf);
				if(stream->rawmax == 0)
					return 0;

				stream->rawp = 0;
				stream->raw = (char*)malloc(stream->rawmax);
				while(read < stream->rawmax)
				{
					r = read_socket(stream->socket, stream->raw + read, stream->rawmax - read, -1);
					if(r == 0)
						return 0;
					read += r;
				}
			}else
			{
				stream->rawp = 0;
				stream->raw = (char*)malloc(65536);
				stream->rawmax = read_socket(stream->socket, stream->raw, 65536, -1);
				if(stream->rawmax == 0)
					return 0;
			}

			if(stream->compress_encoding)
			{
				if(stream->compress_in)
					free(stream->compress_in);
				stream->compress_in = stream->raw;

				if(stream->zstream == 0)
				{
					stream->zstream = (z_stream*)malloc(sizeof(z_stream));
					memset(stream->zstream, 0, sizeof(z_stream));
					if(stream->compress_encoding == HTTP_COMPRESS_ENCODING_GZIP)
					{
						int flag = (unsigned char)stream->compress_in[3];
						int skip = 10;
						unsigned char *p = (unsigned char*)stream->compress_in + 10;

						if(flag & 4)	// FLG.FEXTRA
						{
							unsigned short len = (unsigned short)p[0] * 256 + p[1];
							p += len + 2;
							skip = len + 2;
						}
						if(flag & 8)	//FLG.FNAME
						{
							while(*p)
							{
								p ++;
								skip ++;
							}
						}
						if(flag & 16)	//FLG.FCOMMENT
						{
							while(*p)
							{
								p ++;
								skip ++;
							}
						}
						if(flag & 2)	// FLG.FCRC
						{
							p += 2;
							skip += 10;
						}

						inflateInit2(stream->zstream, -MAX_WBITS);
						stream->zstream->next_in = (unsigned char*)p;
						stream->zstream->avail_in = stream->rawmax - skip;
					}else	//deflate format
					{
						inflateInit(stream->zstream);
						stream->zstream->next_in = (unsigned char*)stream->compress_in;
						stream->compress_len = stream->rawmax;
						stream->zstream->avail_in = stream->rawmax;
					}
				}else
				{
					stream->zstream->next_in = (unsigned char*)stream->compress_in;
					stream->zstream->avail_in = stream->rawmax;
				}
			
				stream->rawp = 0;
				stream->raw = (char*)malloc(65536);
				stream->zstream->next_out = (unsigned char*)stream->raw;
				stream->zstream->avail_out = 65536;
				r = inflate(stream->zstream, Z_NO_FLUSH);
				stream->rawmax = 65536 - stream->zstream->avail_out;

				if(r == Z_OK)
				{

				}else if(r == Z_STREAM_END)
				{
					inflateEnd(stream->zstream);
					free(stream->zstream);
					stream->zstream = 0;
				}else
				{
					inflateEnd(stream->zstream);
					free(stream->zstream);
					stream->zstream = 0;
					return 0;
				}
			}
		}
	}

	size = stream->rawmax - stream->rawp;
	if(len<size)
	{
		memcpy(buffer, stream->raw + stream->rawp, len);
		stream->rawp += len;
		return len;
	}else
	{
		memcpy(buffer, stream->raw + stream->rawp, size);
		stream->rawp += size;
		free(stream->raw);
		stream->raw = 0;
		return size;
	}
}

void http_close_stream(http_stream* stream)
{
	if(stream->raw)
		free(stream->raw);
	if(stream->compress_in)
		free(stream->compress_in);
	if(stream->zstream)
	{
		inflateEnd(stream->zstream);
		free(stream->zstream);
	}

	close_socket(stream->socket);
	free(stream);
}

int http_simple_get(char* url, dynamic_object* reqhers, dynamic_object* reshdrs, char** out)
{
	socket_conn* conn;
	http_stream* stream;
	char* buf;
	int maxlen;
	int buflen;

	*out = 0;
	conn = http_send_request("GET", url, reqhers);
	if(conn == 0)
		return 0;

	stream = http_get_response_header(conn, reshdrs);
	if(stream == 0)
	{
		close_socket(conn);
		dynamic_delete(reshdrs);
		return 0;
	}

	if(stream->content_length)
	{
		buf = (char*)malloc(stream->content_length + 2);
		maxlen = stream->content_length + 1;
		buflen = 0;
	}else
	{
		buf = (char*)malloc(8192);
		maxlen = 8191;
		buflen = 0;
	}

	while(1)
	{
		int r;
		if(buflen == maxlen)
		{
			char* newbuf = (char*)malloc(maxlen * 2 + 1);
			memcpy(newbuf, buf, maxlen);
			maxlen *= 2;
			free(buf);
			buf = newbuf;
		}
		r = http_stream_read(stream, buf + buflen, maxlen - buflen);
		if(r == 0)
			break;
		buflen += r;
	}

	buf[buflen] = 0;
	*out= buf;
	http_close_stream(stream);
	return buflen;
}