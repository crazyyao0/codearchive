#include <stdio.h>
#include <memory.h>

void aes_dump_block(unsigned char* block, int size)
{
	for (int i = 0; i < size; i++)
		printf("%02x ", block[i]);
	printf("\n");
}

// Logarithm table and reverse table
unsigned char ALOG[256];
unsigned char LOG[256];

// sub bytes table
unsigned char S[256];
unsigned char Si[256];

// multiply table
unsigned char T2[256];
unsigned char T3[256];
unsigned char T9[256];
unsigned char T11[256];
unsigned char T13[256];
unsigned char T14[256];

// round key const
unsigned char Rcon[16];

// shift and unshift table
unsigned char Shift[16] = { 0,5,10,15,4,9,14,3,8,13,2,7,12,1,6,11 };
unsigned char Unshift[16] = { 0 };

// mix column matrix
unsigned char M[16] = { 2,3,1,1,
						1,2,3,1,
						1,1,2,3,
						3,1,1,2 };
unsigned char Mi[16];

unsigned char mul(unsigned char a, unsigned char b)
{
	if (a == 0 || b == 0)
		return 0;
	return ALOG[(LOG[a & 0xFF] + LOG[b & 0xFF]) % 255];
}

unsigned char inv(unsigned char a)
{
	if (a < 2)
		return a;
	return ALOG[255 - LOG[a]];
}

void inverse_gfmat4(unsigned char *in, unsigned char* out)
{
	unsigned char m[32] = { 0 };
	for (int y = 0; y < 4; y++)
	{
		for (int x = 0; x < 4; x++)
			m[y * 8 + x] = in[y * 4 + x];
		m[y * 8 + 4 + y] = 1;
	}
	for (int y0 = 0; y0 < 4; y0++)
	{
		for (int y = 0; y < 4; y++)
		{
			if (y == y0) continue;
			unsigned char f = mul(m[y * 8 + y0], inv(m[y0 * 8 + y0]));
			for (int x = 0; x < 8; x++)
				m[y * 8 + x] = m[y * 8 + x] ^ mul(f, m[y0 * 8 + x]);
		}
	}
	for (int y = 0; y < 4; y++)
		for (int x = 0; x < 4; x++)
			out[y * 4 + x] = mul(m[y * 8 + 4 + x], inv(m[y * 8 + y]));
}

void init_aes_consts()
{
	// calculate LOG/ALOG table for mul and div operation on GF(256)
	ALOG[0] = 1;
	for (int i = 0; i < 255; i++)
	{
		int j = (ALOG[i] << 1) ^ ALOG[i];
		if (j >= 256)
			j ^= 0x11B;
		ALOG[i + 1] = (unsigned char)j;
	}

	LOG[1] = 0;
	for (int i = 1; i < 255; i++)
		LOG[ALOG[i]] = i;

	// calculate lookup table for mixcolumns
	inverse_gfmat4(M, Mi);
	for (int i = 0; i < 16; i += 4)
		printf("%3d %3d %3d %3d\n", Mi[i + 0], Mi[i + 1], Mi[i + 2], Mi[i + 3]);
	for (int i = 0; i < 256; i++)
	{
		T2[i] = mul(2, i);
		T3[i] = mul(3, i);
		T9[i] = mul(9, i);
		T11[i] = mul(11, i);
		T13[i] = mul(13, i);
		T14[i] = mul(14, i);
	}

	// calculate subbytes table and inverse subbytes table
	unsigned char MSA[] = {
		1,0,0,0,1,1,1,1,
		1,1,0,0,0,1,1,1,
		1,1,1,0,0,0,1,1,
		1,1,1,1,0,0,0,1,
		1,1,1,1,1,0,0,0,
		0,1,1,1,1,1,0,0,
		0,0,1,1,1,1,1,0,
		0,0,0,1,1,1,1,1
	};
	unsigned char MSB[] = { 1,1,0,0,0,1,1,0 };
	for (int x = 0; x < 256; x++)
	{
		unsigned char xb[8] = { 0 };
		unsigned char yb[8] = { 0 };
		unsigned char xi = inv(x);
		for (int i = 0; i < 8; i++)
			xb[i] = (xi >> i) & 1;
		for (int j = 0; j < 8; j++)
		{
			for (int i = 0; i < 8; i++)
				yb[j] ^= xb[i] & MSA[j * 8 + i];
			yb[j] ^= MSB[j];
		}
		int y = 0;
		for (int i = 0; i < 8; i++)
			y += (yb[i] << i);
		S[x] = y;
		Si[y] = x;
	}

	// calculate key schedule round constants
	Rcon[0] = 1;
	for (int i = 1; i < 16; i++)
	{
		int j = Rcon[i-1] << 1;
		Rcon[i] = j >= 256 ? (unsigned char)j ^ 0x11B : (unsigned char)j;
	}

	// calculate Unshift table
	for (int i = 0; i < 16; i++)
		Unshift[Shift[i]] = i;
}

