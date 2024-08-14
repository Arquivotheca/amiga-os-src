#include "/math.h"

#define  QQQ(a)		DHiToL(a),DLoToL(a)
main()
{
    double o0, o32, n0, n32, t1, t2;

    o0 = LToD(1);
    n0 = DNeg(o0);
    o32 = LMul(0x10000, 0x10000);
    n32 = DNeg(o32);

    printf("o0	$%08lx:%08lx,	n0	$%08lx:%08lx\n", QQQ(o0), QQQ(n0));
    printf("o32	$%08lx:%08lx,	n32	$%08lx:%08lx\n", QQQ(o32), QQQ(n32));
    printf("o0+n0	$%08lx:%08lx\n",
	QQQ(DAdd(o0,n0)));
    printf("o0-n0	$%08lx:%08lx,	n0-o0	$%08lx:%08lx\n",
	QQQ(DSub(o0,n0)), QQQ(DSub(n0,o0)));
    printf("o0+n32	$%08lx:%08lx\n",
	QQQ(DAdd(o0,n32)));
    printf("o0-n32	$%08lx:%08lx,	n32-o0	$%08lx:%08lx\n",
	QQQ(DSub(o0,n32)), QQQ(DSub(n32,o0)));
    printf("o32+n0	$%08lx:%08lx\n",
	QQQ(DAdd(o32,n0)));
    printf("o32-n0	$%08lx:%08lx,	n0-o32	$%08lx:%08lx\n",
	QQQ(DSub(o32,n0)), QQQ(DSub(n0,o32)));
    printf("o32+n32	$%08lx:%08lx\n",
	QQQ(DAdd(o32,n32)));
    printf("o32-n32	$%08lx:%08lx,	n32-o32	$%08lx:%08lx\n",
	QQQ(DSub(o32,n32)), QQQ(DSub(n32,o32)));
    printf("\n");
    printf("-(o32+n0)	$%08lx:%08lx\n",
	QQQ(DNeg(DAdd(o32,n0))));
    printf("100H*100H	$%08lx:%08lx\n",
	QQQ(LMul(0x100, 0x100)));
    printf("-100H*100H	$%08lx:%08lx\n",
	QQQ(LMul(-0x100, 0x100)));
    printf("-100H*-100H	$%08lx:%08lx\n",
	QQQ(LMul(-0x100, -0x100)));
    printf("\n");
    printf("7FFFFFFF**2	$%08lx:%08lx\n",
	QQQ(LMul(0x7fffffff, 0x7fffffff)));
    printf("80000000**2	$%08lx:%08lx\n",
	QQQ(LMul(0x80000000, 0x80000000)));
    printf("\n");
    printf("o0?o0 %ld, o0?n0 %ld, o0?o32 %ld, o0?n32 %ld\n",
	DCmp(o0,o0), DCmp(o0,n0), DCmp(o0,o32), DCmp(o0,n32));
    printf("n0?o0 %ld, n0?n0 %ld, n0?o32 %ld, n0?n32 %ld\n",
	DCmp(n0,o0), DCmp(n0,n0), DCmp(n0,o32), DCmp(n0,n32));
    printf("o32?o0 %ld, o32?n0 %ld, o32?o32 %ld, o32?n32 %ld\n",
	DCmp(o32,o0), DCmp(o32,n0), DCmp(o32,o32), DCmp(o32,n32));
    printf("n32?o0 %ld, n32?n0 %ld, n32?o32 %ld, n32?n32 %ld\n",
	DCmp(n32,o0), DCmp(n32,n0), DCmp(n32,o32), DCmp(n32,n32));
    printf("\n");
    printf("o0>>8	$%08lx:%08lx,	o0>>24	$%08lx:%08lx\n",
	QQQ(DShift(o0,-8)), QQQ(DShift(o0,-24)));
    printf("o0<<8	$%08lx:%08lx,	o0<<24	$%08lx:%08lx\n",
	QQQ(DShift(o0,8)), QQQ(DShift(o0,24)));
    printf("n0>>8	$%08lx:%08lx,	n0>>24	$%08lx:%08lx\n",
	QQQ(DShift(n0,-8)), QQQ(DShift(n0,-24)));
    printf("n0<<8	$%08lx:%08lx,	n0<<24	$%08lx:%08lx\n",
	QQQ(DShift(n0,8)), QQQ(DShift(n0,24)));
    printf("o32>>8	$%08lx:%08lx,	o32>>24	$%08lx:%08lx\n",
	QQQ(DShift(o32,-8)), QQQ(DShift(o32,-24)));
    printf("o32<<8	$%08lx:%08lx,	o32<<24	$%08lx:%08lx\n",
	QQQ(DShift(o32,8)), QQQ(DShift(o32,24)));
    printf("n32>>8	$%08lx:%08lx,	n32>>24	$%08lx:%08lx\n",
	QQQ(DShift(n32,-8)), QQQ(DShift(n32,-24)));
    printf("n32<<8	$%08lx:%08lx,	n32<<24	$%08lx:%08lx\n",
	QQQ(DShift(n32,8)), QQQ(DShift(n32,24)));
    printf("\n");
    printf("2/1	$%08lx:%08lx\n", QQQ(DDiv(LToD(2),LToD(1))));
    printf("3/1	$%08lx:%08lx\n", QQQ(DDiv(LToD(3),LToD(1))));
    printf("4/2	$%08lx:%08lx\n", QQQ(DDiv(LToD(4),LToD(2))));
    printf("5/2	$%08lx:%08lx\n", QQQ(DDiv(LToD(5),LToD(2))));
    printf("\n");
    printf("o0/o0	$%08lx:%08lx\n", QQQ(DDiv(o0,o0)));
    printf("o0/n0	$%08lx:%08lx\n", QQQ(DDiv(o0,n0)));
    printf("o0/o32	$%08lx:%08lx\n", QQQ(DDiv(o0,o32)));
    printf("o0/n32	$%08lx:%08lx\n", QQQ(DDiv(o0,n32)));
    printf("n0/o0	$%08lx:%08lx\n", QQQ(DDiv(n0,o0)));
    printf("n0/n0	$%08lx:%08lx\n", QQQ(DDiv(n0,n0)));
    printf("n0/o32	$%08lx:%08lx\n", QQQ(DDiv(n0,o32)));
    printf("n0/n32	$%08lx:%08lx\n", QQQ(DDiv(n0,n32)));
    printf("o32/o0	$%08lx:%08lx\n", QQQ(DDiv(o32,o0)));
    printf("o32/n0	$%08lx:%08lx\n", QQQ(DDiv(o32,n0)));
    printf("o32/o32	$%08lx:%08lx\n", QQQ(DDiv(o32,o32)));
    printf("o32/n32	$%08lx:%08lx\n", QQQ(DDiv(o32,n32)));
    printf("n32/o0	$%08lx:%08lx\n", QQQ(DDiv(n32,o0)));
    printf("n32/n0	$%08lx:%08lx\n", QQQ(DDiv(n32,n0)));
    printf("n32/o32	$%08lx:%08lx\n", QQQ(DDiv(n32,o32)));
    printf("n32/n32	$%08lx:%08lx\n", QQQ(DDiv(n32,n32)));
    printf("$8000000/$2000	$%08lx:%08lx\n",
	    QQQ(DDiv(LToD(0x8000000),LToD(0x2000))));
    printf("\n");
    t1 = DShift(LToD(80),16);
    t2 = DShift(LToD(2000),16);
    printf("t1 $%08lx:%08lx, t2 $%08lx:%08lx, t1/t2 $%08lx:%08lx\n",
	QQQ(t1), QQQ(t2), QQQ(DDiv(t1,t2)));
    printf("t1<<16 $%08lx:%08lx, (t1<<16)/t2 $%08lx:%08lx\n",
	QQQ(DShift(t1,16)), QQQ(DDiv(DShift(t1,16),t2)));
}
