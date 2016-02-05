#include <memory.h>
#include <stdio.h>
#include "json.h"

char json_getch(json_buf* buf)
{
	if(buf->cur < buf->end)
	{
		char ch = *(char*)(buf->cur);
		buf->cur += 1;
		return ch;
	}else
	{
		buf->cur = buf->end;
		buf->overflow = 1;
		return 0;
	}
}

char json_peekch(json_buf* buf)
{
	if(buf->cur < buf->end)
		return *(char*)(buf->cur);
	else
		return 0;
}

int json_gettoken(json_buf* buf, json_token* tag)
{
	char ch;
	ch = json_getch(buf);
	while(ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n')
		ch = json_getch(buf);

	tag->type = JSON_ERROR;
	tag->len = 0;

	if(ch == '[')
	{
		tag->type = JSON_BEGINARRAY;
		return 1;
	}else if(ch == ']')
	{
		tag->type = JSON_ENDARRAY;
		return 1;
	}else if(ch == '{')
	{
		tag->type = JSON_BEGINOBJECT;
		return 1;
	}else if(ch == '}')
	{
		tag->type = JSON_ENDOBJECT;
		return 1;
	}else if(ch == ':')
	{
		tag->type = JSON_COLON;
		return 1;
	}else if(ch == ',')
	{
		tag->type = JSON_COMMA;
		return 1;
	}else if(ch == '"')
	{
		char c;
		char *dst = buf->cur;
		tag->type = JSON_STRING;
		tag->str_value = buf->cur;
		tag->len = 0;
		c = json_getch(buf);
		while(!buf->overflow && c!='\"')
		{
			tag->len ++;
			if(c == '\\')
			{
				tag->len ++;
				c = json_getch(buf);
				if(c == 'b')
					*dst++ = '\b';
				else if(c == 'f')
					*dst++ = '\f';
				else if(c == 'n')
					*dst++ = '\n';
				else if(c == 'r')
					*dst++ = '\r';
				else if(c == 't')
					*dst++ = '\t';
				else if(c == '\\')
					*dst++ = '\\';
				else if(c == '\"')
					*dst++ = '\"';
				else if(c == '/')
					*dst++ = '/';
			}else
			{
				*dst++ = c;
			}
			c = json_getch(buf);
		}
		if(buf->overflow)
			return 0;
		*dst = 0;
		return 1;
	}else if(ch == 't')
	{
		if(json_getch(buf) == 'r' && json_getch(buf) == 'u' && json_getch(buf) == 'e')
		{
			tag->type = JSON_BOOLEAN;
			tag->bool_value = 1;
			return 1;
		}else
		{
			return 0;
		}
	}else if(ch == 'f')
	{
		if(json_getch(buf) == 'a' && json_getch(buf) == 'l' && json_getch(buf) == 's' && json_getch(buf) == 'e')
		{
			tag->type = JSON_BOOLEAN;
			tag->bool_value = 0;
			return 1;
		}else
		{
			return 0;
		}
	}else if(ch == 'n')
	{
		if(json_getch(buf) == 'u' && json_getch(buf) == 'l' && json_getch(buf) == 'l')
		{
			tag->type = JSON_NULL;
			return 1;
		}else
		{
			return 0;
		}
	}else if(ch >= '0' && ch <= '9' || ch == '-')
	{
		char c;
		tag->type = JSON_INTEGER;
		tag->str_value = buf->cur - 1;
		tag->len = 1;
		c = json_peekch(buf);
		while(c >= '0' && c <= '9' || c == '-' || c == '+' || c == '.' || c=='e' || c=='E')
		{
			if(c == '.')
				tag->type = JSON_NUMBER;
			json_getch(buf);
			tag->len = 1;
			c = json_peekch(buf);
		}

		if(tag->type == JSON_INTEGER)
		{
			if(sscanf(tag->str_value, "%lld", &tag->int_value))
				return 1;
			else
				return 0;
		}else
		{
			if(sscanf(tag->str_value, "%lf", &tag->number_value))
				return 1;
			else
				return 0;
		}
	}else
	{
		return 0;
	}
}

int json_putch(json_buf* buf, char ch)
{
	if(buf)
	{
		if(buf->cur + 1 <= buf->end)
		{
			*buf->cur = (unsigned char)ch;
			buf->cur ++;
		}else
		{
			buf->overflow = 1;
		}
	}
	return 1;
}

int json_put(json_buf* buf, void* var, int len)
{
	if(buf && var && len)
	{
		if(buf->cur + len <= buf->end)
		{
			memcpy(buf->cur, var, len);
			buf->cur += len;
		}else
		{
			buf->overflow = 1;
		}
	}
	return len;
}

int json_write_null(json_buf* buf)
{
	return json_put(buf, (char*)"null", 4);
};

int json_write_bool(json_buf* buf, int b)
{
	if(b)
		return json_put(buf, (char*)"true", 4);
	else
		return json_put(buf, (char*)"false", 5);
};

int json_write_number(json_buf* buf, double d)
{
	char buffer[32];
	int len =sprintf(buffer, "%lf", d);
	return json_put(buf, buffer, len);
};

int json_write_integer(json_buf* buf, long long d)
{
	char buffer[32];
	int len =sprintf(buffer, "%lld", d);
	return json_put(buf, buffer, len);
};

int json_write_string(json_buf* buf, char* s)
{
	int len = json_putch(buf, '\"');
	while(*s)
	{
		if(*s == '\"')
			len += json_put(buf, (char*)"\\\"", 2);
		else if(*s == '\\')
			len += json_put(buf, (char*)"\\\\", 2);
		else if(*s == '\b')
			len += json_put(buf, (char*)"\\b", 2);
		else if(*s == '\f')
			len += json_put(buf, (char*)"\\f", 2);
		else if(*s == '\n')
			len += json_put(buf, (char*)"\\n", 2);
		else if(*s == '\r')
			len += json_put(buf, (char*)"\\r", 2);
		else if(*s == '\t')
			len += json_put(buf, (char*)"\\t", 2);
		else
			len += json_putch(buf, *s);
		s++;
	}
	len += json_putch(buf, '\"');
	return len;
}
