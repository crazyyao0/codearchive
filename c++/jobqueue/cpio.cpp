#include "cpio.h"
#include "http.h"
#include <stdio.h>

#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "Ws2_32.lib")

HANDLE g_hcpio = 0;
BOOL g_quitcpio = FALSE;
int g_context_size = 0;

DWORD WINAPI cpio_work_thread( LPVOID lpParam )
{
	while(!g_quitcpio)
	{
		CPIO_BUFFER *cpio_buffer;
		DWORD dwBytesTransfered;
		SESSION_CONTEXT *session;
		if(GetQueuedCompletionStatus(g_hcpio, &dwBytesTransfered, (PULONG_PTR)&session, (LPOVERLAPPED*)&cpio_buffer, 1000))
		{
			cpio_buffer->callback(session, cpio_buffer, dwBytesTransfered);
			free(cpio_buffer);
		}
	}
	return 0;
}

int cpio_accept_done(SESSION_CONTEXT* session, struct CPIO_BUFFER* cpio, DWORD bytetransfered)
{
	sockaddr* localaddr;
	sockaddr* remoteaddr;
	int localaddrlen;
	int remoteaddrlen;
	printf("[%08X]cpio_accept\n", session->socket);
	GetAcceptExSockaddrs(cpio->buffer, cpio->len - 2 * (sizeof(sockaddr_in6) + 16), 
		sizeof(sockaddr_in6) + 16, sizeof(sockaddr_in6) + 16, &localaddr, &localaddrlen, &remoteaddr, &remoteaddrlen);
	
	SESSION_CONTEXT *new_session = (SESSION_CONTEXT*)malloc(g_context_size);
	new_session->socket = (SOCKET)cpio->handle;
	memcpy(&new_session->local, localaddr, localaddrlen);
	memcpy(&new_session->remote, remoteaddr, remoteaddrlen);

	cpio_accept(session->socket);

	if(CreateIoCompletionPort( cpio->handle, g_hcpio, (ULONG_PTR)new_session, 0) == NULL)
	{
		closesocket((SOCKET)cpio->handle);
		free(new_session);
	}else
	{
		printf("[%08X]new_session and received %d bytes\n", new_session->socket, bytetransfered);
		cpio_receive(new_session->socket);
		http_init_context((HTTP_CONTEXT*)new_session);
		if(bytetransfered)
			http_data_arrived((HTTP_CONTEXT*)new_session, cpio->buffer, bytetransfered);
	}
	return 0;
}

void cpio_accept(SOCKET socket)
{
	// start async accept
	SOCKET s = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if(s == INVALID_SOCKET)
		return;

	DWORD dwBytes;
	CPIO_BUFFER *cpio_ctx = (CPIO_BUFFER*)malloc(sizeof(CPIO_BUFFER));
	cpio_ctx->len = sizeof(cpio_ctx->buffer);
	cpio_ctx->handle = (HANDLE)s;
	cpio_ctx->callback = cpio_accept_done;
	memset(&cpio_ctx->ov, 0, sizeof(OVERLAPPED));

	AcceptEx(socket, s, cpio_ctx->buffer, cpio_ctx->len - 2 * (sizeof(sockaddr_in6) + 16), 
		sizeof(sockaddr_in6) + 16, sizeof(sockaddr_in6) + 16, &dwBytes, &cpio_ctx->ov);
}

int cpio_recv_done(SESSION_CONTEXT* session, struct CPIO_BUFFER* cpio, DWORD bytetransfered)
{
	if(bytetransfered == 0)
	{
		printf("[%08X]session closed by remote\n", session->socket);
		shutdown(session->socket, SD_BOTH);
		closesocket(session->socket);
		http_close_context((HTTP_CONTEXT*)session);
		free(session);
		return 0;
	}else
	{
		printf("[%08X]cpio_receive %d bytes\n", session->socket, bytetransfered);
		cpio_receive(session->socket);
		http_data_arrived((HTTP_CONTEXT*)session, cpio->buffer, bytetransfered);
	}
	return 0;
}

