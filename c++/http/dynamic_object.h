#ifndef DYNAMIC_OBJECT_INCLUDED
#define DYNAMIC_OBJECT_INCLUDED

#include "list.h"

#ifdef _MSC_VER
  #define INLINE static __inline      /* use __forceinline (VC++ specific) */
#else
  #define INLINE static inline        /* use standard inline */
#endif

#define DYNAMIC_TYPE_NULL		0
#define DYNAMIC_TYPE_BOOL		1
#define DYNAMIC_TYPE_INTEGER	2
#define DYNAMIC_TYPE_DOUBLE		3
#define DYNAMIC_TYPE_STRING		4
#define DYNAMIC_TYPE_BYTES		5
#define DYNAMIC_TYPE_ARRAY		6
#define DYNAMIC_TYPE_MAP		7

typedef struct dynamic_object
{
	int type;
	union
	{
		int bool_value;
		long long int_value;
		double double_value;
		struct dynamic_string* string_value;
		struct dynamic_buffer* bytes_value;
		struct dynamic_vector* array_value;
		struct dynamic_hash_table* map_value;
	};
}dynamic_object;

typedef struct dynamic_buffer
{
	int len;
	unsigned char buf[1];
}dynamic_buffer;

typedef struct dynamic_string
{
	int max;
	int len;
	char str[1];
}dynamic_string;

typedef struct dynamic_vector
{
	int max;
	int count;
	struct dynamic_object objects[1];
}dynamic_vector;

typedef struct dynamic_hash_table
{
	int max;
	int count;
	struct list_head all;
	struct list_head *hash_header[1];
}dynamic_hash_table;

typedef struct dynamic_hash_entry
{
	struct list_head list;
	struct dynamic_object value;
	unsigned int hash;
	char key[1];
}dynamic_hash_entry;

dynamic_object* dynamic_create();
void dynamic_delete(dynamic_object *v);

dynamic_object* dynamic_set_null(dynamic_object* obj);
dynamic_object* dynamic_set_bool(dynamic_object*obj, int b);
dynamic_object* dynamic_set_int(dynamic_object*obj, long long a);
dynamic_object* dynamic_set_double(dynamic_object*obj, double a);
dynamic_object* dynamic_set_bytes(dynamic_object*obj, char* buf, int len);

dynamic_object* dynamic_empty_string(dynamic_object*obj);
dynamic_object* dynamic_set_string(dynamic_object*obj, char* s);
dynamic_object* dynamic_set_nstring(dynamic_object*obj, char* s, int slen);
char* dynamic_get_string(dynamic_object*obj);
dynamic_object* dynamic_string_append(dynamic_object*obj, char* s, int slen);
dynamic_object* dynamic_string_printf(dynamic_object*obj, char* s, ...);

dynamic_object* dynamic_empty_map(dynamic_object* obj);
dynamic_object* dynamic_map_insert(dynamic_object* map, char* key);
dynamic_object* dynamic_map_find(dynamic_object* map, char* key);
char* dynamic_map_get_value(dynamic_object* map, char* key);
dynamic_hash_entry* dynamic_map_first(dynamic_object* map);
dynamic_hash_entry* dynamic_map_next(dynamic_object* map, dynamic_hash_entry* entry);

dynamic_object* dynamic_empty_array(dynamic_object* obj);
dynamic_object* dynamic_array_get(dynamic_object* arr, int i);
dynamic_object* dynamic_array_append(dynamic_object* arr);

INLINE int dynamic_is_null(dynamic_object* obj)
{
	return obj->type == DYNAMIC_TYPE_NULL;
}

INLINE int dynamic_is_bool(dynamic_object* obj)
{
	return obj->type == DYNAMIC_TYPE_BOOL;
}

INLINE int dynamic_is_int(dynamic_object* obj)
{
	return obj->type == DYNAMIC_TYPE_INTEGER;
}

INLINE int dynamic_is_double(dynamic_object* obj)
{
	return obj->type == DYNAMIC_TYPE_DOUBLE;
}

INLINE int dynamic_is_string(dynamic_object* obj)
{
	return obj->type == DYNAMIC_TYPE_STRING;
}

INLINE int dynamic_is_bytes(dynamic_object* obj)
{
	return obj->type == DYNAMIC_TYPE_BYTES;
}

INLINE int dynamic_is_map(dynamic_object* obj)
{
	return obj->type == DYNAMIC_TYPE_MAP;
}

INLINE int dynamic_is_array(dynamic_object* obj)
{
	return obj->type == DYNAMIC_TYPE_ARRAY;
}

int dynamic_as_bool(dynamic_object* obj);
long long dynamic_as_integer(dynamic_object* obj);
double dynamic_as_double(dynamic_object* obj);
int dynamic_as_string(dynamic_object* obj, char* buf, int len);
unsigned char* dynamic_as_bytes(dynamic_object* obj);

int dynamic_size(dynamic_object* obj);
void dynamic_dump(dynamic_object* obj, int tab);

INLINE void dynamic_init(dynamic_object* v)
{
	v->type = DYNAMIC_TYPE_NULL;
}

INLINE void dynamic_release(dynamic_object* v)
{
	dynamic_set_null(v);
}

#endif