// sudoku.cpp : Defines the entry point for the console application.
//
#include <Windows.h>
#include <stdio.h>
#include <memory.h>

void idct(float *block);
void dct(float *block);

typedef char i8;
typedef short i16;
typedef int i32;
typedef long long i64;
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

u64 getclock()
{
	u64 a;
	u64 b;
	QueryPerformanceCounter((LARGE_INTEGER*)&a);
	QueryPerformanceFrequency((LARGE_INTEGER*)&b);
	return a * 1000000 / b;
}

const static unsigned m_sZigZag[64] =
{
	0, 1, 8,16, 9, 2, 3,10,
	17,24,32,25,18,11, 4, 5,
	12,19,26,33,40,48,41,34,
	27,20,13, 6, 7,14,21,28,
	35,42,49,56,57,50,43,36,
	29,22,15,23,30,37,44,51,
	58,59,52,45,38,31,39,46,
	53,60,61,54,47,55,62,63
};

const static unsigned m_sUnZigZag[64] =
{
	 0, 1, 5, 6,14,15,27,28,
	 2, 4, 7,13,16,26,29,42,
	 3, 8,12,17,25,30,41,43,
	 9,11,18,24,31,40,44,53,
	10,19,23,32,39,45,52,54,
	20,22,33,38,46,51,55,60,
	21,34,37,47,50,56,59,61,
	35,36,48,49,57,58,62,63
};

const static unsigned m_default_dcqt[64] =
{
	16, 11, 10, 16, 24, 40, 51, 61,
	12, 12, 14, 19, 26, 58, 60, 55,
	14, 13, 16, 24, 40, 57, 69, 56,
	14, 17, 22, 29, 51, 87, 80, 62,
	18, 22, 37, 56, 68, 109,103,77,
	24, 35, 55, 64, 81, 104,113,92,
	49, 64, 78, 87, 103,121,120,101,
	72, 92, 95, 98, 112,100,103,99
};

const static unsigned m_default_acqt[64] =
{
	17,18,24,47,99,99,99,99,
	18,21,26,66,99,99,99,99,
	24,26,56,99,99,99,99,99,
	47,66,99,99,99,99,99,99,
	99,99,99,99,99,99,99,99,
	99,99,99,99,99,99,99,99,
	99,99,99,99,99,99,99,99,
	99,99,99,99,99,99,99,99,
};

class JPEGBitStream
{
public:
	u8* buf;
	u8 feed;
	int bytelen;
	int bitoff;
	int byteoff;

	JPEGBitStream(u8* buffer, int length)
	{
		buf = buffer;
		bytelen = length;
		bitoff = -1;
		byteoff = 0;
	}

	bool resetnext()
	{
		if(buf[byteoff] == 0xFF && buf[byteoff+1]>=0xD0 && buf[byteoff+1]<=0xD7)
		{
			byteoff += 2;
			bitoff = -1;
			return true;
		}else
			return false;
	}

	inline i8 readbit()
	{
		if(bitoff<0)
		{
			if(byteoff == bytelen)
				return -1;

			if(buf[byteoff] == 0xFF)
			{
				byteoff++;
				if(buf[byteoff] == 0)
				{
					feed = 0xFF;
				}
				else
				{
					return -1;
				}
			}else
			{
				feed = buf[byteoff];
			}

			byteoff++;
			bitoff = 7;
		}

		i8 bit = (feed>>bitoff)&1;
		bitoff --;
		return bit;
	}

	i32 readint(int len)
	{
		int i;
		i32 r = 0;
		for(i=0;i<len;i++)
		{
			i8 bit = readbit();
			if(bit == -1)
				return -1;

			r = (r<<1) + bit;
		}
		return r;
	}

	void seek(int a, int b)
	{
		byteoff = a;
		bitoff = 7-b;
	}
};

class QuantizationTable
{
public:
	u8 m[64];

	u8* createTable(u8* buf, int len)
	{
		int i;

		if(len != 64)
			return false;
		for(i=0;i<64;i++)
			m[i] = buf[m_sUnZigZag[i]];
		return buf+64;
	}

