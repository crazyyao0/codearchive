#include "dynamic_object.h"
#include <malloc.h>
#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#pragma warning(disable:4996)
#define snprintf _snprintf
#endif

dynamic_object* dynamic_create()
{
	dynamic_object *v = (dynamic_object*)malloc(sizeof(dynamic_object));
	v->type = DYNAMIC_TYPE_NULL;
	return v;
}

void dynamic_delete(dynamic_object *v)
{
	dynamic_set_null(v);
	free(v);
}

dynamic_object* dynamic_set_null(dynamic_object* obj)
{
	int i;
	if(obj->type == DYNAMIC_TYPE_STRING)
	{
		free(obj->string_value);
	}
	else if(obj->type == DYNAMIC_TYPE_BYTES)
	{
		free(obj->bytes_value);
	}
	else if(obj->type == DYNAMIC_TYPE_ARRAY)
	{
		for(i=0;i<obj->array_value->count;i++)
		{
			dynamic_set_null(obj->array_value->objects + i);
		}
		free(obj->array_value);
	}
	else if(obj->type == DYNAMIC_TYPE_MAP)
	{
		list_head* item = obj->map_value->all.next;
		while(item != &obj->map_value->all)
		{
			list_head* next = item->next;
			dynamic_hash_entry* entry = list_entry(item, dynamic_hash_entry, list);
			list_del(item);
			dynamic_set_null(&entry->value);
			free(item);
			item = next;
		}
		free(obj->map_value);
	}
	obj->type = DYNAMIC_TYPE_NULL;
	return obj;
}

dynamic_object* dynamic_set_bool(dynamic_object*obj, int b)
{
	dynamic_set_null(obj);
	obj->type = DYNAMIC_TYPE_BOOL;
	if(b)
		obj->bool_value = 1;
	else
		obj->bool_value = 0;
	return obj;
}

dynamic_object* dynamic_set_int(dynamic_object*obj, long long a)
{
	dynamic_set_null(obj);
	obj->type = DYNAMIC_TYPE_INTEGER;
	obj->int_value = a;
	return obj;
}

dynamic_object* dynamic_set_double(dynamic_object*obj, double a)
{
	dynamic_set_null(obj);
	obj->type = DYNAMIC_TYPE_DOUBLE;
	obj->double_value = a;
	return obj;
}

dynamic_object* dynamic_empty_string(dynamic_object*obj)
{
	dynamic_set_null(obj);
	obj->type = DYNAMIC_TYPE_STRING;
	obj->string_value = (dynamic_string*)malloc(sizeof(dynamic_string) + 8);
	obj->string_value->len = 0;
	obj->string_value->max = 8;
	obj->string_value->str[0] = 0;
	return obj;
}

dynamic_object* dynamic_string_append(dynamic_object*obj, char* s, int slen)
{
	int len;
	int max;

	if(obj->type != DYNAMIC_TYPE_STRING)
	{
		dynamic_empty_string(obj);
	}

	len = obj->string_value->len + slen;
	max = obj->string_value->max;
	while(max < len)
		max *= 2;

	if(max != obj->string_value->max)
	{
		dynamic_string* newstr = (dynamic_string*)malloc(sizeof(dynamic_string) + max);
		newstr->len = obj->string_value->len;
		newstr->max = max;
		memcpy(newstr->str, obj->string_value->str, obj->string_value->len);
		newstr->str[obj->string_value->len + 1] = 0;
		free(obj->string_value);
		obj->string_value = newstr;
	}

	memcpy(obj->string_value->str + obj->string_value->len, s, slen + 1);
	obj->string_value->len = len;
	obj->string_value->str[len+1] = 0;
	return obj;
}

dynamic_object* dynamic_set_string(dynamic_object*obj, char* s)
{
	dynamic_empty_string(obj);
	dynamic_string_append(obj, s, strlen(s));
	return obj;
}

dynamic_object* dynamic_set_nstring(dynamic_object*obj, char* s, int slen)
{
	dynamic_empty_string(obj);
	dynamic_string_append(obj, s, slen);
	return obj;
}

char* dynamic_get_string(dynamic_object*obj)
{
	if(obj->type != DYNAMIC_TYPE_STRING)
		return 0;
	else
		return obj->string_value->str;
}

dynamic_object* dynamic_set_bytes(dynamic_object*obj, char* buf, int len)
{
	dynamic_set_null(obj);
	obj->type = DYNAMIC_TYPE_BYTES;
	obj->bytes_value = (dynamic_buffer*)malloc(sizeof(dynamic_buffer) + len);
	obj->bytes_value->len = len;
	memcpy(obj->bytes_value->buf, buf, obj->bytes_value->len);
	return obj;
}

