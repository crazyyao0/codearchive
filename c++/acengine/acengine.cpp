#include <stdio.h>
#include <malloc.h>
#include <memory.h>
#include <queue>
#include "acengine.h"

void ac_heap_init(ac_alloc_t* heap)
{
	heap->current = 0;
	heap->end = 0;
}

unsigned char* ac_heap_alloc(ac_alloc_t* heap, int size)
{
	unsigned char* ret = 0;

	if(heap->current + size > heap->end)
	{
		heap->current = (unsigned char*)malloc(65500);
		heap->end = heap->current + 65500;
		heap->mempool.push_back(heap->current);
	}

	ret = heap->current;
	heap->current += size;
	return ret;
}

void ac_heap_destory(ac_alloc_t* heap)
{
	for(size_t i=0; i<heap->mempool.size(); i++)
		free(heap->mempool[i]);
	heap->mempool.clear();
	heap->current = 0;
	heap->end = 0;
}


static ac_state_t* ac_create_state256(ac_context_t* ctx)
{
	ac_state_t* state = (ac_state_t*)ac_heap_alloc(&ctx->heap, sizeof(ac_state256_t));
	memset(state->sub_states, -1, sizeof(int)*256);
	state->has_output = 0;
	state->symbol = 0;
	state->failure = 0;
	state->output = 0xFFFFFFFF;
	state->num_sub_states = 256;

	ctx->alloccount++;
	ctx->allocsize += sizeof(ac_state256_t);
	return state;
}

static ac_state_t* ac_create_state1(ac_context_t* ctx)
{
	ac_state_t* state = (ac_state_t*)ac_heap_alloc(&ctx->heap, sizeof(ac_state_t));
	state->sub_states[0] = 0xFFFFFFFF;
	state->has_output = 0;
	state->symbol = 0;
	state->failure = 0;
	state->output = 0xFFFFFFFF;
	state->num_sub_states = 1;

	ctx->alloccount++;
	ctx->allocsize += sizeof(ac_state_t);
	return state;
}

static void ac_append_output(ac_context_t* ctx, int s, unsigned int id)
{
	ac_state_t* state = ctx->all_states[s];
	if(state->has_output == 0)
	{
		state->has_output = 1;
		state->output = id;
		ctx->outputcount++;
	}else if (state->has_output == 1)
	{
		unsigned int* p = (unsigned int*)ac_heap_alloc(&ctx->heap, sizeof(unsigned int)*2);
		p[0] = state->output;
		p[1] = id;
		state->output = ctx->outputs.size();
		ctx->outputs.push_back(p);
		state->has_output = 2;

		ctx->alloccount++;
		ctx->allocsize += sizeof(unsigned int)*2;
	}else if(state->has_output < 255)
	{
		unsigned int* n = (unsigned int*)ac_heap_alloc(&ctx->heap, sizeof(unsigned int)*(state->has_output+1));
		unsigned int* o = ctx->outputs[state->output];
		memcpy(n, o, sizeof(unsigned int)*state->has_output);
		n[state->has_output] = id;
		ctx->outputs[state->output] = n;
		state->has_output++;

		ctx->freecount++;
		ctx->freesize += sizeof(unsigned int)*(state->has_output-1);
		ctx->alloccount++;
		ctx->allocsize += sizeof(unsigned int)*state->has_output;
	}else
	{
		printf("output count limit exceeded!\n");
	}
}

static unsigned int ac_find_state(ac_context_t* ctx, int s, unsigned char c)
{
	ac_state_t* state = ctx->all_states[s];
	if(state->num_sub_states == 0)
		return -1;
	else if(state->num_sub_states == 1)
		if(c == (state->sub_states[0] & 0xFF))
			return state->sub_states[0] >> 8;
		else
			return -1;
	else if(state->sub_states[c] == -1)
		return -1;
	else
		return state->sub_states[c] >> 8;
}