	void dumpTable()
	{
		int i;
		int j;
		for(j=0;j<8;j++)
		{
			for(i=0;i<8;i++)
				printf("%4d", m[j*8+i]);
			printf("\n");
		}
	}
};

struct HuffmanTableNode
{
	i16 left;
	i16 right;
};

class HuffmanTable
{
public:
	HuffmanTableNode m[1024];
	int allocnext;
	int allocremain;

	HuffmanTable()
	{
		memset(m, 0xff, sizeof(m));
		m[0].left = 1;
		m[0].right = 2;
		allocnext = 1;
		allocremain = 2;
	}

	u8* createTable(u8* buf, int len)
	{
		int i;
		int j;
		u8* n = buf + 16;
		u8* end = buf + len;
		u8 level = 15;

		if (len<16)
			return 0;

		for(i=15;i>=0;i--)
		{
			if(buf[i] == 0)
				level --;
			else
				break;
		}

		for(i=0;i<16;i++)
		{
			if( n + *buf > end || *buf > allocremain)	// buffer overflow or alloc overflow
				return 0;
			if( i > level)
				break;
			if(*buf)
			{
				for(j=0;j<*buf;j++)
				{
					m[allocnext].left = 0;
					m[allocnext].right = *n;
					n++;
					allocnext++;
				}
				allocremain -= *buf;
			}

			for(j=0;j<allocremain;j++)
			{
				m[allocnext + j].left = allocnext + allocremain + j * 2;
				m[allocnext + j].right = allocnext + allocremain + j * 2 + 1;
			}

			allocnext = allocnext + allocremain;
			allocremain *= 2;
			buf++;
		}
		return n;
	}

	i32 read(JPEGBitStream* stream)
	{
		int nodeidx = 0;
		while(1)
		{
			HuffmanTableNode* node = m + nodeidx;
			if(node->left == -1)
				return -1;
			if(node->left == 0)
				return node->right;

			int bit = stream->readbit();
			if(bit <0)
				return -1;
			else 
			if(bit == 0)
				nodeidx = node->left;
			else 
				nodeidx = node->right;
		}
	}

	void dumpTable()
	{
		char prefix[20];
		memset(prefix, 0, 20);
		dumpTable(prefix, 0, 0);
	}

	void dumpTable(char* prefix, int prefixlen, i16 i)
	{
		if(m[i].left == -1)
		{
			return;
		}if(m[i].left == 0)
		{
			printf("  %s = %02X\n", prefix, m[i].right);
		}else
		{
			prefix[prefixlen]='0';
			dumpTable(prefix, prefixlen+1, m[i].left);
			prefix[prefixlen]='1';
			dumpTable(prefix, prefixlen+1, m[i].right);
			prefix[prefixlen]=0;
		}
	}
};

struct ImageComponent
{
	u8 id;
	u8 hsf;
	u8 vsf;
	u8 qtidx;
	u8 dchtidx;
	u8 achtidx;

	i16 prevdc;
	u8 repeat;
	u8 width;
	u8 height;
	u8 xscale;
	u8 yscale;
	u8 xoffset[8];
	u8 yoffset[8];

	u8* qt;
	HuffmanTable* acht;
	HuffmanTable* dcht;
};

class ImageHeader
{
public:
	u8 precision;
	u16 height;
	u16 width;
	u8 component_count;

	u8 mcu_width;
	u8 mcu_height;
	u16 horizontal_mcu_count;
	u16 vertical_mcu_count;
	
	u8 spectral_selection_start;
	u8 spectral_selection_end;
	u8 successive_approximation;

	ImageComponent components[4];

	u32* pixel;
	float* mcu_YCbCr;

	ImageHeader()
	{
		pixel = 0;
		mcu_YCbCr = 0;
	}

	~ImageHeader()
	{
		if(pixel)
			delete[] pixel;
		if(mcu_YCbCr)
			delete[] mcu_YCbCr;
	}

