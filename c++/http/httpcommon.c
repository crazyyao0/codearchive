#include "httpcommon.h"
#include "socket.h"


//js_escape			!#$&'()*+,-./:;=?@_~	space=%20
//firefox_address	!"'()*-.<>[\]^_`{|}~	space=+
//chrome_address	"*-.<>_~		space=+
//ie_address		*-.@_			space=+
//firefox_sent		*-._			space=+
//chrome_sent		*-._			space=+
//ie_sent			*-.@_			space=+
static char escape_table[] = 
//                   1               2               3               4               5               6               7
//   0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF
//	                                  !"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~
	"00000000000000000000000000000000000000000010011011111111110000000111111111111111111111111110000101111111111111111111111111100000";

int http_decode_url(char* src, int len, char* out)
{
	int outlen = 0;
	char* end = src + len;
	int a;
	int b;

	while(src<end)
	{
		if(*src == '%')
		{
			if(src + 2 >= end)
				return 0;

			a = src[1];
			b = src[2];

			if(a>='a' && a<='f')
				a = a-'a'+10;
			else if(a>='A' && a<='F')
				a = a-'A'+10;
			else if(a>='0' && a<='9')
				a = a-'0';
			else
				return 0;

			if(b>='a' && b<='f')
				b = b-'a'+10;
			else if(b>='A' && b<='F')
				b = b-'A'+10;
			else if(b>='0' && b<='9')
				b = b-'0';
			else
				return 0;
			out[outlen] = (char)(a * 16 + b);
			src += 3;
			outlen ++;
		}else if(*src == '+')
		{
			out[outlen] = ' ';
			src ++;
			outlen ++;
		}
		else
		{
			out[outlen] = *src;
			src ++;
			outlen ++;
		}
	}
	return outlen;
}

int http_encode_url(char* src, int len, char* out)
{
	int outlen = 0;
	char* end = src + len;
	while(src < end)
	{
		if(*src > 0 && escape_table[*src] == '1')
		{
			out[outlen] = *src;
			outlen ++;
			src ++;
		}else if(*src == ' ')
		{
			out[outlen] = '+';
			outlen ++;
			src ++;
		}else
		{
			int a, b;
			out[outlen] = '%';
			a = (*src >> 4) & 0xF;
			b = *src & 0xF;
			if(a>=10)
				out[outlen + 1] = a + 'A' - 10;
			else
				out[outlen + 1] = a + '0';
			
			if(b>=10)
				out[outlen + 2] = b + 'A' - 10;
			else
				out[outlen + 2] = b + '0';

			src ++;
			outlen += 3;
		}
	}
	return outlen;
}

int http_parse_urlencoded(dynamic_object* parameter, char* buf)
{
	char keystr[64];
	char *a;
	char *b = buf;
	TRexMatch key;
	TRexMatch value;

	do
	{
		if(trex_search(g_rex_url_encoded, b, &a, &b) == 0)
			return 0;

		trex_getsubexp(g_rex_url_encoded, 1, &key);
		trex_getsubexp(g_rex_url_encoded, 2, &value);
		if(key.len<64)
		{
			memcpy(keystr, key.begin, key.len);
			keystr[key.len] = 0;

			if(value.len > 1)
			{
				dynamic_set_nstring(dynamic_map_insert(parameter, keystr), (char*)value.begin + 1, value.len - 1);
			}else
			{
				dynamic_set_string(dynamic_map_insert(parameter, keystr), "");
			}
		}
	}while(1);
	return dynamic_size(parameter);
}

TRex *g_rex_url;
TRex *g_rex_reqhdr;
TRex *g_rex_reshdr;
TRex *g_rex_httphdr;
TRex *g_rex_url_encoded;

void http_init()
{
	const char *error = 0;
	init_socket();
	g_rex_url = trex_compile("^([^:/]+://)?([^:/]+)(:\\d+)?(/.*)?", &error);
	g_rex_reqhdr = trex_compile("^([^ ]+) (/[^?# ]*)(\\?[^# ]*)?(#[^ ]*)? HTTP/(1.\\d)\\r\\n", &error);
	g_rex_reshdr = trex_compile("^HTTP/1.\\d (\\d+) (.+)\\r\\n", &error);
	g_rex_httphdr = trex_compile("^([^: ]+): (.+)\\r\\n", &error);
	g_rex_url_encoded = trex_compile("^([^=&]+)(=[^&]+)?&?", &error);
}

static const char *monthstr[]={"Jan ", "Feb ", "Mar ", "Apr ", "May ", "Jun ", "Jul ", "Aug ", "Sep ", "Oct ", "Nov ", "Dec "};
static const char *weekstr[]={"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

// need 30 bytes length
//           1         2
// 012345678901234567890123456789
// Sun, 06 Nov 1994 08:49:37 GMT  ; RFC 822, updated by RFC 1123, 29 bytes long
// Sunday, 06-Nov-94 08:49:37 GMT ; RFC 850, obsoleted by RFC 1036, 30-33 bytes long
// Sun Nov  6 08:49:37 1994       ; ANSI C's asctime() format, 24 bytes long

time_t http_parse_time(char* str)
{
	int i;
	struct tm t;
	int len = strlen(str);

	if(len == 29)
	{
		t.tm_year = atol(str + 12) - 1900;
		t.tm_mon = 0;
		for(i=0;i<12;i++)
		{
			if(*(unsigned int*)(str+8) == *(unsigned int*)monthstr[i])
			{
				t.tm_mon = i;
				break;
			}
		}
		t.tm_mday = atol(str + 5);

		t.tm_hour = atol(str + 17);
		t.tm_min = atol(str + 20);
		t.tm_sec = atol(str + 23);
		return mktime(&t);
	}else if(len == 24)
	{
		t.tm_year = atol(str + 20) - 1900;
		t.tm_mon = 0;
		for(i=0;i<12;i++)
		{
			if(*(unsigned int*)(str+4) == *(unsigned int*)monthstr[i])
			{
				t.tm_mon = i;
				break;
			}
		}
		t.tm_mday = atol(str + 8);

		t.tm_hour = atol(str + 11);
		t.tm_min = atol(str + 14);
		t.tm_sec = atol(str + 17);
		return mktime(&t);
	}else if(len >=30 && len<=33)
	{
		str = strchr(str, ',');
		if(str == 0)
			return 0;

		t.tm_year = atol(str + 9);
		t.tm_mon = 0;
		for(i=0;i<12;i++)
		{
			if(*(unsigned int*)(str+5) == *(unsigned int*)monthstr[i])
			{
				t.tm_mon = i;
				break;
			}
		}
		t.tm_mday = atol(str + 2);

		t.tm_hour = atol(str + 12);
		t.tm_min = atol(str + 15);
		t.tm_sec = atol(str + 18);
		return mktime(&t);
	}else
	{
		return 0;
	}
}

int http_format_time(time_t tick, char* str)
{
	struct tm* t = gmtime(&tick);
	return sprintf(str, "%s, %02d %s%d %02d:%02d:%02d GMT", weekstr[t->tm_wday], 
		t->tm_mday, monthstr[t->tm_mon], t->tm_year + 1900, 
		t->tm_hour, t->tm_min, t->tm_sec);
}