static void ac_insert_state(ac_context_t* ctx, int p, int s, unsigned char c)
{
	ac_state_t* state = ctx->all_states[p];
	unsigned int sv = s;
	sv = (sv << 8) + c;

	if(state->num_sub_states == 0)
	{
		state->sub_states[0] = sv;
		state->num_sub_states = 1;
	}else if(state->num_sub_states == 1)
	{
		ac_state_t* newstate = ac_create_state256(ctx);
		newstate->failure = state->failure;
		newstate->has_output = state->has_output;
		newstate->output = state->output;
		newstate->symbol = state->symbol;
		newstate->parent= state->parent;
		newstate->sub_states[state->sub_states[0]&0xFF] = state->sub_states[0];
		newstate->sub_states[c] = sv;
		ctx->all_states[p] = newstate;

		ctx->freecount++;
		ctx->freesize += sizeof(ac_state_t);
	}else
	{
		state->sub_states[c] = sv;
	}
}

void ac_init(ac_context_t* ctx)
{
	ac_state_t* root;
	ac_heap_init(&ctx->heap);
	root = ac_create_state256(ctx);
	root->failure = -1;
	root->parent = -1;
	ctx->all_states.clear();
	ctx->all_states.push_back(root);

	ctx->alloccount = 0;
	ctx->allocsize = 0;
	ctx->freecount = 0;
	ctx->freesize = 0;
	ctx->outputcount = 0;
}

void ac_free(ac_context_t* ctx)
{
	ac_heap_destory(&ctx->heap);
	ctx->all_states.clear();
	ctx->outputs.clear();
}

int ac_addstring(ac_context_t* ctx, unsigned char* buf, int len, unsigned int id)
{
	int current = 0;
	for(int i=0;i<len;i++)
	{
		unsigned char a = buf[i];
		unsigned int s = ac_find_state(ctx, current, a);
		if(s == -1)
		{
			int next = ctx->all_states.size();
			ac_state_t* state = ac_create_state1(ctx);
			state->num_sub_states = 0;
			state->symbol = a;
			state->parent = current;
			ctx->all_states.push_back(state);
			ac_insert_state(ctx, current, next, a);
			current = next;
		}else
			current = s;
	}
	ac_append_output(ctx, current, id);
	return 1;
}

static int ac_finish_internal(ac_context_t* ctx, int current)
{
	ac_state_t* node = ctx->all_states[current];
	int parent = node->parent;

	if(current == 0)
		return 1;

	while(1)
	{
		ac_state_t* pnode = ctx->all_states[parent];
		if(pnode->failure == -1)
		{
			node->failure = 0;
			break;
		}
		unsigned int found = ac_find_state(ctx, pnode->failure, node->symbol);
		if(found != -1)
		{
			node->failure = found;
			ac_state_t* fnode = ctx->all_states[found];
			if(fnode->has_output == 1)
			{
				ac_append_output(ctx, current, fnode->output);
			}else if(fnode->has_output > 1)
			{
				unsigned int* outputs = ctx->outputs[fnode->output];
				for(int i=0;i<fnode->has_output;i++)
					ac_append_output(ctx, current, outputs[i]);
			}
			break;
		}
		parent = pnode->failure;
	}
	return 1;
}

int ac_finish(ac_context_t* ctx)
{
	ac_state_t* root = ctx->all_states[0];
	std::queue<int> nodelist;
	nodelist.push(0);

	while(!nodelist.empty())
	{
		int s = nodelist.front();
		ac_state_t* node = ctx->all_states[s];
		nodelist.pop();
		ac_finish_internal(ctx, s);

		for(int i=0; i< node->num_sub_states; i++)
		{
			if(node->sub_states[i] == 0xFFFFFFFF)
				continue;
			nodelist.push(node->sub_states[i] >> 8);
		}
	}
	return 1;
}