	bool reset()
	{
		int i;
		for(i = 0; i<component_count; i++)
		{
			components[i].prevdc = 0;
		}
		return true;
	}

	bool loadFrame(u8* buf, int len)
	{
		int i;
		int maxhsf;
		int maxvsf;

		if(len<6)
			return false;

		precision = buf[0];
		height = (buf[1]<<8) + buf[2];
		width = (buf[3]<<8) + buf[4];
		component_count = buf[5];

		if(component_count < 1 || component_count > 4 || len < 6 + component_count * 3)
			return false;

		maxhsf = 0;
		maxvsf = 0;
		for(i = 0; i<component_count; i++)
		{
			components[i].id = buf[6 + i*3];
			components[i].hsf = (buf[7 + i*3] >> 4) & 0xF;
			components[i].vsf = buf[7 + i*3] & 0xF;
			components[i].qtidx = buf[8 + i*3];
			components[i].prevdc = 0;
			if(components[i].hsf > maxhsf)
				maxhsf = components[i].hsf;
			if(components[i].vsf > maxvsf)
				maxvsf = components[i].vsf;
		}

		mcu_width = maxhsf * 8;
		horizontal_mcu_count = (width + mcu_width - 1) / mcu_width;

		mcu_height = maxvsf * 8;
		vertical_mcu_count = (height + mcu_height - 1) / mcu_height;

		pixel = new u32[mcu_width*horizontal_mcu_count*mcu_height*vertical_mcu_count];
		mcu_YCbCr = new float[3*mcu_width*mcu_height];

		for(i = 0; i<component_count; i++)
		{
			int h,v;
			components[i].repeat = components[i].hsf * components[i].vsf;
			components[i].width = mcu_width / components[i].hsf;
			components[i].height = mcu_height / components[i].vsf;
			components[i].xscale = maxhsf / components[i].hsf;
			components[i].yscale = maxvsf / components[i].vsf;
			for(v = 0; v<components[i].vsf; v++)
			for(h = 0; h<components[i].hsf; h++)
			{
				components[i].xoffset[v*components[i].hsf + h] = h*components[i].width;
				components[i].yoffset[v*components[i].hsf + h] = v*components[i].height;
			}
		}
		return true;
	}

	bool loadScan(u8* buf, int len)
	{
		int i;

		if(len<4)
			return false;

		if(buf[0] != component_count)
			return false;

		for(i = 0; i<component_count; i++)
		{
			components[i].dchtidx = (buf[2 + i*2] >> 4) & 0xF;
			components[i].achtidx = buf[2 + i*2] & 0xF;
		}

		if(len < 4 + component_count * 2)
			return false;

		spectral_selection_start = buf[1 + component_count * 2];
		spectral_selection_end = buf[2 + component_count * 2];
		successive_approximation = buf[3 + component_count * 2];
		return true;
	}

	void dumpFrame()
	{
		int i;
		printf("  Bits Per Sample = %d\n", precision);
		printf("  Image: width=%d, height=%d\n", width, height);
		printf("  Number of Img components = %d\n", component_count);
		for(i=0;i<component_count;i++)
		{
			printf("    Component[%d]: ID=%d, HSF=%d, VSF=%d, QT Idx=%d\n", i, 
				components[i].id, components[i].hsf, components[i].vsf, components[i].qtidx);
		}
		printf("  MCU: width=%d height=%d\n", mcu_width, mcu_height);
		printf("  MCU Count: horizontal=%d vertical=%d\n", horizontal_mcu_count, vertical_mcu_count);
	}

	void dumpScan()
	{
		int i;
		printf("  Number of Img components = %d\n", component_count);
		for(i=0;i<component_count;i++)
		{
			printf("    Component[%d]: ID=%d, DCHT Idx=%d, ACHT Idx=%d\n", i, 
				components[i].id, components[i].dchtidx, components[i].achtidx);
		}
		printf("  Spectral Selection Start = %d\n", spectral_selection_start);
		printf("  Spectral Selection End = %d\n", spectral_selection_end);
		printf("  Successive Approximation = %d\n", successive_approximation);
	}
};