dynamic_object* dynamic_empty_map(dynamic_object* obj)
{
	dynamic_set_null(obj);
	obj->type = DYNAMIC_TYPE_MAP;
	obj->map_value = (dynamic_hash_table*)malloc(sizeof(dynamic_hash_table) + sizeof(struct list_head *) * 4);
	INIT_LIST_HEAD(&obj->map_value->all);
	obj->map_value->max = 4;
	obj->map_value->count = 0;
	memset(obj->map_value->hash_header, 0, sizeof(struct list_head *) * 4);
	return obj;
}

unsigned int dynamic_hash_string(char* str)
{
	unsigned int hash = 1315423911;
	while (*str)
		hash ^= ((hash << 5) + (*str++) + (hash >> 2));
	return (hash & 0x7FFFFFFF);
}

void dynamic_hash_insert(dynamic_hash_table* table, dynamic_hash_entry* entry)
{
	unsigned int hash = dynamic_hash_string(entry->key) % table->max;
	list_head** entry_head = &table->hash_header[hash];
	entry->hash = hash;
	if(*entry_head)
	{
		list_add(&entry->list, *entry_head);
	}else
	{
		list_add(&entry->list, &table->all);
		*entry_head = &entry->list;
	}
	table->count ++;
}

dynamic_hash_entry* dynamic_hash_find(dynamic_hash_table* table, char* key)
{
	unsigned int hash = dynamic_hash_string(key) % table->max;
	list_head** entry_head = &table->hash_header[hash];
	if(*entry_head)
	{
		list_head* item = *entry_head;
		while(1)
		{
			dynamic_hash_entry* entry = list_entry(item, dynamic_hash_entry, list);
			if(entry->hash != hash)
				break;
			if(strcmp(entry->key, key) == 0)
				return entry;
			item=item->next;
		}
	}
	return 0;
}

dynamic_object* dynamic_map_insert(dynamic_object* map, char* key)
{
	dynamic_hash_entry* newentry;
	int keylen;

	if(map->type != DYNAMIC_TYPE_MAP)
	{
		dynamic_empty_map(map);
	}
	
	newentry = dynamic_hash_find(map->map_value, key);
	if(newentry)
	{
		return &newentry->value;
	}

	if(map->map_value->count >= map->map_value->max)
	{
		// rehash map
		list_head* item;

		dynamic_hash_table* newtable = (dynamic_hash_table*)malloc(sizeof(dynamic_hash_table) + sizeof(struct list_head *) * map->map_value->max * 2);
		newtable->max = map->map_value->max * 2;
		newtable->count = 0;
		INIT_LIST_HEAD(&newtable->all);
		memset(newtable->hash_header, 0, sizeof(struct list_head *) * newtable->max);

		item = map->map_value->all.next;
		while(item != &map->map_value->all)
		{
			list_head* next = item->next;
			dynamic_hash_entry* entry = list_entry(item, dynamic_hash_entry, list);
			list_del(item);
			dynamic_hash_insert(newtable, entry);
			item = next;
		}
		free(map->map_value);
		map->map_value = newtable;
	}
	keylen = strlen(key);
	newentry = (dynamic_hash_entry*)malloc(sizeof(dynamic_hash_entry) + keylen);
	strcpy(newentry->key, key);
	dynamic_set_null(&newentry->value);
	INIT_LIST_HEAD(&newentry->list);
	dynamic_hash_insert(map->map_value, newentry);
	return &newentry->value;
}

dynamic_object* dynamic_map_find(dynamic_object* map, char* key)
{
	dynamic_hash_entry* e;
	if(map->type != DYNAMIC_TYPE_MAP)
		return 0;
	e = dynamic_hash_find(map->map_value, key);
	if(e)
		return &e->value;
	return 0;
}

dynamic_hash_entry* dynamic_map_first(dynamic_object* map)
{
	list_head* item;
	if(map->type != DYNAMIC_TYPE_MAP)
		return 0;
	item = map->map_value->all.next;
	if(item == &map->map_value->all)
		return 0;
	return list_entry(item, dynamic_hash_entry, list);
}

dynamic_hash_entry* dynamic_map_next(dynamic_object* map, dynamic_hash_entry* entry)
{
	list_head* item;
	if(map->type != DYNAMIC_TYPE_MAP)
		return 0;
	item = entry->list.next;
	if(item == &map->map_value->all)
		return 0;
	return list_entry(item, dynamic_hash_entry, list);
}

dynamic_object* dynamic_empty_array(dynamic_object* obj)
{
	dynamic_set_null(obj);
	obj->type = DYNAMIC_TYPE_ARRAY;
	obj->array_value = (dynamic_vector*) malloc(sizeof(dynamic_vector) + sizeof(dynamic_object)*4);
	obj->array_value->max = 4;
	obj->array_value->count = 0;
	memset(obj->array_value->objects, 0, sizeof(dynamic_object)*4);
	return obj;
}

