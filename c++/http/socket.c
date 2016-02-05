#include "socket.h"
#include <openssl/err.h>

SSL_CTX* g_ssl_ctx = 0;

void init_socket()
{
	static int g_socket_init = 0;
	if(g_socket_init == 0)
	{
#ifdef _WIN32
		WSADATA wsaData;
		WSAStartup(MAKEWORD(2,2), &wsaData);
#endif
		SSL_library_init();
		OpenSSL_add_all_algorithms();
		SSL_load_error_strings();
		g_ssl_ctx = SSL_CTX_new(TLSv1_client_method());

		g_socket_init = 1;
	}
}

socket_conn* connect_socket(socket_addr* addr, int ssl)
{
	SOCKET s;
	SSL *ss = 0;
	socket_conn* conn;

	if(addr->family == AF_INET)
	{
		s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if(s == INVALID_SOCKET)
			return 0;

		if(connect(s, (struct sockaddr*)addr, sizeof(addr->addrv4))!=0)
		{
			closesocket(s);
			return 0;
		}
	}else if(addr->family == AF_INET6)
	{
		s = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
		if(s == INVALID_SOCKET)
			return 0;

		if(connect(s, (struct sockaddr*)addr, sizeof(addr->addrv6))!=0)
		{
			closesocket(s);
			return 0;
		}
	}else
		return 0;

	if(ssl)
	{
		ss = SSL_new(g_ssl_ctx);
		if(ss == 0)
		{
			closesocket(s);
			return 0;
		}

		if(SSL_set_fd(ss, s) == 0)
		{
			SSL_free(ss);
			closesocket(s);
			return 0;
		}
		if(SSL_connect(ss)!=1)
		{
			SSL_free(ss);
			closesocket(s);
			return 0;
		}
	}

	conn = (socket_conn*)malloc(sizeof(socket_conn));
	memset(conn, 0, sizeof(socket_conn));
	conn->socket = s;
	conn->sslHandle = ss;
	conn->remote = *addr;
	return conn;
}

void close_socket(socket_conn* conn)
{
	if(conn)
	{
		if (conn->sslHandle)
		{
			SSL_shutdown (conn->sslHandle);
			SSL_free (conn->sslHandle);
		}

		if (conn->socket)
		{
			shutdown(conn->socket, 2);
			closesocket (conn->socket);
		}

		free(conn);
	}
}

int do_ssl_handshake(socket_conn* conn)
{
	if(conn->sslContext && conn->sslHandle == 0)
	{
		conn->sslHandle = SSL_new(conn->sslContext);
		if(conn->sslHandle == 0)
		{
			return 0;
		}
		if(SSL_set_fd(conn->sslHandle, conn->socket) == 0)
		{
			return 0;
		}
		if(SSL_accept(conn->sslHandle)!=1)
		{
			printf("%s\n", ERR_error_string(ERR_get_error(), 0));
			return 0;
		}
	}
	return 1;
}


int read_socket(socket_conn* conn, char* buffer, int len, int timeout)
{
	if(conn==0 || do_ssl_handshake(conn)==0)
		return 0;

	if(conn->cache)
	{
		int size = conn->cacheend - conn->cachepos;
		if(size == 0)
		{
			conn->cachepos = 0;
			conn->cacheend = 0;
			free(conn->cache);
			conn->cache = 0;
		}else if(size <= len)
		{
			memcpy(buffer, conn->cache + conn->cachepos, size);
			conn->cachepos = 0;
			conn->cacheend = 0;
			free(conn->cache);
			conn->cache = 0;
			return size;
		}else
		{
			memcpy(buffer, conn->cache + conn->cachepos, len);
			conn->cachepos += size;
			return len;
		}
	}
	
	if(conn->sslHandle)
	{
		return SSL_read (conn->sslHandle, buffer, len);
	}else
	{
		return recv(conn->socket, buffer, len, 0);
	}
}

int readline_socket(socket_conn* conn, char* buffer, int len)
{
	int copied = 0;
	int remain = len;

	if(conn==0 || do_ssl_handshake(conn)==0)
		return 0;

	if(conn->cache == 0)
		conn->cache = (char*)malloc(8192);

	while(1)
	{
		char* cur;
		char* end;
		if(conn->cachepos == conn->cacheend)
		{
			if(conn->sslHandle)
			{
				conn->cacheend = SSL_read(conn->sslHandle, conn->cache, 8192);
			}else
			{
				conn->cacheend = recv(conn->socket, conn->cache, 8192, 0);
			}
			if(conn->cacheend <= 0)
				return 0;
			conn->cachepos = 0;
		}

		cur = conn->cache + conn->cachepos;
		end = conn->cache + conn->cacheend;
		while(cur < end)
		{
			if(*cur == '\n')
				break;
			cur++;
		}

		if(cur < end)
		{
			int tocopy = cur - conn->cache - conn->cachepos + 1;
			if(tocopy>remain)
				return 0;

			memcpy(buffer + copied, conn->cache + conn->cachepos, tocopy);
			conn->cachepos += tocopy;
			copied += tocopy;
			return copied;
		}else
		{
			int tocopy = conn->cacheend - conn->cachepos;
			if(tocopy>remain)
				return 0;

			memcpy(buffer + copied, conn->cache + conn->cachepos, tocopy);
			conn->cachepos += tocopy;
			copied += tocopy;
			remain -= tocopy;
		}
	}
}

