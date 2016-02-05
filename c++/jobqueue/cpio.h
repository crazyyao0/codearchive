#ifndef CPIO_INCLUDED
#define CPIO_INCLUDED

#include <Winsock2.h>
#include <Mswsock.h>
#include <ws2ipdef.h>
#include <Windows.h>

typedef union SocketAddress
{
	unsigned short family;
	struct sockaddr_in addrv4;
	struct sockaddr_in6 addrv6;
}SocketAddress;

typedef struct SESSION_CONTEXT
{
	SOCKET socket;
	SocketAddress local;
	SocketAddress remote;
}SESSION_CONTEXT;

#define MAX_BUFFER_LEN 8192

typedef struct CPIO_BUFFER
{
	OVERLAPPED ov;
	int(*callback)(struct SESSION_CONTEXT*, struct CPIO_BUFFER*, DWORD bytetransfered);
	HANDLE handle;
	int len;
	char buffer[MAX_BUFFER_LEN];
}CPIO_CONTEXT;

void cpio_accept(SOCKET socket);
void cpio_receive(SOCKET socket);
void cpio_send(SOCKET socket, char* buffer, int len);

int cpio_accept_done(SESSION_CONTEXT* session, struct CPIO_BUFFER* cpio, DWORD bytetransfered);
int cpio_recv_done(SESSION_CONTEXT* session, struct CPIO_BUFFER* cpio, DWORD bytetransfered);
int cpio_send_done(SESSION_CONTEXT* session, struct CPIO_BUFFER* cpio, DWORD bytetransfered);

void cpio_init(int context_size);
void cpio_init_socket(SocketAddress* local);
void cpio_close_socket(SOCKET socket);

#endif