unsigned char round_keys[16 * 11];
void init_aes_key(unsigned char * key)
{
	memcpy(round_keys, key, 16);
	aes_dump_block(round_keys, 16);
	unsigned char* prev = round_keys;
	unsigned char* next = round_keys + 16;
	for (int i = 0; i < 10; i++)
	{
		next[0] = S[prev[13]] ^ prev[0] ^ Rcon[i];
		next[1] = S[prev[14]] ^ prev[1];
		next[2] = S[prev[15]] ^ prev[2];
		next[3] = S[prev[12]] ^ prev[3];
		next[4] = next[0] ^ prev[4];
		next[5] = next[1] ^ prev[5];
		next[6] = next[2] ^ prev[6];
		next[7] = next[3] ^ prev[7];
		next[8] = next[4] ^ prev[8];
		next[9] = next[5] ^ prev[9];
		next[10] = next[6] ^ prev[10];
		next[11] = next[7] ^ prev[11];
		next[12] = next[8] ^ prev[12];
		next[13] = next[9] ^ prev[13];
		next[14] = next[10] ^ prev[14];
		next[15] = next[11] ^ prev[15];

		aes_dump_block(next, 16);
		prev = next;
		next = next + 16;
	}
}

void aes_block_encrypt(unsigned char* in, unsigned char* out)
{
	unsigned char a[16];
	unsigned char b[16];
	// round 0
	for (int i = 0; i < 16; i++)
		a[i] = in[i] ^ round_keys[i];

	// round 1 - round 9
	for (int i = 1; i < 10; i++)
	{
		for (int j = 0; j < 16; j++)
			b[j] = S[a[Shift[j]]];
		for (int j = 0; j < 16; j += 4)
		{
			// | 2, 3, 1, 1 |
			// | 1, 2, 3, 1 |
			// | 1, 1, 2, 3 |
			// | 3, 1, 1, 2 |
			a[j + 0] = T2[b[j + 0]] ^ T3[b[j + 1]] ^ b[j + 2] ^ b[j + 3] ^ round_keys[i * 16 + j + 0];
			a[j + 1] = b[j + 0] ^ T2[b[j + 1]] ^ T3[b[j + 2]] ^ b[j + 3] ^ round_keys[i * 16 + j + 1];
			a[j + 2] = b[j + 0] ^ b[j + 1] ^ T2[b[j + 2]] ^ T3[b[j + 3]] ^ round_keys[i * 16 + j + 2];
			a[j + 3] = T3[b[j + 0]] ^ b[j + 1] ^ b[j + 2] ^ T2[b[j + 3]] ^ round_keys[i * 16 + j + 3];
		}
	}

	// round 10
	for (int j = 0; j < 16; j++)
		out[j] = S[a[Shift[j]]] ^ round_keys[160 + j];
	aes_dump_block(out, 16);
}

void aes_block_decrypt(unsigned char* in, unsigned char* out)
{
	unsigned char a[16];
	unsigned char b[16];
	// round 10
	for (int j = 0; j < 16; j++)
		a[j] = Si[in[Unshift[j]] ^ round_keys[160 + Unshift[j]]];
	// round 9 - round 1
	for (int i = 9; i > 0; i--)
	{
		for (int j = 0; j < 16; j++)
			a[j] = a[j] ^ round_keys[i * 16 + j];
		for (int j = 0; j < 16; j += 4)
		{
			// | 14, 11, 13,  9 |
			// |  9, 14, 11, 13 |
			// | 13,  9, 14, 11 |
			// | 11, 13,  9, 14 |
			b[j + 0] = T14[a[j + 0]] ^ T11[a[j + 1]] ^ T13[a[j + 2]] ^ T9[a[j + 3]];
			b[j + 1] = T9[a[j + 0]] ^ T14[a[j + 1]] ^ T11[a[j + 2]] ^ T13[a[j + 3]];
			b[j + 2] = T13[a[j + 0]] ^ T9[a[j + 1]] ^ T14[a[j + 2]] ^ T11[a[j + 3]];
			b[j + 3] = T11[a[j + 0]] ^ T13[a[j + 1]] ^ T9[a[j + 2]] ^ T14[a[j + 3]];
		}
		for (int j = 0; j < 16; j++)
			a[j] = Si[b[Unshift[j]]];
	}
	// round 0
	for (int i = 0; i < 16; i++)
		out[i] = a[i] ^ round_keys[i];
	aes_dump_block(out, 16);
}

int main()
{
	unsigned char key[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };
	unsigned char clear[] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff };
	unsigned char encrypted[16];
	unsigned char decrypted[16];
	printf("init constants...\n");
	init_aes_consts();
	printf("key schedule...\n");
	init_aes_key(key);
	printf("encrypt...\n");
	aes_block_encrypt(clear, encrypted);
	printf("decrypt...\n");
	aes_block_decrypt(encrypted, decrypted);
    return 0;
}