int write_socket(socket_conn* conn, char* buffer, int len, int timeout)
{
	int count = 0;
	if(conn==0 || do_ssl_handshake(conn)==0)
		return 0;

	if(conn->sslHandle)
	{
		while(count < len)
		{
			int r = SSL_write (conn->sslHandle, buffer, len);
			if(r<0)
				break;
			count += r;
		}
	}else
	{
		while(count < len)
		{
			int r = send(conn->socket, buffer, len, 0);
			if(r<0)
				break;
			count += r;
		}
	}
	return count;
}

int get_socket_addr(socket_addr* addr, char* host, int port)
{
	struct addrinfo* info = 0;
	int err;

	memset(addr, 0, sizeof(socket_addr));
	err = getaddrinfo(host, 0, 0, &info);
	if(err==0)
	{
		memcpy(addr, info->ai_addr, info->ai_addrlen);
		freeaddrinfo(info);
		addr->addrv6.sin6_port = htons(port);
		return 1;
	}
	return 0;
}

socket_conn g_server_socket[10];
int g_server_socket_size = 0;

int listen_server_socket(socket_addr* addr, char* crt, char* key)
{
	SOCKET s;
	SSL_CTX* ctx = 0;

	if(g_server_socket_size == 10)
		return 0;

	if(addr->family == AF_INET)
	{
		s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if(s == INVALID_SOCKET)
			return 0;

		if(bind(s, (struct sockaddr*)addr, sizeof(addr->addrv4))!=0)
		{
			closesocket(s);
			return 0;
		}
	}else if(addr->family == AF_INET6)
	{
		s = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
		if(s == INVALID_SOCKET)
			return 0;

		if(bind(s, (struct sockaddr*)addr, sizeof(addr->addrv6))!=0)
		{
			closesocket(s);
			return 0;
		}
	}else
		return 0;

	if(listen(s, SOMAXCONN) != 0)
	{
		closesocket(s);
		return 0;
	}

	if(crt && key)
	{
		ctx = SSL_CTX_new(SSLv23_server_method());
		SSL_CTX_use_certificate_file(ctx, crt, SSL_FILETYPE_PEM);
		SSL_CTX_use_PrivateKey_file(ctx, key, SSL_FILETYPE_PEM);
		if ( !SSL_CTX_check_private_key(ctx) )
		{
			SSL_CTX_free(ctx);
			closesocket(s);
			return 0;
		}
	}

	memset(&g_server_socket[g_server_socket_size], 0, sizeof(socket_conn));
	g_server_socket[g_server_socket_size].socket = s;
	g_server_socket[g_server_socket_size].local = *addr;
	g_server_socket[g_server_socket_size].sslContext = ctx;
	g_server_socket_size++;
	return 1;
}

socket_conn* accept_socket()
{
	int i;
	int ret;
	int max_sock = 0;
	fd_set set;

	FD_ZERO(&set);
	for(i = 0; i<g_server_socket_size; i++)
	{
		if(max_sock < g_server_socket[i].socket)
			max_sock = g_server_socket[i].socket;
		FD_SET(g_server_socket[i].socket, &set);
	}

	if ((ret = select(max_sock + 1, &set, NULL, NULL, 0)) == SOCKET_ERROR)
	{
		return 0;
	}

	for(i = 0; i<g_server_socket_size; i++)
	{
		if(FD_ISSET(g_server_socket[i].socket, &set))
		{
			SOCKET s_conn;
			socket_conn * conn;
			SSL* ss = 0;
			socket_addr addr;
			socklen_t addr_len = sizeof(addr);
			memset(&addr, 0, sizeof(addr));

			s_conn = accept(g_server_socket[i].socket, (struct sockaddr*)&addr, &addr_len);
			if(s_conn == INVALID_SOCKET)
			{
				return 0;
			}

			conn = (socket_conn*)malloc(sizeof(socket_conn));
			memset(conn, 0, sizeof(socket_conn));
			conn->socket = s_conn;
			conn->remote = addr;
			conn->local = g_server_socket[i].local;
			conn->sslContext = g_server_socket[i].sslContext;
			return conn;
		}
	}
	return 0;
}
