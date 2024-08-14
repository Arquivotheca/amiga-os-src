/***********************************************************
	shuf.c -- extract static Huffman coding
***********************************************************/
#include <stdio.h>
#include "lharc.h"
#include "slidehuf.h"
#ifdef __STDC__
#include <stdlib.h>
#endif

#define N1 286  /* alphabet size */
#define N2 (2 * N1 - 1)  /* # of nodes in Huffman tree */
#define EXTRABITS 8
	/* >= log2(F-THRESHOLD+258-N1) */
#define BUFBITS  16  /* >= log2(MAXBUF) */
#define LENFIELD  4  /* bit size of length field for tree output */
#define NP (8 * 1024 / 64)
#define NP2 (NP * 2 - 1)

static unsigned int np;

void decode_start_st0(void)
{
	n_max = 286;
	maxmatch = MAXMATCH;
	init_getbits();
	np = 1 << (MAX_DICBIT - 6);
}

void encode_p_st0(unsigned short j)
{
	unsigned short i;

	i = j >> 6;
	putcode(pt_len[i], pt_code[i]);
	putbits(6, j & 0x3f);
}

int fixed[2][16] = {
 /* old compatible */
    { 3,  1,  4, 12, 24, 48,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/* 8K buf */
    { 2,  1,  1,  3,  6, 13, 31, 78,  0,  0,  0,  0,  0,  0,  0,  0}
};

STATIC void ready_made(int method)
{
    int i, j;
    unsigned int code, weight;
    int *tbl;

    tbl = fixed[method];
    j = *tbl++;
    weight = 1 << (16 - j);
    code = 0; 

    for (i = 0; i < np; i++)
    {
	while (*tbl == i)
	{
	    j++;
	    tbl++;
	    weight >>= 1;
	}
	pt_len[i] = j;
	pt_code[i] = code;
	code += weight;
    }
}

void encode_start_fix(void)
{
    n_max = 314;
    maxmatch = 60;
    np = 1 << (12 - 6);
    init_putbits();
    start_c_dyn();
    ready_made(0);
}

STATIC void read_tree_c(void)  /* read tree from file */
{
    int i, c;

    i = 0;
    while (i < N1)
    {
	if (getbits(1))
	    c_len[i] = getbits(LENFIELD) + 1;
	else
	    c_len[i] = 0;

	if (++i == 3 && c_len[0] == 1 && c_len[1] == 1 && c_len[2] == 1)
	{
	    c = getbits(CBIT);

	    for (i = 0; i < N1; i++)
		c_len[i] = 0;
	    for (i = 0; i < 4096; i++)
		c_table[i] = c;
	    return;
	}
    }
    make_table(N1, c_len, 12, c_table);
}

STATIC void read_tree_p(void)  /* read tree from file */
{
    int i, c;

    i = 0;
    while (i < NP)
    {
	pt_len[i] = getbits(LENFIELD);
	if (++i == 3 && pt_len[0] == 1 && pt_len[1] == 1 && pt_len[2] == 1)
	{
	    c = getbits(MAX_DICBIT - 6);
	    for (i = 0; i < NP; i++)
		c_len[i] = 0;
	    for (i = 0; i < 256; i++)
		c_table[i] = c;
	    return;
	}
    }
}

void decode_start_fix(void)
{
	n_max = 314;
	maxmatch = 60;
	init_getbits();
	np = 1 << (12 - 6);
	start_c_dyn();
	ready_made(0);
	make_table(np, pt_len, 8, pt_table);
}

unsigned short decode_c_st0(void)
{
	int i, j;
	static unsigned short blocksize = 0;

	if (blocksize == 0) {  /* read block head */
		blocksize = getbits(BUFBITS);  /* read block blocksize */
		read_tree_c();
		if (getbits(1)) {
			read_tree_p();
		} else {
			ready_made(1);
		}
		make_table(NP, pt_len, 8, pt_table);
	}
	blocksize--;
	j = c_table[bitbuf >> 4];
	if (j < N1) fillbuf(c_len[j]);
	else {
		fillbuf(12); i = bitbuf;
		do {
			if ((short)i < 0) j = right[j];
			else              j = left [j];
			i <<= 1;
		} while (j >= N1);
		fillbuf(c_len[j] - 12);
	}
	if (j == N1 - 1)
		j += getbits(EXTRABITS);
	return (unsigned short)j;
}

unsigned short decode_p_st0(void)
{
	int i, j;

	j = pt_table[bitbuf >> 8];
	if (j < np) {
		fillbuf(pt_len[j]);
	} else {
		fillbuf(8); i = bitbuf;
		do {
			if ((short)i < 0) j = right[j];
			else              j = left [j];
			i <<= 1;
		} while (j >= np);
		fillbuf(pt_len[j] - 8);
	}
	return ((unsigned short)((j << 6) + getbits(6)));
}