dynamic_object* dynamic_array_get(dynamic_object* arr, int i)
{
	int j;
	dynamic_object *e;
	if(arr->type != DYNAMIC_TYPE_ARRAY)
	{
		dynamic_empty_array(arr);
	}

	if(i >= arr->array_value->max)
	{
		dynamic_vector* newvector;

		arr->array_value->max *= 2;
		while(i >= arr->array_value->max)
		{
			arr->array_value->max *=2;
		}

		newvector = (dynamic_vector*) malloc(sizeof(dynamic_vector) + sizeof(dynamic_object) * arr->array_value->max);
		newvector->max = arr->array_value->max;
		newvector->count = arr->array_value->count;
		memset(newvector->objects, 0, sizeof(dynamic_object) * arr->array_value->max);
		memcpy(newvector->objects, arr->array_value->objects, sizeof(dynamic_object) * arr->array_value->count);
		free(arr->array_value);
		arr->array_value = newvector;
	}

	if(i >= arr->array_value->count)
	{
		for(j=arr->array_value->count; j <= i;j++)
		{
			e = arr->array_value->objects + j;
			e->type = DYNAMIC_TYPE_NULL;
		}
		arr->array_value->count = j;
	}

	return arr->array_value->objects + i;
}

dynamic_object* dynamic_array_append(dynamic_object* arr)
{
	return dynamic_array_get(arr, arr->array_value->count);
}

int dynamic_is_null(dynamic_object* obj)
{
	if(obj->type == DYNAMIC_TYPE_NULL)
		return 1;
	else
		return 0;
}

int dynamic_is_bool(dynamic_object* obj)
{
	if(obj->type == DYNAMIC_TYPE_BOOL)
		return 1;
	else
		return 0;
}

int dynamic_is_int(dynamic_object* obj)
{
	if(obj->type == DYNAMIC_TYPE_INTEGER)
		return 1;
	else
		return 0;
}

int dynamic_is_double(dynamic_object* obj)
{
	if(obj->type == DYNAMIC_TYPE_DOUBLE)
		return 1;
	else
		return 0;
}

int dynamic_is_string(dynamic_object* obj)
{
	if(obj->type == DYNAMIC_TYPE_STRING)
		return 1;
	else
		return 0;
}

int dynamic_is_bytes(dynamic_object* obj)
{
	if(obj->type == DYNAMIC_TYPE_BYTES)
		return 1;
	else
		return 0;
}

int dynamic_is_map(dynamic_object* obj)
{
	if(obj->type == DYNAMIC_TYPE_MAP)
		return 1;
	else
		return 0;
}

int dynamic_is_array(dynamic_object* obj)
{
	if(obj->type == DYNAMIC_TYPE_ARRAY)
		return 1;
	else
		return 0;
}

int dynamic_as_bool(dynamic_object* obj)
{
	switch(obj->type)
	{
	case DYNAMIC_TYPE_BOOL:
		return obj->bool_value;
	case DYNAMIC_TYPE_INTEGER:
		if(obj->int_value != 0)
			return 1;
		else
			return 0;
	case DYNAMIC_TYPE_DOUBLE:
		if(obj->double_value != 0.0)
			return 1;
		else
			return 0;
	case DYNAMIC_TYPE_STRING:
		if(obj->string_value->len == 0 || strcmp(obj->string_value->str, "false") == 0)
			return 0;
		else
			return 1;
	case DYNAMIC_TYPE_BYTES:
		if(obj->bytes_value->len)
			return 1;
		else
			return 0;
	case DYNAMIC_TYPE_ARRAY:
		if(obj->array_value->count)
			return 1;
		else
			return 0;
	case DYNAMIC_TYPE_MAP:
		if(obj->map_value->count)
			return 1;
		else
			return 0;
	default:
		return 0;
	}
}

long long dynamic_as_integer(dynamic_object* obj)
{
	switch(obj->type)
	{
	case DYNAMIC_TYPE_BOOL:
		return obj->bool_value;
	case DYNAMIC_TYPE_INTEGER:
		return obj->int_value;
	case DYNAMIC_TYPE_DOUBLE:
		return (long long)obj->double_value;
	case DYNAMIC_TYPE_STRING:
		return atol(obj->string_value->str);
	case DYNAMIC_TYPE_BYTES:
		return obj->bytes_value->len;
	case DYNAMIC_TYPE_ARRAY:
		return obj->array_value->count;
	case DYNAMIC_TYPE_MAP:
		return obj->map_value->count;
	default:
		return 0;
	}
}