class JPEGDecode
{
public:

	QuantizationTable qt[2];
	HuffmanTable dcht[2];
	HuffmanTable acht[2];
	ImageHeader fhdr;
	u32 rsti;

	u32 YCbCrToRGB(float *YCbCr)
	{
		i32 R = (i32)(YCbCr[0] + 1.402f * (YCbCr[2]-128));
		i32 G = (i32)(YCbCr[0] - 0.34414f * (YCbCr[1]-128) - 0.71414f * (YCbCr[2]-128));
		i32 B = (i32)(YCbCr[0] + 1.772f * (YCbCr[1]-128));

		if(R<0)
			R=0;
		if(R>255)
			R=255;
		if(G<0)
			G=0;
		if(G>255)
			G=255;
		if(B<0)
			B=0;
		if(B>255)
			B=255;

		return (R<<16) + (G<<8) + B;
	}

	void RGBToYCbCr(float *YCbCr, u8* rgb)
	{
		float B = rgb[0];
		float G = rgb[1];
		float R = rgb[2];

		YCbCr[0] = 0 + 0.299f*R + 0.587f*G + 0.114f*B;
		YCbCr[1] = 128 - 0.168736f*R - 0.331264f*G + 0.5f*B;
		YCbCr[2] = 128 + 0.5f*R - 0.418688f*G - 0.081312f*B;
	}

	i32 jpeg_coefficient_value(int len, i32 v)
	{
		if(len == 0)
			return 0;
		if(v < (1<<(len-1)))
			return  1 + v -(1<<len);
		else
			return v;
	}

	bool read_dc_value(float* mat, ImageComponent* com, JPEGBitStream *bitstream)
	{
		i32 len = com->dcht->read(bitstream);
		if(len == -1)
			return false;
		i32 v = bitstream->readint(len);
		if(v == -1)
			return false;

		i32 value = jpeg_coefficient_value(len, v) + com->prevdc;
		mat[0] = (float)(value * com->qt[0]);
		com->prevdc = value;
		return true;
	}

	bool read_ac_value(float* mat, ImageComponent* com, JPEGBitStream *bitstream)
	{
		int i;
		int idx = 1;
		for(i=1;i<64;i++)
			mat[i] = 0.0f;

		while(idx<64)
		{
			i32 c = com->acht->read(bitstream);
			if(c == -1)
				return false;
			if(c == 0) // EOB
				break;

			i32 len = c & 0xF;
			i32 zerorun = (c >> 4) & 0xF;
			i32 v = bitstream->readint(len);

			if(v == -1 || zerorun + idx +1 > 64)
				return false;

			for(i=0; i<zerorun; i++)
				mat[m_sZigZag[idx++]] = 0.0f;
			mat[m_sZigZag[idx]] = (float)jpeg_coefficient_value(len, v) * com->qt[m_sZigZag[idx]];
			idx ++;
		}
		return true;
	}

	void dumpmat(float* mat)
	{
		int i;
		int j;
		for(j=0;j<8;j++)
		{
			for(i=0;i<8;i++)
				printf("%8.2f", mat[j*8+i]);
			printf("\n");
		}
	}

