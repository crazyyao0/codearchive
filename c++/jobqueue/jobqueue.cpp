#include <Winsock2.h>
#include <Windows.h>
#include <stdio.h>

extern "C"
{
	#include "trex.h"
	#include "http.h"
}



class Counter
{
	long long counter;
public :
	Counter()
	{
		QueryPerformanceFrequency((LARGE_INTEGER *)&counter);
		counter/=10000;
	}

	long long Tick()
	{
		return __rdtsc() / counter;
	}
} counter;

void http_init();

void test1()
{
	const char *error = NULL;
	const char *begin,*end;
	char *exp1 = "^([^ ]+) (/[^?# ]*)(\\?[^# ]*)?(#[^ ]*)? HTTP/([^ ]+)$";
	char *exp2 = "^([^ ]+): (.+)$";
	char *exp3 = "([^=]+)(=[^&])?(&([^=]+)(=[^&]))*";


	long long t = counter.Tick();
	TRex *x = trex_compile(exp3, &error);
	printf("compile %lld\n", counter.Tick() - t);

	t = counter.Tick();
	if(trex_search(x,"aaa=bbb&ccc=ddd&eee",&begin,&end))
	{
		printf("execute %lld\n", counter.Tick() - t);
		int i,n = trex_getsubexpcount(x);
		TRexMatch match;
		for(i = 1; i < n; i++)
		{
			t = counter.Tick();
			trex_getsubexp(x,i,&match);
			printf("trex_getsubexp %lld ", counter.Tick() - t);

			for(int j=0;j<match.len;j++)
				putchar(match.begin[j]);
			printf("\n");
		}
		printf("match! %d sub matches\n",trex_getsubexpcount(x));
	}
	else
	{
		printf("execute %lld\n", counter.Tick() - t);
		printf("no match!\n");
	}
}

int __cdecl docgi(HTTP_CONTEXT* context)
{
	dynamic_object *hdr = context->req_hdr;
	char* postdata = context->post_data;

	return 200;
}

int main(int argc, char* argv[])
{
//	http_init();
//	http_register_cgi("/do", docgi);

	test1();

	while(1)
		Sleep(1000);
	return 0;
}

