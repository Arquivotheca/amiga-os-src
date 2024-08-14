/***********************************************************
	maketree.c -- make Huffman tree
***********************************************************/
#ifndef LHEXTRACT

#include "lharc.h"
#include "slidehuf.h"

static short n, heapsize, heap[NC + 1];
static unsigned short *freq, *sort;
static unsigned char *len;
static unsigned short len_cnt[17];

void make_code(int n, unsigned char len[], unsigned short code[])
{
    unsigned short weight[17];	/* 0x10000ul >> bitlen */
    unsigned short start[17];	/* start code */
    unsigned short j, k[17] =
    {
	0,
	0x8000,0x4000,0x2000,0x1000,0x0800,0x0400,0x0200,0x0100,
	0x0080,0x0040,0x0020,0x0010,0x0008,0x0004,0x0002,0x0001,
    };
    int i;

    j = 0;

    for (i = 1; i <= 16; i++)
    {
	start[i] = j;
	j += (weight[i] = k[i]) * len_cnt[i];
    }

    for (i = 0; i < n; i++)
    {
	j = len[i];
	code[i] = start[j];
	start[j] += weight[j];
    }
}

STATIC void count_len(int i)  /* call with i = root */
{
	static unsigned char depth = 0;

	if (i < n) len_cnt[depth < 16 ? depth : 16]++;
	else {
		depth++;
		count_len(left [i]);
		count_len(right[i]);
		depth--;
	}
}

STATIC void make_len(int root)
{
    int i, k;
    unsigned int cum;

    for (i = 0; i <= 16; i++)
	len_cnt[i] = 0;
  
    count_len(root);
    cum = 0;
    for (i = 16; i > 0; i--)
    {
	cum += len_cnt[i] << (16 - i);
    }

#if (UINT_MAX != 0xffff)
    cum &= 0xffff;
#endif

    /* adjust len */
    if (cum)
    {
	fprintf(stderr, "17");
	len_cnt[16] -= cum;	/* always len_cnt[16] > cum */

	do
	{
	    for (i = 15; i > 0; i--)
	    {
		if (len_cnt[i])
		{
		    len_cnt[i]--;
		    len_cnt[i + 1] += 2;
		    break;
		}
	    }
	} while (--cum);
    }

    /* make len */
    for (i = 16; i > 0; i--)
    {
	k = len_cnt[i];
	while (k > 0)
	{
	    len[*sort++] = i;
	    k--;
	}
    }
}

/* priority queue; send i-th entry down heap */
STATIC void downheap(int i)
{
    short j, k;

    k = heap[i];
    while ((j = 2 * i) <= heapsize)
    {
	if (j < heapsize && freq[heap[j]] > freq[heap[j + 1]])
	    j++;

	if (freq[k] <= freq[heap[j]])
	    break;

	heap[i] = heap[j];
	i = j;
    }
    heap[i] = k;
}

	/* make tree, calculate len[], return root */
short make_tree(int nparm,unsigned short freqparm[],unsigned char lenparm[],
		unsigned short codeparm[])
{
    short i, j, k, avail;

    n = nparm;
    freq = freqparm;
    len = lenparm;
    avail = n;
    heapsize = 0;
    heap[1] = 0;

    for (i = 0; i < n; i++)
    {
	len[i] = 0;
	if (freq[i])
	    heap[++heapsize] = i;
    }

    if (heapsize < 2)
    {
	codeparm[heap[1]] = 0;
	return heap[1];
    }

    for (i = heapsize / 2; i >= 1; i--)
	downheap(i);		/* make priority queue */

    sort = codeparm;

    do
    {			/* while queue has at least two entries */
	i = heap[1];		/* take out least-freq entry */

	if (i < n)
	    *sort++ = i;

	heap[1] = heap[heapsize--];
	downheap(1);
	j = heap[1];		/* next least-freq entry */

	if (j < n)
	    *sort++ = j;

	k = avail++;		/* generate new node */
	freq[k] = freq[i] + freq[j];
	heap[1] = k;
	downheap(1); /* put into queue */
	left[k] = i;
	right[k] = j;
    }
    while (heapsize > 1);

    sort = codeparm;
    make_len(k);
    make_code(nparm, lenparm, codeparm);
    return k;			/* return root */
}

#endif
