#include <malloc.h>
#include <memory.h>

void* bm_compile(unsigned char* pat, int patlen);
int bm_search(unsigned char* str, int len, void* ctx);
void* kmp_compile(unsigned char* pat, int patlen);
int kmp_search(unsigned char* str, int len, void* ctx);

struct strsearch_context_t
{
	int (*searchop)(unsigned char*, int, void*);
	unsigned char* pat;
	int patlen;
};

static inline void* str_compile(unsigned char* pat, int patlen)
{
	if(patlen>4)
		return bm_compile(pat, patlen);
	else
		return kmp_compile(pat, patlen);
}

static inline int str_search(unsigned char* str, int len, void* ctx)
{
	return ((strsearch_context_t*)ctx)->searchop(str, len, ctx);
}

static inline void str_context_free(void* ctx)
{
	free(ctx);
}
