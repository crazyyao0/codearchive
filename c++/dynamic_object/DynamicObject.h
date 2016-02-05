// DynamicObject.h -
//

#pragma once

#include <string>
#include <vector>
#include <map>
#include <string.h>

#define DO_UNKNOWN 0	// null
#define DO_INTEGER 1	// i64
#define DO_BOOLEAN 2	// bool
#define DO_NUMBER  3	// double
#define DO_STRING  4	// std::string
#define DO_ARRAY   5	// std::vector
#define DO_OBJECT  6	// std::map

#define TY_INTEGER32 int
#define TY_INTEGER long long
#define TY_BOOLEAN bool
#define TY_NUMBER  double
#define TY_STRING  std::string
#define TY_ARRAY   std::vector<DynamicObject>
#define TY_OBJECT  std::map<std::string, DynamicObject>

namespace Utilities
{

///////////////////////////////////////////////////////////////////////////////
// DynamicObject class -
class DynamicObject
{
public:
	void* value;
	int type;
	DynamicObject() : value(0), type(DO_UNKNOWN)
	{

	}

	DynamicObject(TY_INTEGER x) : type(DO_INTEGER)
	{
		value = new TY_INTEGER(x);
	}

	DynamicObject(int x) : type(DO_INTEGER)
	{
		value = new TY_INTEGER(x);
	}

	DynamicObject(long x) : type(DO_INTEGER)
	{
		value = new TY_INTEGER(x);
	}

	DynamicObject(unsigned int x) : type(DO_INTEGER)
	{
		value = new TY_INTEGER(x);
	}

	DynamicObject(unsigned long x) : type(DO_INTEGER)
	{
		value = new TY_INTEGER(x);
	}

	DynamicObject(TY_BOOLEAN x) : type(DO_BOOLEAN)
	{
		value = new TY_BOOLEAN(x);
	}

	DynamicObject(TY_NUMBER x) : type(DO_NUMBER)
	{
		value = new TY_NUMBER(x);
	}

	DynamicObject(const TY_STRING& x) : type(DO_STRING)
	{
		value = new TY_STRING(x);
	}

	DynamicObject(const TY_ARRAY& x) : type(DO_ARRAY)
	{
		value = new TY_ARRAY(x);
	}

	DynamicObject(const TY_OBJECT& x) : type(DO_OBJECT)
	{
		value = new TY_OBJECT(x);
	}

	DynamicObject(const DynamicObject& o)
	{
		value = o.copyValue();
		type = o.type;
	}

	~DynamicObject()
	{
		del();
	}

	void* copyValue() const
	{
		switch (type)
		{
		case DO_INTEGER:
			return new TY_INTEGER(*(TY_INTEGER *)value);
		case DO_BOOLEAN:
			return new TY_BOOLEAN(*(TY_BOOLEAN *)value);
		case DO_NUMBER:
			return new TY_NUMBER(*(TY_NUMBER *)value);
		case DO_STRING:
			return new TY_STRING(*(TY_STRING *)value);
		case DO_ARRAY:
			return new TY_ARRAY(*(TY_ARRAY *)value);
		case DO_OBJECT:
			return new TY_OBJECT(*(TY_OBJECT*)value);
		}

		return 0;
	}

	void del()
	{
		switch (type)
		{
		case DO_INTEGER:
			delete (TY_INTEGER *)value;
			break;
		case DO_BOOLEAN:
			delete (TY_BOOLEAN *)value;
			break;
		case DO_NUMBER:
			delete (TY_NUMBER *)value;
			break;
		case DO_STRING:
			delete (TY_STRING *)value;
			break;
		case DO_ARRAY:
			delete (TY_ARRAY *)value;
			break;
		case DO_OBJECT:
			delete (TY_OBJECT *)value;
			break;
		}

		value = 0;
		type = DO_UNKNOWN;
	}

	DynamicObject& operator = (const DynamicObject& o)
	{
		del();
		value = o.copyValue();
		type = o.type;
		return *this;
	}

	DynamicObject& operator = (TY_INTEGER x)
	{
		del();
		type = DO_INTEGER;
		value = new TY_INTEGER(x);
		return *this;
	}

	DynamicObject& operator = (int x)
	{
		del();
		type = DO_INTEGER;
		value = new TY_INTEGER(x);
		return *this;
	}

	DynamicObject& operator = (unsigned int x)
	{
		del();
		type = DO_INTEGER;
		value = new TY_INTEGER(x);
		return *this;
	}

	DynamicObject& operator = (long x)
	{
		del();
		type = DO_INTEGER;
		value = new TY_INTEGER(x);
		return *this;
	}

	DynamicObject& operator = (unsigned long x)
	{
		del();
		type = DO_INTEGER;
		value = new TY_INTEGER(x);
		return *this;
	}

