#include "dynamic_object.h"
#include "json.h"

int dynamic_from_jsonvalue(dynamic_object* ret, json_buf* buf)
{
	int i;
	json_token token;

	if(!json_gettoken(buf, &token))
	{
		return 0;
	}

	switch(token.type)
	{
	case JSON_INTEGER:
		dynamic_set_int(ret, token.int_value);
		return 1;
	case JSON_NUMBER:
		dynamic_set_double(ret, token.number_value);
		return 1;
	case JSON_STRING:
		dynamic_set_string(ret, token.str_value);
		return 1;
	case JSON_BOOLEAN:
		dynamic_set_bool(ret, token.bool_value);
		return 1;
	case JSON_NULL:
		dynamic_set_null(ret);
		return 1;
	case JSON_BEGINOBJECT:
		dynamic_empty_map(ret);
		while(1)
		{
			dynamic_object* v;
			if(!json_gettoken(buf, &token))
				return 0;
			else if(token.type == JSON_ENDOBJECT)
				break;
			else if(token.type != JSON_STRING)
				return 0;
			v = dynamic_map_insert(ret, token.str_value);

			if(!json_gettoken(buf, &token) || token.type != JSON_COLON)
				return 0;

			if(!dynamic_from_jsonvalue(v, buf))
				return 0;

			if(!json_gettoken(buf, &token))
				return 0;
			else if(token.type == JSON_ENDOBJECT)
				break;
			else if(token.type != JSON_COMMA)
				return 0;
		}
		return 1;
	case JSON_BEGINARRAY:
		dynamic_empty_array(ret);
		i = 0;
		while(1)
		{
			dynamic_object* v;
			if(json_peekch(buf) == JSON_ENDARRAY)
			{
				json_gettoken(buf, &token);
				break;
			}

			v = dynamic_array_get(ret, i++);
			if(!dynamic_from_jsonvalue(v, buf))
				return 0;

			if(!json_gettoken(buf, &token))
				return 0;
			else if(token.type == JSON_ENDARRAY)
				break;
			else if(token.type != JSON_COMMA)
				return 0;
		}
		return 1;
	default:
		return 0;
	}
}

int dynamic_to_jsonvalue(json_buf* buf, dynamic_object* obj)
{
	switch(obj->type)
	{
	case DYNAMIC_TYPE_NULL:
		return json_write_null(buf);
	case DYNAMIC_TYPE_BOOL:
		return json_write_bool(buf, obj->bool_value);
	case DYNAMIC_TYPE_INTEGER:
		return json_write_integer(buf, obj->int_value);
	case DYNAMIC_TYPE_DOUBLE:
		return json_write_number(buf, obj->double_value);
	case DYNAMIC_TYPE_STRING:
		return json_write_string(buf, obj->string_value->str);
	case DYNAMIC_TYPE_MAP:
		{
			dynamic_hash_entry* e = dynamic_map_first(obj);
			int len = json_putch(buf, '{');
			while(e)
			{
				len += json_write_string(buf, e->key);
				len += json_putch(buf, ':');
				len += dynamic_to_jsonvalue(buf, &e->value);
				e = dynamic_map_next(obj, e);
				if(e)
					len += json_putch(buf, ',');
			}
			len += json_putch(buf, '}');
			return len;
		}
	case DYNAMIC_TYPE_ARRAY:
		{
			int i;
			int size = dynamic_size(obj);
			int len = json_putch(buf, '[');
			for(i=0;i<size;i++)
			{
				len += dynamic_to_jsonvalue(buf, dynamic_array_get(obj, i));
				if(i+1 != size)
					len += json_putch(buf, ',');
			}
			len += json_putch(buf, ']');
			return len;
		}
	default:
		return 0;
	}
}

dynamic_object* dynamic_from_json(char* buffer, int len)
{
	dynamic_object* ret;
	json_buf buf;

	buf.start = buffer;
	buf.end = buffer + len;
	buf.cur = buffer;
	buf.overflow = 0;

	ret = dynamic_create();
	if(!dynamic_from_jsonvalue(ret, &buf))
	{
		dynamic_delete(ret);
		return 0;
	}else
	{
		return ret;
	}
}

int dynamic_to_json(char* buf, int *len, dynamic_object* obj)
{
	json_buf buffer;
	buffer.start = buf;
	buffer.end = buf + *len;
	buffer.cur = buf;
	buffer.overflow = 0;

	*len = dynamic_to_jsonvalue(&buffer, obj);
	if(buffer.overflow)
		return 0;
	else
		return 1;
}