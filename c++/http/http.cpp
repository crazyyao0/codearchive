extern "C"
{
	#include "socket.h"
	#include "httpclient.h"
	#include "httpserver.h"
}

DWORD WINAPI http_do_request(void *lPtr)
{
	socket_conn* conn = (socket_conn*)lPtr;
	if(conn == 0)
		return 0;

	printf("[%08x]new connection\n", GetCurrentThreadId());
	while(1)
	{
		int keepalive = 0;
		dynamic_object * req = dynamic_create();
		if(http_read_request(conn, req) == 0)
		{
			dynamic_delete(req);
			break;
		}
		char* path = dynamic_get_string(dynamic_map_find(req, "PATH"));
		char* method = dynamic_get_string(dynamic_map_find(req, "METHOD"));
		char* connection = dynamic_get_string(dynamic_map_find(req, HTTP_HEADER_CONNECTION));
		if(connection && strcmp(connection, "keep-alive")==0)
			keepalive = 1;
		printf("[%08x]%s %s\n", GetCurrentThreadId(), method, path);

		if(strcmp(path, "/hello") == 0)
		{
			char* html = "<html><body>Hello World!</body></html>";
			int htmllen = strlen(html);
			dynamic_object * res = dynamic_create();
			dynamic_string_printf(dynamic_map_insert(res, HTTP_HEADER_CONTENT_LENGTH), "%d", htmllen);
			dynamic_set_string(dynamic_map_insert(res, HTTP_HEADER_CONTENT_TYPE), "text/html");

			http_send_response(conn, 200, res);
			write_socket(conn, html, htmllen, -1);
			dynamic_delete(res);
		}else
		{
			http_send_file(conn, req);
		}
		dynamic_delete(req);
		if(keepalive == 0)
			break;
	}

	close_socket(conn);
	printf("[%08x]close connection\n", GetCurrentThreadId());
	return 0;
}

extern "C"
{
	#include "list.h"

	typedef struct UserSpaceThread
	{
		DWORD* esp;
		DWORD state;
		list_head list;
		list_head active;
		char stack[1];
	}UserSpaceThread;

	typedef int(*USTFUNC)(void* param);
	void __fastcall SwitchUST(void* from, void* to);
	DWORD USTValue;

	void USTMain(USTFUNC func, void* param)
	{
		func(param);
		ExitThread(1);
	}

	UserSpaceThread* CreateUst(USTFUNC func, void* param)
	{
		UserSpaceThread* thread = (UserSpaceThread*)malloc(65536);
		thread->esp = (DWORD*)(((unsigned char*)thread) + 65500);
		thread->state = 0;
		INIT_LIST_HEAD(&thread->list);
		INIT_LIST_HEAD(&thread->active);
		thread->esp[0] = 0;	//edi
		thread->esp[1] = 0;	//esi
		thread->esp[2] = 0; //ebx
		thread->esp[3] = 0; //ebp
		thread->esp[4] = (DWORD)USTMain; //epc
		thread->esp[5] = 0; //epc
		thread->esp[6] = (DWORD)func; //func entry
		thread->esp[7] = (DWORD)param; //param for USTMain

		return thread;
	}

	void InitUST(UserSpaceThread* thread)
	{
		DWORD sp;
		USTValue = TlsAlloc();
		TlsSetValue(USTValue, &sp);
		SwitchUST(&sp, thread);
	}

	void USTDoSchedule(UserSpaceThread* dst)
	{
		UserSpaceThread* src = (UserSpaceThread*)TlsGetValue(USTValue);
		TlsSetValue(USTValue, dst);
		SwitchUST(src, dst);
	}

	void USTScheduleNext()
	{

	}
}

UserSpaceThread* t1;
UserSpaceThread* t2;

int _cdecl test2(void*param)
{
	for(int i=0;i<100;i++)
	{
		printf("thread2 [%08x] i=%d\n",GetCurrentThreadId(), i);
		TlsSetValue(USTValue, t1);
		SwitchUST(t2, t1);
		Sleep(1000);
	}
	return 1;
}

int _cdecl test1(void*param)
{
	for(int i=0;i<100;i++)
	{
		printf("thread1 [%08x] i=%d\n",GetCurrentThreadId(), i);
		TlsSetValue(USTValue, t2);
		SwitchUST(t1, t2);
		Sleep(1000);
	}
	return 1;
}

int main(int argc, char* argv[])
{
	socket_addr addr;

	t1 = CreateUst(test1, 0);
	t2 = CreateUst(test2, 0);
	InitUST(t1);

/*
	if(http_server_init(&addr, 1) == 0)
		return 0;

	memset(&addr, 0, sizeof(addr));
	addr.addrv4.sin_family = AF_INET;
	addr.addrv4.sin_port = htons(8008);
	addr.addrv4.sin_addr.s_addr = INADDR_ANY;
	if(listen_server_socket(&addr, "ca.crt", "ca.key")==0)
		return 0;

	memset(&addr, 0, sizeof(addr));
	addr.addrv4.sin_family = AF_INET;
	addr.addrv4.sin_port = htons(8007);
	addr.addrv4.sin_addr.s_addr = INADDR_ANY;
	if(listen_server_socket(&addr, 0, 0)==0)
		return 0;


	while(1)
	{
		socket_conn* conn = accept_socket();
		if(conn == 0)
			return 0;

		HANDLE h = CreateThread(NULL, 0, http_do_request, conn, 0, NULL);
		if(h == 0)
		{
			http_send_response(conn, 500, 0);
			close_socket(conn);
		}else
		{
			CloseHandle(h);
		}
	}
	*/
	return 1;
}
