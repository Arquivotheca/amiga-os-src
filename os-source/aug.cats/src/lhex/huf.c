/***********************************************************
	huf.c -- new static Huffman
***********************************************************/
#ifdef sony_news
#include <sys/param.h>
#endif
#if defined(__STDC__) || defined(NEWSOS)
#include <stdlib.h>
#endif
#include "slidehuf.h"

#include "lharc.h"

#define NP (MAX_DICBIT + 1)
#define NT (USHRT_BIT + 3)
#define PBIT 4  /* smallest integer such that (1 << PBIT) > NP */
#define TBIT 5  /* smallest integer such that (1 << TBIT) > NT */
/*#if NT > NP
	#define NPT NT
#else
	#define NPT NP
#endif*/
#define NPT 0x80

unsigned short left[2 * NC - 1], right[2 * NC - 1];
unsigned char c_len[NC], pt_len[NPT];
unsigned short c_freq[2 * NC - 1], c_table[4096], c_code[NC],
	   p_freq[2 * NP - 1], pt_table[256], pt_code[NPT],
	   t_freq[2 * NT - 1];

static unsigned char *buf;
static unsigned short bufsiz;
static unsigned short blocksize;

#ifndef LHEXTRACT
/***** encoding *****/

static void count_t_freq(void)
{
	short i, k, n, count;

	for (i = 0; i < NT; i++) t_freq[i] = 0;
	n = NC;
	while (n > 0 && c_len[n - 1] == 0) n--;
	i = 0;
	while (i < n) {
		k = c_len[i++];
		if (k == 0) {
			count = 1;
			while (i < n && c_len[i] == 0) {  i++;  count++;  }
			if (count <= 2) t_freq[0] += count;
			else if (count <= 18) t_freq[1]++;
			else if (count == 19) {  t_freq[0]++;  t_freq[1]++;  }
			else t_freq[2]++;
		} else t_freq[k + 2]++;
	}
}

static void write_pt_len(short n, short nbit, short i_special)
{
	short i, k;

	while (n > 0 && pt_len[n - 1] == 0) n--;
	putbits(nbit, n);
	i = 0;
	while (i < n) {
		k = pt_len[i++];
		if (k <= 6) putbits(3, k);
/* NOTE: original exceeds unsigned short! */
//		else putbits(k - 3, USHRT_MAX << 1);
		else putbits(k - 3, USHRT_MAX);
		if (i == i_special) {
			while (i < 6 && pt_len[i] == 0) i++;
			putbits(2, i - 3);
		}
	}
}

static void write_c_len(void)
{
	short i, k, n, count;

	n = NC;
	while (n > 0 && c_len[n - 1] == 0) n--;
	putbits(CBIT, n);
	i = 0;
	while (i < n) {
		k = c_len[i++];
		if (k == 0) {
			count = 1;
			while (i < n && c_len[i] == 0) {  i++;  count++;  }
			if (count <= 2) {
				for (k = 0; k < count; k++)
					putcode(pt_len[0], pt_code[0]);
			} else if (count <= 18) {
				putcode(pt_len[1], pt_code[1]);
				putbits(4, count - 3);
			} else if (count == 19) {
				putcode(pt_len[0], pt_code[0]);
				putcode(pt_len[1], pt_code[1]);
				putbits(4, 15);
			} else {
				putcode(pt_len[2], pt_code[2]);
				putbits(CBIT, count - 20);
			}
		} else putcode(pt_len[k + 2], pt_code[k + 2]);
	}
}

static void encode_c(short c)
{
	putcode(c_len[c], c_code[c]);
}

static void encode_p(unsigned short p)
{
	unsigned short c, q;

	c = 0;  q = p;  while (q) {  q >>= 1;  c++;  }
	putcode(pt_len[c], pt_code[c]);
	if (c > 1) putbits(c - 1, p);
}

static void send_block(void)
{
	unsigned char flags;
	unsigned short i, k, root, pos, size;

	root = make_tree(NC, c_freq, c_len, c_code);
	size = c_freq[root];  putbits(16, size);
	if (root >= NC) {
		count_t_freq();
		root = make_tree(NT, t_freq, pt_len, pt_code);
		if (root >= NT) {
			write_pt_len(NT, TBIT, 3);
		} else {
			putbits(TBIT, 0);  putbits(TBIT, root);
		}
		write_c_len();
	} else {
		putbits(TBIT, 0);  putbits(TBIT, 0);
		putbits(CBIT, 0);  putbits(CBIT, root);
	}
	root = make_tree(NP, p_freq, pt_len, pt_code);
	if (root >= NP) {
		write_pt_len(NP, PBIT, -1);
	} else {
		putbits(PBIT, 0);  putbits(PBIT, root);
	}
	pos = 0;
	for (i = 0; i < size; i++) {
		if (i % CHAR_BIT == 0) flags = buf[pos++];  else flags <<= 1;
		if (flags & (1 << (CHAR_BIT - 1))) {
			encode_c(buf[pos++] + (1 << CHAR_BIT));
			k = buf[pos++] << CHAR_BIT;  k += buf[pos++];
			encode_p(k);
		} else encode_c(buf[pos++]);
		if (unpackable) return;
	}
	for (i = 0; i < NC; i++) c_freq[i] = 0;
	for (i = 0; i < NP; i++) p_freq[i] = 0;
}