static void ac_dump_path(ac_context_t* ctx, int s)
{
	ac_state_t* node = ctx->all_states[s];
	if(node->parent != -1)
		ac_dump_path(ctx, node->parent);
	if(s)
	{
		if(node->symbol >= 32 && node->symbol <= 126)
			printf("%c", node->symbol);
		else
			printf("\\x%02x", node->symbol);
	}
}

void ac_dump_state(ac_context_t* ctx, int s)
{
	ac_state_t* node = ctx->all_states[s];
	printf("%d symbol=%02x failure=%d parent=%d", s, node->symbol, node->failure, node->parent);

	printf(" path='");
	ac_dump_path(ctx, s);
	printf("'");

	if(node->num_sub_states)
	{
		printf(" substates=[");
		for(int j=0; j< node->num_sub_states; j++)
		{
			if(node->sub_states[j] == 0xFFFFFFFF)
				continue;
			unsigned int state = node->sub_states[j];
			unsigned char symbol = state & 0xFF;

			if(symbol >= 32 && symbol <= 126)
				printf(" %c:%d", symbol, state >> 8);
			else
				printf(" %02x:%d", symbol, state >> 8);
		}
		printf("]");
	}

	if(node->has_output == 1)
	{
		printf(" output=[%d]", node->output);
	}else if(node->has_output >1)
	{
		unsigned int* outputs = ctx->outputs[node->output];
		printf(" output=[");
		for(int i=0;i<node->has_output;i++)
		{
			printf(" %d", outputs[i]);
		}
		printf("]");
	}
	printf("\n");
}

void ac_states_stat(ac_context_t* ctx)
{
	int sub[257];
	int output[257];
	memset(sub, 0, sizeof(sub));
	memset(output, 0, sizeof(sub));

	for(size_t i=0; i<ctx->all_states.size(); i++)
	{
		ac_state_t* node = ctx->all_states[i];
		int c = 0;
		for(int j = 0; j<node->num_sub_states; j++)
		{
			if(node->sub_states[j]!=0xFFFFFFFF)
				c++;
		}
		sub[c]++;
		output[node->has_output] ++;
	}

	printf("memory alloc size = %d\n", ctx->allocsize);
	printf("memory free size = %d\n", ctx->freesize);
	printf("memory alloc count = %d\n", ctx->alloccount);
	printf("memory free count = %d\n", ctx->freecount);

	printf("total nodes = %ld\n", ctx->all_states.size());
	for(int i=0; i<257; i++)
	{
		if(sub[i])
			printf("%d subnodes node = %d\n", i, sub[i]);
	}

	printf("total output nodes = %d\n", ctx->outputcount);
	for(int i=0; i<257; i++)
	{
		if(output[i])
			printf("%d outputs node = %d\n", i, output[i]);
	}
}

int ac_search(ac_context_t* ctx, unsigned char* buf, int len, int* base, int* state, AC_CALLBACK fn, void* userdata)
{
	int current = *state;
	unsigned char* cur = buf;
	unsigned char* end = buf+len;
	int exit_loop = 0;

	while(cur<end && exit_loop == 0)
	{
		ac_state_t* pnode;
		unsigned int found = ac_find_state(ctx, current, *cur);
		if(found == -1)
		{
			current = ctx->all_states[current]->failure;
			if(current == -1)
			{
				current = 0;
				cur++;
			}
			continue;
		}
		current = found;
		cur++;

		pnode = ctx->all_states[current];
		
		if(pnode->has_output == 1)
		{
			if(fn(cur - buf + *base, pnode->output, userdata) == AC_STOP)
				exit_loop = 1;
		}else if(pnode->has_output > 1)
		{
			unsigned int* outputs = ctx->outputs[pnode->output];
			for(int i=0; i<pnode->has_output; i++)
			{
				if(fn(cur - buf + *base, outputs[i], userdata) == AC_STOP)
				{
					exit_loop = 1;
					break;
				}
			}
		}
	}
	*base += cur - buf;
	*state = current;
	return 1;
}