	bool process_mcu(int x, int y, JPEGBitStream *bitstream)
	{
		int i,j;
		int v, h;
		int xscale, yscale;
		float mat[64];
		float* YCbCr = fhdr.mcu_YCbCr;


		for(i=0;i<fhdr.component_count;i++)
		{
			ImageComponent* com = fhdr.components + i;
			for(j = 0; j<com->repeat; j++)
			{
//				printf("mcu(%d,%d) comid(%d,%d), offset(%4x.%d)\n", x, y, com->id, j, bitstream->byteoff, 7 - bitstream->bitoff);
				if(!read_dc_value(mat, com, bitstream))
				{
					printf("failed to process mcu (%d,%d)\n", x, y);
					return false;
				}
				if(!read_ac_value(mat, com, bitstream))
				{
					printf("failed to process mcu (%d,%d)\n", x, y);
					return false;
				}

//				dumpmat(mat);
				idct(mat);
				


				for(v = 0; v<8; v++)
				for(h = 0; h<8; h++)
				{
					for(yscale = 0; yscale<com->yscale; yscale++)
					for(xscale = 0; xscale<com->xscale; xscale++)
					{
						int tx = com->xoffset[j] + h*com->xscale + xscale;
						int ty = com->yoffset[j] + v*com->yscale + yscale;
						int idx = 3*(tx+ty*fhdr.mcu_width) + i;
						YCbCr[idx] = mat[v*8+h] + 128.0f;
					}
				}
			}
		}
		for(j=0;j<fhdr.mcu_height;j++)
		for(i=0;i<fhdr.mcu_width;i++)
		{
			u32 rgb = YCbCrToRGB(YCbCr + (j*fhdr.mcu_width+i)*3);
			int tx = x*fhdr.mcu_width + i;
			int ty = y*fhdr.mcu_height + j;
			int line = fhdr.mcu_width*fhdr.horizontal_mcu_count;
			fhdr.pixel[ty*line+tx] = rgb;
		}
		return true;
	}

	u8* process_data(u8* m, int len)
	{
		int i,j;
		int rst = 0;
		JPEGBitStream bitstream(m, len);
		for(j=0;j<fhdr.vertical_mcu_count;j++)
		{
			for(i=0;i<fhdr.horizontal_mcu_count;i++)
			{
				process_mcu(i, j, &bitstream);
				rst ++;
				if(rst == rsti)
				{
					rst = 0;
					bitstream.resetnext();
					fhdr.reset();
				}
			}
		}
		return m;
	}

	u8* process_DQT(u8* m, int len)
	{
		int i;
		if(len != 0x41 && len != 0x82)
			return 0;

		for(i = 0; i < len / 0x41; i++)
		{
			u8 idx = *(m + i * 0x41);
			if(idx != 0 && idx != 1)
				return 0;
			qt[idx].createTable(m + i * 0x41 + 1, 64);
			printf("ID=%d\n", idx);
			qt[idx].dumpTable();
		}
		return m + len;
	}

	u8* process_DHT(u8* m, int len)
	{
		u8 *end = m + len;
		while(m < end)
		{
			u8* r;
			u8 idx = *m & 0xF;
			u8 cls = ((*m) >> 4) & 0xF;
			if( idx != 0 && idx != 1 || cls != 0 && cls != 1)
				return 0;

			if(cls == 0x00)
			{
				printf("ID=%d, Class=0 (DC Table)\n", idx);
				r = dcht[idx].createTable(m+1, len-1);
				dcht[idx].dumpTable();
			}else 
			{
				printf("ID=%d, Class=1 (AC Table)\n", idx);
				r = acht[idx].createTable(m+1, len-1);
				acht[idx].dumpTable();
			}
			if(r == 0)
				return 0;
			m = r;
		}
		return m;
	}

