#include <malloc.h>
#include <memory.h>

struct bm_context_t
{
	int (*searchop)(unsigned char*, int, void*);
	unsigned char* pat;
	int patlen;
	int bmBc[256];
	int bmGs[1];
};
int bm_search(unsigned char* str, int len, void* ctx);

void* bm_compile(unsigned char* pat, int patlen)
{
	int i,j,f,g = patlen - 1;
	int* suff = (int*)malloc(sizeof(int)*patlen);
	bm_context_t* r = (bm_context_t*)malloc(sizeof(bm_context_t) + sizeof(int)*patlen + patlen);

	r->patlen = patlen;
	r->pat = (unsigned char*)(r->bmGs + patlen);
	memcpy(r->pat, pat, patlen);
	r->searchop = bm_search;

	/* gen bmBC*/
    for (i=0; i < 256; i++)
		r->bmBc[i] = patlen;
    for (i=0; i < patlen - 1; i++)
        r->bmBc[pat[i]] = patlen - 1 - i;

	/* gen suff */
	suff[patlen - 1] = patlen;
	for (i = patlen - 2; i >= 0; --i) {
		if (i > g && suff[i + patlen - 1 - f] < i - g)
			suff[i] = suff[i + patlen - 1 - f];
		else {
			if (i < g)
				g = i;
			f = i;
			while (g >= 0 && pat[g] == pat[g + patlen - 1 - f])
				--g;
			suff[i] = f - g;
		}
	}

	/* gen bmGs */
	for (i = 0; i < patlen; ++i)
		r->bmGs[i] = patlen;
	j = 0;
	for (i = patlen - 1; i >= 0; --i)
		if (suff[i] == i + 1)
			for (; j < patlen - 1 - i; ++j)
				if (r->bmGs[j] == patlen)
					r->bmGs[j] = patlen - 1 - i;
	for (i = 0; i <= patlen - 2; ++i)
		r->bmGs[patlen - 1 - suff[i]] = patlen - 1 - i;

	free(suff);
	return r;
}

#define MAX(a,b) (a)>(b)?(a):(b)

int bm_search(unsigned char* str, int len, void* ctx)
{
	int i, j = 0;
	unsigned char* pat = ((bm_context_t*)ctx)->pat;
	int panlen = ((bm_context_t*)ctx)->patlen;

	while (j <= len - panlen) {
		for (i = panlen - 1; i >= 0 && pat[i] == str[i + j]; --i);
		if (i < 0)
			return j;
		else
			j += MAX(((bm_context_t*)ctx)->bmGs[i], ((bm_context_t*)ctx)->bmBc[str[i + j]] - panlen + 1 + i);
	}
	return -1;
}

struct kmp_context_t
{
	int (*searchop)(unsigned char*, int, void*);
	unsigned char* pat;
	int patlen;
	int kmpNext[1];
};
int kmp_search(unsigned char* str, int len, void* ctx);

void* kmp_compile(unsigned char* pat, int patlen)
{
	int i,j;
	kmp_context_t* r = (kmp_context_t*)malloc(sizeof(kmp_context_t) + sizeof(int)*patlen + patlen);

	r->patlen = patlen;
	r->pat = (unsigned char*)(r->kmpNext + patlen);
	memcpy(r->pat, pat, patlen);
	r->searchop = kmp_search;

	i = 0;
	j = r->kmpNext[0] = -1;
	while (i < patlen) {
		while (j > -1 && pat[i] != pat[j])
			j = r->kmpNext[j];
		i++;
		j++;
		if (pat[i] == pat[j])
			r->kmpNext[i] = r->kmpNext[j];
		else
			r->kmpNext[i] = j;
	}
	return r;
}

int kmp_search(unsigned char* str, int len, void* ctx)
{
	int i, j;
	unsigned char* pat = ((kmp_context_t*)ctx)->pat;
	int panlen = ((kmp_context_t*)ctx)->patlen;

	i = j = 0;
	while (j < len) {
		while (i > -1 && pat[i] != str[j])
			i = ((kmp_context_t*)ctx)->kmpNext[i];
		i++;
		j++;
		if (i >= panlen)
			return j;
	}
	return -1;
}
