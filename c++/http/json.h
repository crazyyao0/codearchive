#define JSON_ERROR		0
#define JSON_INTEGER	1
#define JSON_NUMBER		2
#define JSON_STRING		3
#define JSON_BOOLEAN	4
#define JSON_NULL		5
#define JSON_COLON		':'
#define JSON_COMMA		','
#define JSON_BEGINOBJECT	'{'
#define JSON_ENDOBJECT		'}'
#define JSON_BEGINARRAY		'['
#define JSON_ENDARRAY		']'


typedef struct json_buf
{
	char* start;
	char* cur;
	char* end;
	int overflow;
}json_buf;

typedef struct json_token
{
	int type;
	int len;
	union{
		char* str_value;
		long long int_value;
		double number_value;
		int bool_value;
	};
}json_token;

char json_getch(json_buf* buf);
char json_peekch(json_buf* buf);
int json_gettoken(json_buf* buf, json_token* tag);
int json_putch(json_buf* buf, char ch);
int json_put(json_buf* buf, void* var, int len);
int json_write_null(json_buf* buf);
int json_write_bool(json_buf* buf, int b);
int json_write_number(json_buf* buf, double d);
int json_write_integer(json_buf* buf, long long d);
int json_write_string(json_buf* buf, char* s);