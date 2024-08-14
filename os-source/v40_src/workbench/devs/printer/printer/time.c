#include <exec/types.h>

#include "printer_iprotos.h"

#define TDEBUG	0	/* time debugging */
#define TCDEBUG	0	/* time critical debugging */

#if TDEBUG
#include <hardware/cia.h>
extern struct CIA ciaa;
ULONG ReadVBlankTime()
{
	ULONG jiffy;

	jiffy = ciaa.ciatodhi << 16;
	jiffy |= ciaa.ciatodmid << 8;
	jiffy |= ciaa.ciatodlow;
	return(jiffy);
}
#endif TDEBUG

#if TCDEBUG
#include <hardware/custom.h>
#include "timing.h"
extern struct Custom custom;
ULONG *vposr = &custom.vposr;

void ReadVertTime(t)
struct mytimeval *t;
{
	ULONG time = *vposr & 0x1ffff;
	t->row = time >> 8;
	t->col = time & 0xff;
}

void ElapsedVertTime(t1, t2, total)
struct mytimeval *t1, *t2, *total;
{
	ULONG elapsedrow, elapsedcol;

	elapsedrow = t2->row;
	if (t2->row < t1->row) {
		elapsedrow += 263;
	}
	elapsedrow -= t1->row;
	elapsedcol = t2->col;
	if (t2->col < t1->col) {
		elapsedcol += 228;
		elapsedrow--;
	}
	elapsedcol -= t1->col;
	total->col += elapsedcol;
	if (total->col > 228) {
		total->col -= 228;
		total->row++;
	}
	total->row += elapsedrow;
	total->count++;
}

void PrintAvgTime(s, total, sum)
char *s;
struct mytimeval *total, *sum;
{
	struct mytimeval avg;
	ULONG count = total->count, percent;

	if (count) {
		avg.row = total->row / count;
		avg.col = (total->col + (total->row - avg.row * count) * 228) / count;
	}
	else {
		avg.row = avg.col = 0;
	}
	percent = (total->row * 100 + sum->row / 2) / sum->row;
	kprintf("%s total=%ld:%ld, count=%ld, avg=%ld:%ld, percent=%ld\n",
		s, total->row, total->col, count, avg.row, avg.col, percent);
}
#endif TCDEBUG