static unsigned short output_pos, output_mask;

void output_st1(unsigned short c, unsigned short p)
{
	static unsigned short cpos;

	output_mask >>= 1;
	if (output_mask == 0) {
		output_mask = 1 << (CHAR_BIT - 1);
		if (output_pos >= bufsiz - 3 * CHAR_BIT) {
			send_block();
			if (unpackable) return;
			output_pos = 0;
		}
		cpos = output_pos++;  buf[cpos] = 0;
	}
	buf[output_pos++] = (unsigned char) c;  c_freq[c]++;
	if (c >= (1 << CHAR_BIT)) {
		buf[cpos] |= output_mask;
		buf[output_pos++] = (unsigned char)(p >> CHAR_BIT);
		buf[output_pos++] = (unsigned char) p;
		c = 0;  while (p) {  p >>= 1;  c++;  }
		p_freq[c]++;
	}
}

unsigned char *alloc_buf(void)
{
	bufsiz = 16 * 1024; /* 65408U; */
	while ((buf = (unsigned char *)malloc(bufsiz)) == NULL) {
		bufsiz = (bufsiz / 10) * 9;
		if (bufsiz < 4 * 1024) break;
	}
	return buf;
}

void encode_start_st1(void)
{
	int i;

	for (i = 0; i < NC; i++) c_freq[i] = 0;
	for (i = 0; i < NP; i++) p_freq[i] = 0;
	output_pos = output_mask = 0;
	init_putbits();
	buf[0] = 0;
}

void encode_end_st1(void)
{
	if (! unpackable) {
		send_block();
		putbits(CHAR_BIT - 1, 0);  /* flush remaining bits */
	}
}
#endif /* LHEXTRACT */

/***** decoding *****/

static void read_pt_len(short nn, short nbit, short i_special)
{
	short i, c, n;

	n = getbits(nbit);
	if (n == 0) {
		c = getbits(nbit);
		for (i = 0; i < nn; i++) pt_len[i] = 0;
		for (i = 0; i < 256; i++) pt_table[i] = c;
	} else {
		i = 0;
		while (i < n) {
			c = bitbuf >> (16 - 3);
			if (c == 7) {
				unsigned short mask = 1 << (16 - 4);
				while (mask & bitbuf) {  mask >>= 1;  c++;  }
			}
			fillbuf((c < 7) ? 3 : c - 3);
			pt_len[i++] = c;
			if (i == i_special) {
				c = getbits(2);
				while (--c >= 0) pt_len[i++] = 0;
			}
		}
		while (i < nn) pt_len[i++] = 0;
		make_table(nn, pt_len, 8, pt_table);
	}
}

static void read_c_len(void)
{
	short i, c, n;

	n = getbits(CBIT);
	if (n == 0) {
		c = getbits(CBIT);
		for (i = 0; i < NC; i++) c_len[i] = 0;
		for (i = 0; i < 4096; i++) c_table[i] = c;
	} else {
		i = 0;
		while (i < n) {
			c = pt_table[bitbuf >> (16 - 8)];
			if (c >= NT) {
				unsigned short mask = 1 << (16 - 9);
				do {
					if (bitbuf & mask) c = right[c];
					else               c = left [c];
					mask >>= 1;
				} while (c >= NT);
			}
			fillbuf(pt_len[c]);
			if (c <= 2) {
				if      (c == 0) c = 1;
				else if (c == 1) c = getbits(4) + 3;
				else             c = getbits(CBIT) + 20;
				while (--c >= 0) c_len[i++] = 0;
			} else c_len[i++] = c - 2;
		}
		while (i < NC) c_len[i++] = 0;
		make_table(NC, c_len, 12, c_table);
	}
}

unsigned short decode_c_st1(void)
{
	unsigned short j, mask;

	if (blocksize == 0) {
		blocksize = getbits(16);
		read_pt_len(NT, TBIT, 3);
		read_c_len();
		read_pt_len(NP, PBIT, -1);
	}
	blocksize--;
	j = c_table[bitbuf >> 4];
	if (j < NC) fillbuf(c_len[j]);
	else {
		fillbuf(12);  mask = 1 << (16 - 1);
		do {
			if (bitbuf & mask) j = right[j];
			else               j = left [j];
			mask >>= 1;
		} while (j >= NC);
		fillbuf(c_len[j] - 12);
	}
	return j;
}

unsigned short decode_p_st1(void)
{
	unsigned short j, mask;

	j = pt_table[bitbuf >> (16 - 8)];
	if (j < NP) fillbuf(pt_len[j]);
	else {
		fillbuf(8);  mask = 1 << (16 - 1);
		do {
			if (bitbuf & mask) j = right[j];
			else               j = left [j];
			mask >>= 1;
		} while (j >= NP);
		fillbuf(pt_len[j] - 8);
	}
	if (j != 0) j = (1 << (j - 1)) + getbits(j - 1);
	return j;
}

void decode_start_st1(void)
{
	init_getbits();
	blocksize = 0;
}