	u8* process_marker(u8* m, u32 limit)
	{
		if(*m++ != 0xFF)
			return 0;
		u8 marker = *m++;

		if(marker == 0xD8)	// SOI Start of Image
		{
			printf("*** Marker: %02X SOI, Length: 0 ***\n", *m);
			return m;
		}else if(marker == 0xD9)	// EOI End of Image
		{
			printf("*** Marker: %02X EOI, Length: 0 ***\n", *m);
			return 0;
		}
		
		u16 len = (m[0]<<8) + m[1];

		if(marker == 0xC0)	// SOF0 Start of Frame 0
		{
			printf("*** Marker: %02X SOF0 (Baseline), Length: %d ***\n", marker, len);
			fhdr.loadFrame(m+2, len-2);
			fhdr.dumpFrame();
		}else if(marker == 0xC1)	// SOF1 Start of Frame 1
		{
			printf("*** Marker: %02X SOF1 (Extended Sequential), Length: %d ***\n", marker, len);			
		}else if(marker == 0xC2)	// SOF2 Start of Frame 2
		{
			printf("*** Marker: %02X SOF2 (Progressive), Length: %d ***\n", marker, len);
			
		}else if(marker == 0xC3)	// SOF3 Start of Frame 3
		{
			printf("*** Marker: %02X SOF3 (Lossless), Length: %d ***\n", marker, len);
			
		}else if(marker == 0xC4)	// DHT Define Huffman Table
		{
			printf("*** Marker: %02X DHT, Length: %d ***\n", marker, len);
			process_DHT(m+2, len-2);
		}else if(marker == 0xDD)	// DRI Define Restart Interval
		{
			printf("*** Marker: %02X DRI, Length: 4 ***\n", *m);
			rsti = (m[2]<<8) + m[3];
		}else if(marker == 0xDA)	// SOS Start of Scan
		{
			printf("*** Marker: %02X SOS, Length: %d ***\n", marker, len);
			fhdr.loadScan(m+2, len-2);
			fhdr.dumpScan();

			for(int i = 0; i<fhdr.component_count; i++)
			{
				fhdr.components[i].qt = qt[fhdr.components[i].qtidx].m;
				fhdr.components[i].acht = acht+fhdr.components[i].achtidx;
				fhdr.components[i].dcht = dcht+fhdr.components[i].dchtidx;
			}

			return process_data(m+len, limit-len);
		}else if(marker == 0xDB)	// DQT Define Quantization Table
		{
			printf("*** Marker: %02X DQT, Length: %d ***\n", marker, len);
			process_DQT(m+2, len-2);
		}else if(marker >= 0xE0 && marker <= 0xEF) //APPn Application-specific
		{
			printf("*** Marker: %02X APPn, Length: %d ***\n", marker, len);
			
		}else if(marker== 0xFE)	// Comment
		{
			printf("*** Marker: %02X COM, Length: %d ***\n", marker, len);
		}
		m+=len;
		return m;
	}

	void load(const char* filename)
	{
		int size;
		u8 * jpegbuf;

		FILE* fp = fopen(filename, "rb");
		fseek(fp, 0, SEEK_END);
		size = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		jpegbuf = new u8[size];
		fread(jpegbuf, 1, size, fp);
		fclose(fp);

		rsti = 0xFFFFFFFF;

		u8 * m = jpegbuf;
		while(m)
		{
			u64 starttick = getclock();
			m = process_marker(m, size - (jpegbuf - m));
			printf("--- timespan %lld ---\n", getclock() - starttick);
		}

		delete[] jpegbuf;
	}

	void savebmp(const char* filename)
	{
		int i,j;
		BITMAPFILEHEADER fileheader;
		BITMAPINFOHEADER infoheader;
		ZeroMemory(&fileheader, sizeof(fileheader));
		ZeroMemory(&infoheader, sizeof(infoheader));


		int imagewidth = fhdr.mcu_width * fhdr.horizontal_mcu_count;
		int imageheight = fhdr.mcu_height * fhdr.vertical_mcu_count;

		fileheader.bfType = 'MB';
		fileheader.bfSize = fileheader.bfOffBits + imagewidth * imageheight * 3;
		fileheader.bfOffBits = sizeof(fileheader) + sizeof(infoheader);

		infoheader.biSize = sizeof(infoheader);
		infoheader.biWidth = imagewidth;
		infoheader.biHeight = imageheight;
		infoheader.biPlanes = 1;
		infoheader.biBitCount = 24;
		infoheader.biSizeImage = imagewidth * imageheight * 3;

		FILE* fp = fopen(filename, "wb");
		fwrite(&fileheader, sizeof(fileheader), 1, fp);
		fwrite(&infoheader, sizeof(infoheader), 1, fp);

		for(j=imageheight-1;j>=0;j--)
			for(i=0;i<imagewidth;i++)
				fwrite(fhdr.pixel + j * imagewidth + i, 3, 1, fp);

		fclose(fp);
	}
};