void cpio_receive(SOCKET socket)
{
	DWORD dwFlags = 0;
	DWORD dwBytes = 0;
	CPIO_BUFFER *cpio_ctx = (CPIO_BUFFER*)malloc(sizeof(CPIO_BUFFER));
	cpio_ctx->len = sizeof(cpio_ctx->buffer);
	cpio_ctx->handle = (HANDLE)socket;
	cpio_ctx->callback = cpio_recv_done;
	memset(&cpio_ctx->ov, 0, sizeof(OVERLAPPED));

	WSABUF buf;
	buf.buf = cpio_ctx->buffer;
	buf.len = cpio_ctx->len;

	WSARecv(socket, &buf, 1, &dwBytes, &dwFlags, &cpio_ctx->ov, NULL);
}

int cpio_send_done(SESSION_CONTEXT* session, struct CPIO_BUFFER* cpio, DWORD bytetransfered)
{
	if(bytetransfered == 0)
	{
		printf("[%08X]0 bytes sent\n", session->socket);
		return 0;
	}else
	{
		printf("[%08X]cpio_send %d bytes\n", session->socket, bytetransfered);
		http_data_sent((HTTP_CONTEXT*)session, bytetransfered);
	}
	return 0;
}

void cpio_send(SOCKET socket, char* buffer, int len)
{
	DWORD dwFlags = 0;
	DWORD dwBytes = 0;
	CPIO_BUFFER *cpio_ctx = (CPIO_BUFFER*)malloc(sizeof(CPIO_BUFFER) - MAX_BUFFER_LEN);
	cpio_ctx->len = len;
	cpio_ctx->handle = (HANDLE)socket;
	cpio_ctx->callback = cpio_send_done;
	memset(&cpio_ctx->ov, 0, sizeof(OVERLAPPED));

	WSABUF buf;
	buf.buf = buffer;
	buf.len = len;
	WSASend(socket, &buf, 1, &dwBytes, dwFlags, &cpio_ctx->ov, NULL);
}

void cpio_close_socket(SOCKET socket)
{
	printf("[%08X]session closed by server\n", socket);
	shutdown(socket, SD_BOTH);
}

void cpio_init_socket(SocketAddress* local)
{
	if(local->family == AF_INET)
	{
		SOCKET socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
		if(socket != INVALID_SOCKET) do
		{
			if(bind(socket, (struct sockaddr *)&local->addrv4, sizeof(struct sockaddr_in)) == SOCKET_ERROR)
			{
				closesocket(socket);
				socket = INVALID_SOCKET;
				break;
			}

			if(listen(socket, SOMAXCONN) == SOCKET_ERROR)
			{
				closesocket(socket);
				socket = INVALID_SOCKET;
				break;
			}

			SESSION_CONTEXT * context = (SESSION_CONTEXT*)malloc(g_context_size);
			context->socket = socket;
			if(CreateIoCompletionPort( (HANDLE)socket, g_hcpio, (ULONG_PTR)context, 0) == NULL)
			{
				closesocket(socket);
				socket = INVALID_SOCKET;
				free(context);
				break;
			}
			cpio_accept(context->socket);
		}while(0);
	}else if(local->family == AF_INET6)
	{
		SOCKET socket = WSASocket(AF_INET6, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
		if(socket != INVALID_SOCKET) do
		{
			if(bind(socket, (struct sockaddr *)&local->addrv6, sizeof(struct sockaddr_in6)) == SOCKET_ERROR)
			{
				closesocket(socket);
				socket = INVALID_SOCKET;
				break;
			}

			if(listen(socket, SOMAXCONN) == SOCKET_ERROR)
			{
				closesocket(socket);
				socket = INVALID_SOCKET;
				break;
			}

			SESSION_CONTEXT * context = (SESSION_CONTEXT*)malloc(g_context_size);
			context->socket = socket;
			if(CreateIoCompletionPort( (HANDLE)socket, g_hcpio, 0, 0) == NULL)
			{
				closesocket(socket);
				socket = INVALID_SOCKET;
				free(context);
				break;
			}
			cpio_accept(context->socket);
		}while(0);
	}
}

void cpio_init(int context_size)
{
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2,2), &wsaData);
	
	g_context_size = context_size;

	g_hcpio = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if(g_hcpio == NULL)
	{
		return;
	}

	SYSTEM_INFO si;
    GetSystemInfo(&si);
	for (DWORD i = 0; i < si.dwNumberOfProcessors; i++)
	{
		CreateThread(0, 0, cpio_work_thread, 0, 0, 0);
	}
}