double dynamic_as_double(dynamic_object* obj)
{
	switch(obj->type)
	{
	case DYNAMIC_TYPE_BOOL:
		return (double)obj->bool_value;
	case DYNAMIC_TYPE_INTEGER:
		return (double)obj->int_value;
	case DYNAMIC_TYPE_DOUBLE:
		return obj->double_value;
	case DYNAMIC_TYPE_STRING:
		return atof(obj->string_value->str);
	case DYNAMIC_TYPE_BYTES:
		return (double)obj->bytes_value->len;
	case DYNAMIC_TYPE_ARRAY:
		return (double)obj->array_value->count;
	case DYNAMIC_TYPE_MAP:
		return (double)obj->map_value->count;
	default:
		return 0.0;
	}
}

int dynamic_as_string(dynamic_object* obj, char* buf, int len)
{
	switch(obj->type)
	{
	case DYNAMIC_TYPE_BOOL:
		if(obj->bool_value)
		{
			return snprintf(buf, len, "true");
		}else
		{
			return snprintf(buf, len, "false");
		}
	case DYNAMIC_TYPE_INTEGER:
		return snprintf(buf, len, "%lld", obj->int_value);
	case DYNAMIC_TYPE_DOUBLE:
		return snprintf(buf, len, "%lf", obj->double_value);
	case DYNAMIC_TYPE_STRING:
		return snprintf(buf, len, "%s", obj->string_value);
	case DYNAMIC_TYPE_BYTES:
		return snprintf(buf, len, "buf(%d)", obj->bytes_value->len);
	case DYNAMIC_TYPE_ARRAY:
		return snprintf(buf, len, "array(%d)", obj->array_value->count);
	case DYNAMIC_TYPE_MAP:
		return snprintf(buf, len, "map(%d)", obj->map_value->count);
	default:
		buf[0] = 0;
		return 1;
	}
}

unsigned char* dynamic_as_bytes(dynamic_object* obj)
{
	switch(obj->type)
	{
	case DYNAMIC_TYPE_BOOL:
		return (unsigned char*)&obj->bool_value;
	case DYNAMIC_TYPE_INTEGER:
		return (unsigned char*)&obj->int_value;
	case DYNAMIC_TYPE_DOUBLE:
		return (unsigned char*)&obj->double_value;
	case DYNAMIC_TYPE_STRING:
		return (unsigned char*)obj->string_value->str;
	case DYNAMIC_TYPE_BYTES:
		return obj->bytes_value->buf;
	default:
		return 0;
	}
}

int dynamic_size(dynamic_object* obj)
{
	switch(obj->type)
	{
	case DYNAMIC_TYPE_BOOL:
		return 1;
	case DYNAMIC_TYPE_INTEGER:
		if(obj->int_value | 0xFFFFFFFFFFFFFF00)
			return 1;
		else if(obj->int_value | 0xFFFFFFFFFFFF0000)
			return 2;
		else if(obj->int_value | 0xFFFFFFFF00000000)
			return 4;
		else
			return 8;
	case DYNAMIC_TYPE_DOUBLE:
		return 8;
	case DYNAMIC_TYPE_STRING:
		return obj->string_value->len;
	case DYNAMIC_TYPE_BYTES:
		return obj->bytes_value->len;
	case DYNAMIC_TYPE_ARRAY:
		return obj->array_value->count;
	case DYNAMIC_TYPE_MAP:
		return obj->map_value->count;
	default:
		return 0;
	}
}

void dynamic_dump(dynamic_object* obj, int tab)
{
	int i;
	int j;
	dynamic_hash_entry* e;

	switch(obj->type)
	{
	case DYNAMIC_TYPE_NULL:
		printf("null\n");
		break;
	case DYNAMIC_TYPE_BOOL:
		if(obj->bool_value)
			printf("true\n");
		else
			printf("false\n");
		break;
	case DYNAMIC_TYPE_INTEGER:
		printf("%lld\n", obj->int_value);
		break;
	case DYNAMIC_TYPE_DOUBLE:
		printf("%lf\n", obj->double_value);
		break;
	case DYNAMIC_TYPE_STRING:
		printf("%s\n", obj->string_value->str);
		break;
	case DYNAMIC_TYPE_BYTES:
		printf("bytes(%d)\n", obj->bytes_value->len);
		break;
	case DYNAMIC_TYPE_ARRAY:
		printf("[\n");
		for(i=0;i<dynamic_size(obj);i++)
		{
			for(j=0;j<=tab;j++)
			{
				printf("  ");
			}
			dynamic_dump(dynamic_array_get(obj, i), tab+1);
		}
		for(i=0;i<tab;i++)
			printf("  ");
		printf("]\n");
		break;
	case DYNAMIC_TYPE_MAP:
		printf("{\n");
		e = dynamic_map_first(obj);
		while(e)
		{
			for(j=0;j<=tab;j++)
			{
				printf("  ");
			}
			printf("%s:",e->key);
			dynamic_dump(&e->value, tab+1);
			e = dynamic_map_next(obj, e);
		}
		for(i=0;i<tab;i++)
			printf("  ");
		printf("}\n");
		break;
	}
}