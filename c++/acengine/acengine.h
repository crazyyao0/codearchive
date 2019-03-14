#include <vector>

#define AC_CONTINUE 0
#define AC_STOP 1

typedef int (*AC_CALLBACK)(int offset, unsigned int id, void* userdata);

struct ac_state_t
{
	unsigned int failure;
	unsigned int output;
	unsigned int parent;
	unsigned char symbol;
	unsigned char has_output;
	unsigned short num_sub_states;
	unsigned int sub_states[1];
};

struct ac_state256_t
{
	unsigned int failure;
	unsigned int output;
	unsigned int parent;
	unsigned char symbol;
	unsigned char has_output;
	unsigned short num_sub_states;
	unsigned int sub_states[256];
};

struct ac_alloc_t
{
	std::vector<unsigned char*> mempool;
	unsigned char* current;
	unsigned char* end;
};

void ac_heap_init(ac_alloc_t* heap);
unsigned char* ac_heap_alloc(ac_alloc_t* heap, int size);
void ac_heap_destory(ac_alloc_t* heap);

struct ac_context_t
{
	std::vector<ac_state_t*> all_states;
	std::vector<unsigned int*> outputs;
	ac_alloc_t heap;

	unsigned int allocsize;
	unsigned int alloccount;
	unsigned int freesize;
	unsigned int freecount;
	unsigned int outputcount;
};

void ac_init(ac_context_t* ctx);
void ac_free(ac_context_t* ctx);
int ac_addstring(ac_context_t* ctx, unsigned char* buf, int len, unsigned int id);
int ac_finish(ac_context_t* ctx);
void ac_dump_state(ac_context_t* ctx, int s);
void ac_states_stat(ac_context_t* ctx);
int ac_search(ac_context_t* ctx, unsigned char* buf, int len, int* base, int* state, AC_CALLBACK fn, void* userdata);