	DynamicObject& operator = (TY_BOOLEAN x)
	{
		del();
		type = DO_BOOLEAN;
		value = new TY_BOOLEAN(x);
		return *this;
	}

	DynamicObject& operator = (TY_NUMBER x)
	{
		del();
		type = DO_NUMBER;
		value = new TY_NUMBER(x);
		return *this;
	}

	DynamicObject& operator = (const TY_STRING& x)
	{
		del();
		type = DO_STRING;
		value = new TY_STRING(x);
		return *this;
	}

	DynamicObject& operator = (const char* x)
	{
		del();
		type = DO_STRING;
		value = new TY_STRING(x);
		return *this;
	}

	DynamicObject& operator = (TY_ARRAY& x)
	{
		del();
		type = DO_ARRAY;
		value = new TY_ARRAY(x);
		return *this;
	}

	DynamicObject& operator = (TY_OBJECT& x)
	{
		del();
		type = DO_OBJECT;
		value = new TY_OBJECT(x);
		return *this;
	}

    operator TY_INTEGER32(void)
    {
        if (type == DO_INTEGER)
        {
            return static_cast<TY_INTEGER32>(*(TY_INTEGER*)value);
        }
        else if (type == DO_BOOLEAN)
        {
            return *(TY_BOOLEAN*)value ? 1 : 0;
        }
        else
        {
            throw "Type mismatch";
        }
    }

    operator TY_INTEGER(void)
    {
        if (type == DO_INTEGER)
        {
            return *(TY_INTEGER*)value;
        }
        else if (type == DO_BOOLEAN)
        {
            return *(TY_BOOLEAN*)value ? 1 : 0;
        }
        else
        {
            throw "Type mismatch";
        }
    }

    operator TY_BOOLEAN(void)
    {
        if (type == DO_BOOLEAN)
        {
            return *(TY_BOOLEAN*)value;
        }
        else if (type == DO_INTEGER)
        {
            return *(TY_INTEGER*)value != 0;
        }
        else if (type == DO_NUMBER)
        {
            return *(TY_NUMBER*)value != 0;
        }
        else
        {
            throw "Type mismatch";
        }
    }

    operator TY_NUMBER(void)
    {
        if (type == DO_NUMBER)
        {
            return *(TY_NUMBER*)value;
        }
        else if (type == DO_BOOLEAN)
        {
            return *(TY_BOOLEAN*)value ? 1.0 : 0.0;
        }
        else if (type == DO_INTEGER)
        {
            return static_cast<TY_NUMBER>(*(TY_INTEGER*)value);
        }
        else
        {
            throw "Type mismatch";
        }
    }

    operator TY_STRING&(void)
    {
        if (type == DO_STRING)
        {
            return *(TY_STRING*)value;
        }
        else
        {
            throw "Type mismatch";
        }
    }

    operator char* (void)
    {
        if (type == DO_STRING)
        {
            TY_STRING& str = *(TY_STRING*)value;
            return const_cast<char*>(str.c_str());
        }
        else
        {
            throw "Type mismatch";
        }
    }

	void asArray(void)
	{
        del();
        type = DO_ARRAY;
        value = new TY_ARRAY();
	}

	void asObject(void)
	{
		del();
        type = DO_OBJECT;
        value = new TY_OBJECT();
	}

    DynamicObject& operator [] (const char* name)
    {
        if (type == DO_UNKNOWN)
		{
			type = DO_OBJECT;
			value = new TY_OBJECT();
		}

        if (type != DO_OBJECT)
        {
            throw "Type mismatch";
        }

        return (*(TY_OBJECT *)value)[name];
    }

	DynamicObject& operator [] (const std::string& name)
	{
        return operator [] (name.c_str());
	}
	
	DynamicObject& operator [] (size_t index)
	{
		if (type == DO_UNKNOWN)
        {
            type = DO_ARRAY;
            value = new TY_ARRAY(index + 1);
        }

        if (type != DO_ARRAY)
        {
            throw "Type mismatch";
        }

        TY_ARRAY& arr = *(TY_ARRAY*)value;
        if (arr.size() < (index + 1))
        {
            arr.resize(index + 1);
        }

        return arr[index];
	}

    size_t size(void)
    {
        if (type == DO_UNKNOWN)
        {
            type = DO_ARRAY;
            value = new TY_ARRAY();
        }

        if (type != DO_ARRAY)
        {
            throw "Type mismatch";
        }

        TY_ARRAY& arr = *(TY_ARRAY*)value;
        return arr.size();
    }

	size_t propertyList(std::vector<std::string>& keys) const
	{
		TY_OBJECT::iterator it;
		TY_OBJECT& m = *(TY_OBJECT *)value;
		for (it = m.begin(); it != m.end(); it++)
			keys.push_back(it->first);
		return keys.size();
	}

	static bool JSON_Encode(DynamicObject& obj, std::string& str);
	static int JSON_Decode(DynamicObject& obj, const std::string& str);
};

}
