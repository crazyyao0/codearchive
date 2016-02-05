#ifndef SOCKET_INCLUDED
#define SOCKET_INCLUDED

#ifdef _WIN32
#include <Winsock2.h>
#include <Mswsock.h>
#include <ws2ipdef.h>
#include <Ws2tcpip.h>
#include <Windows.h>
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "Ws2_32.lib")
#define snprintf _snprintf
#define atoll _atoi64 
#define inet_pton InetPton
#define inet_ntop InetNtop
#define strcasecmp stricmp
#define strncasecmp _strnicmp
typedef int socklen_t;
#endif

#ifdef __GNUC__
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
typedef int SOCKET;
#ifndef SOCKET_ERROR
#define SOCKET_ERROR -1
#endif
#ifndef INVALID_SOCKET
#define INVALID_SOCKET -1
#endif
#define closesocket(fd) close(fd)
#endif



#include "openssl/ssl.h"

#ifdef _WIN32
#pragma comment( lib, "libeay32.lib" )
#pragma comment( lib, "ssleay32.lib" )
#endif

typedef union socket_addr
{
	unsigned short family;
	struct sockaddr_in addrv4;
	struct sockaddr_in6 addrv6;
}socket_addr;

typedef struct socket_conn
{
    int socket;
	socket_addr local;
	socket_addr remote;
    SSL* sslHandle;
	SSL_CTX* sslContext;

	char* cache;
	int cachepos;
	int cacheend;
} socket_conn;

SSL_CTX* init_ssl();
void init_socket();

int get_socket_addr(socket_addr* addr, char* host, int port);
socket_conn* connect_socket(socket_addr* addr, int ssl);
void close_socket(socket_conn* conn);
int read_socket(socket_conn* conn, char* buffer, int len, int timeout);
int readline_socket(socket_conn* conn, char* buffer, int len);
int write_socket(socket_conn* conn, char* buffer, int len, int timeout);
int listen_server_socket(socket_addr* addr, char* crt, char* key);
socket_conn* accept_socket();
#endif
