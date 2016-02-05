// DynamicObject.cpp -
//

#include "DynamicObject.h"
extern "C"
{
	#include "json.h"
};

using namespace Utilities;

int JSON_EncodeValue(json_buf* buf, DynamicObject& obj)
{
	switch (obj.type)
	{
	case DO_UNKNOWN:
		return json_write_null(buf);
	case DO_INTEGER:
		return json_write_integer(buf, obj);
	case DO_BOOLEAN:
		return json_write_bool(buf, obj);
	case DO_NUMBER:
		return json_write_number(buf, obj);
	case DO_STRING:
		return json_write_string(buf, obj);
	case DO_ARRAY:
		{
			int len = json_putch(buf, '[');
            size_t size = obj.size();
			for (size_t i = 0; i < size; i++)
			{
				len += JSON_EncodeValue(buf, obj[i]);
				if (i != size - 1)
					len += json_putch(buf, ',');
			}
			len += json_putch(buf, ']');
			return len;
		}
	case DO_OBJECT:
		{
			int len = json_putch(buf, '{');
			std::vector<std::string> keys;
            obj.propertyList(keys);
            size_t size = keys.size();
			for (size_t i = 0; i < size; i++)
			{
				std::string& key = keys[i];
				len += json_write_string(buf, (char*)key.c_str());
				len += json_putch(buf, ':');
				len += JSON_EncodeValue(buf, obj[key]);
				if (i != keys.size() - 1)
					len += json_putch(buf, ',');
			}
			len += json_putch(buf, '}');
			return len;
		}
	default:
		return 0;
	}
}

bool DynamicObject::JSON_Encode(DynamicObject& obj, std::string& str)
{
	char * jsonbuf = new char[1024];
	json_buf buffer;
	buffer.start = jsonbuf;
	buffer.end = jsonbuf + 1023;
	buffer.cur = jsonbuf;
	buffer.overflow = 0;

	int len = JSON_EncodeValue(&buffer, obj);
	if (buffer.overflow)
	{
		delete(jsonbuf);
		jsonbuf = new char[len + 1];
		buffer.start = jsonbuf;
		buffer.end = jsonbuf + len;
		buffer.cur = jsonbuf;
		buffer.overflow = 0;
		len = JSON_EncodeValue(&buffer, obj);
	}

	if (buffer.overflow)
	{
		str = "";
	}
	else
	{
		jsonbuf[len] = 0;
		str = jsonbuf;
	}
	
    delete(jsonbuf);
    return str.size() != 0;
}

int JSON_DecodeValue(json_buf* buf, DynamicObject& obj)
{
	size_t i;
	json_token token;

	if (!json_gettoken(buf, &token))
	{
		return 0;
	}

	switch (token.type)
	{
	case JSON_INTEGER:
		obj = token.int_value;
		return 1;
	case JSON_NUMBER:
		obj = token.number_value;
		return 1;
	case JSON_STRING:
		obj = token.str_value;
		return 1;
	case JSON_BOOLEAN:
		if (token.bool_value)
			obj = true;
		else
			obj = false;
		return 1;
	case JSON_NULL:
		obj.del();
		return 1;
	case JSON_BEGINOBJECT:
		while (1)
		{
			if (!json_gettoken(buf, &token))
				return 0;
			else if (token.type == JSON_ENDOBJECT)
				break;
			else if (token.type != JSON_STRING)
				return 0;

			DynamicObject& o = obj[(const char*)token.str_value];
			if (!json_gettoken(buf, &token) || token.type != JSON_COLON)
				return 0;

			if (!JSON_DecodeValue(buf, o))
				return 0;

			if (!json_gettoken(buf, &token))
				return 0;
			else if (token.type == JSON_ENDOBJECT)
				break;
			else if (token.type != JSON_COMMA)
				return 0;
		}
		return 1;
	case JSON_BEGINARRAY:
		i = 0;
		while (1)
		{
			if (json_peekch(buf) == JSON_ENDARRAY)
			{
				json_gettoken(buf, &token);
				break;
			}
			
            DynamicObject&o = obj[i++];
			if (!JSON_DecodeValue(buf, o))
				return 0;

			if (!json_gettoken(buf, &token))
				return 0;
			else if (token.type == JSON_ENDARRAY)
				break;
			else if (token.type != JSON_COMMA)
				return 0;
		}
		return 1;
	default:
		return 0;
	}
}

int DynamicObject::JSON_Decode(DynamicObject& obj, const std::string& str)
{
	char * jsonbuf = new char[str.length()];
	memcpy(jsonbuf, str.c_str(), str.length());

	json_buf buf;
	buf.start = jsonbuf;
	buf.end = jsonbuf + str.length();
	buf.cur = jsonbuf;
	buf.overflow = 0;

	if (JSON_DecodeValue(&buf, obj))
	{
		delete jsonbuf;
		return 1;
	}
    else
	{
		delete jsonbuf;
		return 0;
	}
}
