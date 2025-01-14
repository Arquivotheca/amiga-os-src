#include "math.h"

#define NEWCODE

#ifdef NEWCODE

struct coerce
{
	union
	{
		double	dbl;
		DOUBLE	bits;
	} qwe;
};

extern get_r_g();

struct quad
{
	unsigned long	left;
	unsigned long	right;
};

struct quad A1[17]=	/* table of 64 bit values */
{
	{ 0x80000000,0x00000000 },/*0000,*/
	{ 0x7a92be8a,0x92436616 },/*3dc0,*/
	{ 0x75606373,0xee921c97 },/*6820,*/
	{ 0x70666f76,0x154a7089 },/*8320,*/	/* rounded up */
	{ 0x6ba27e65,0x6b4eb57a },/*1ce0,*/
	{ 0x6712460a,0x8fc24072 },/*f120,*/	/* rounded up */
	{ 0x62b39508,0xaa836d6f },/*9f20,*/	/* rounded up */
	{ 0x5e8451cf,0xac061b5f },/*5440,*/
	{ 0x5a827999,0xfcef3242 },/*2cc0,*/
	{ 0x56ac1f75,0x2150a563 },/*24c0,*/
	{ 0x52ff6b54,0xd8a89c75 },/*0e60,*/
	{ 0x4f7a9930,0x48d088d7 },/*d040,*/	/* rounded up */
	{ 0x4c1bf828,0xc6dc54b8 },/*a360,*/	/* rounded up */
	{ 0x48e1e9b9,0xd588e19b },/*07e0,*/
	{ 0x45cae0f1,0xf545eb73 },/*7e00,*/
	{ 0x42d561b3,0xe6243d8a },/*62e0,*/
};

double	p1=	0.00130208333334565;
double	p2=	0.00000305175731568;
double	p3=	0.00000000852136365;

double	q[7] =
{
	0.693147180559937815
	0.240226506956777522
	0.055504108424756866
	0.009618117691387241
	0.001333081011340821
	0.000150774061788142
};

double	log2e=	0.18033688011112042592e+0;

struct quad q_p1,q_p2,q_p3,q_log2e;
struct quad q_w2;

int k = 1;	/* for binary machine */
int m;
int r;

double U1;
double Y1,Y2;
double W;
double W1,W2;
double w2;
int iW1;
int I,N;

int p_prime,m_prime;

/*#define REDUCE(v)	(double)(floor(AUGMENTEXPONET(v,4)*0.0625)*/

double REDUCE(d)
double d;
{
	int x;
	x = EXTRACTEXPONENT(d);
	SETEXPONENT(d,x+4);
	d = floor(x);
	SETEXPONENT(d,x-4);
	return (d);
}

double REFLOAT(x)
int x;
{
	double t;
	t = x;
	return(SETEXPONENT(t,EXTRACTEXPONENT(t)-32));
}

struct quad gprime;

#define HIDDENBIT	0x00100000		/* bit 20 */

struct quad IWI_MAX = { 0x01ffffff, 0xfffffff0 };
struct quad IWI_MIN = { 0x00000000, 0x00000010 };

unsigned long masks[33]=
{
	0,
	0x00000001, 0x00000003, 0x00000007, 0x0000000f,
	0x0000001f, 0x0000003f, 0x0000007f, 0x000000ff,
	0x000001ff, 0x000003ff, 0x000007ff, 0x00000fff,
	0x00001fff, 0x00003fff, 0x00007fff, 0x0000ffff,
	0x0001ffff, 0x0003ffff, 0x0007ffff, 0x000fffff,
	0x001fffff, 0x003fffff, 0x007fffff, 0x00ffffff,
	0x01ffffff, 0x03ffffff, 0x07ffffff, 0x0fffffff,
	0x1fffffff, 0x3fffffff, 0x7fffffff, 0xffffffff
};

qshift_right();

qshift_left(q,n)
/*	shift 64 bit value left by n bits */
struct quad *q;
{
	unsigned long t;
	if (n<0)	qshift_right(q,-n);
	t = q->right;
	q->right <<= n;
	q->left <<= n;
	q->left |= (t >> (32-n)) & masks[n];
}

qshift_right(q,n)
struct quad *q;
int n;
{
	unsigned long t;
	if (n<0)	qshift_left(q,-n);
	t = q->left;
	q->left >>= n;
	q->right >>= n;
	q->right |= (t << (32-n));
}

int q_cmp(a,b)
struct quad *a,*b;
/* returns -1 if a<b */
/* returns 0  if a=b */
/* returns 1  if a>b */
{
	if (a->left < b->left) return (-1);
	if (a->left > b->left) return (1);
	if (a->right < b->right) return(-1);
	if (a->right > b->right) return (1);
	return (0);
}

q_add(a,b)
struct quad *a,*b;
/* adds a to b */
{
	unsigned long t;
	unsigned overflow = 0;
	t = a->right + b->right;
	if ( (t < a->right) || (t < b->right) ) overflow = 1;
	b->right = t;
	b->left += a->left + overflow;
}

q_neg(a)
struct quad *a;
/* negate quad a */
{
	struct quad t;
	a->left = ~a->left;
	a->right = ~a->right;
	t.left = 0;
	t.right = 1;
	q_add(&t,a);
}

q_sub(a,b)
struct quad *a,*b;
/* subtract a from b */
{
	struct quad t;
	t = *a;
	q_neg(&t);
	q_add(&t,b);
}

struct squad
{
	unsigned short i[4];
};

copyquad(a,b)
struct quad *a,*b;
{
	*b = *a;
}

addlong(t,p)
unsigned long t,*p;
/* returns boolean TRUE if overflow */
{
	unsigned long qwe;
	qwe = *p;
	*p += t;
	if ( (*p < qwe) || (*p < t) )	return (1);
	return (0);
}

addword(t,p)
unsigned short t,*p;
{
	unsigned short qwe;
	qwe = *p;
	*p += t;
	if ( (*p < qwe) || (*p < t) )	return (1);
	return (0);
}

q_multiply(a,b)
struct quad *a,*b;
/* multiply a*b leave result in b */
{
	struct squad a1;
	struct squad b1;
	unsigned long t;

	unsigned short product[6];

	copyquad(a,&a1);
	copyquad(b,&b1);

	addlong(a1.i[0] * b1.i[0],&product[0]);
	if (addlong(a1.i[0] * b1.i[1],&product[1]))	addword(1,&product[0]);
	if (addlong(a1.i[1] * b1.i[0],&product[1]))	addword(1,&product[0]);

	if (addlong(a1.i[1] * b1.i[1],&product[2]))	addlong(1,&product[0]);
	if (addlong(a1.i[0] * b1.i[2],&product[2]))	addlong(1,&product[0]);
	if (addlong(a1.i[2] * b1.i[0],&product[2]))	addlong(1,&product[0]);
	
	if (addlong(a1.i[0] * b1.i[3],&product[3]))
		if (addlong(1,&product[1]))	addword(1,&product[0]);
	if (addlong(a1.i[3] * b1.i[0],&product[3]))
		if (addlong(1,&product[1]))	addword(1,&product[0]);
	if (addlong(a1.i[1] * b1.i[2],&product[3]))
		if (addlong(1,&product[1]))	addword(1,&product[0]);
	if (addlong(a1.i[2] * b1.i[1],&product[3]))
		if (addlong(1,&product[1]))	addword(1,&product[0]);
}

q_divide(divisor,b)
struct quad *divisor,*b;
/* divide divisor into b, leave result in b */
{
	struct quad divisor;
	struct quad accumulator;
	struct quad abit;
	int n;
	int t;

	accumulator = *b;

	abit.left = 0x80000000;
	abit.right = 0;

	b->left = 0;
	b->right = 0;

	for (n = 0; n<64; n++)
	{
		t = q_cmp(divisor,accumulator);
		if ( t != 1)	/* if divisor is not greater than acc */
		{
			q_add(&abit,b);
			q_sub(divisor,&accumulator);
		}
		qshift_left(&accumulator,1);
		qshift_right(&abit,1);
	}
}

float_to_quad(x,q)
DOUBLE *x;
struct quad *q;
{
	int exponent;
	q->left = x->dbl_frac1 | HIDDENBIT;
	q->right = x->dbl_frac2;
	exponent = EXTRACTEXPONENT(*x);
	exponent += 11;
	qshift_left(q,exponent);
}

get_g(x)
DOUBLE	x;
{
	gprime.left = x.dbl_frac1 | HIDDENBIT;
	gprime.right = x.dbl_frac2;
	qshift_left(&gprime,r-1);
}

int p;
struct quad z;
struct quad eight_R;
struct quad U2;

get_p()
{
	int t;
	p = 1;
	t = q_cmp(&gprime,&A1[9]);
	if (t != 1)	p = 9;
	t = q_cmp(&gprime,&A1[p+4]);
	if (t != 1)	p += 4;
	t = q_cmp(&gprime,&A1[p+2]);
	if (t != 1)	p += 2;
}

get_z()
{
	struct quad dividend,divisor;

	dividend = gprime;
	q_sub(&dividend,&A1[p+1]);

	divisor = gprime;
	q_add(&divisor,&A1[p+1]);

	qshift_left(&dividend,4);

	z = dividend;

	q_divide(&divisor,&z);
}

struct quad v;

double explog(x,y)
double x,y;
{
	struct quad t;
	double result;
	int i;
	/* determine m */
	m = EXTRACTEXPONENT(x);
	/* determine r,g */
	r = 11;	/* number of zero bits left of full integer mantissa */
	get_g(x);

	/* determine p */
	get_p();

	/* compute R(z)->U2 */
	
	v = z;
	q_multiply(&z,&v);	/* v = z*z */

	float_to_quad(&p1,&q_p1);
	float_to_quad(&p2,&q_p2);
	float_to_quad(&p3,&q_p3);

	t = q_p3;
	q_multiply(&v,&t);
	q_add(&q_p2,&t);
	q_multiply(&v,&t);
	q_add(&q_p1,&t);

	eight_R = t;
	q_multiply(&v,&eight_R);
	q_multiply(&z,&eight_R);
	q_add(&z,&eight_R);

	U2 = eight_R;
	float_to_quad(&log2e,&q_log2e);
	q_multiply(&q_log2e,&U2);

	/* compute YxU -> W */

	{
		int t;
		t = (k*m-r)*16-p;
		U1 = t;
		AUGMENTEXPONENT(U1,-4);
	}

	Y1 = REDUCE(y);
	Y2 = y - Y1;
	W = REFLOAT(U2)*y + U1*Y2;
	W1 = REDUCE(W);
	W2 = W-W1;
	W = W1+U1*Y1;
	W1 = REDUCE(W);
	W2 = W2+(W-W1);
	W = REDUCE(W2);
	iW1 = 16*(W1+W);
	w2 = floor(W2-W);
	float_to_quad(&w2,&q_w2);

	/* suggestions 10,11,12 */
	/* ignored */

	/* determine p', r', m' */

	if (iW1 <= 0)	I = 0;
	else		I = 1;

	N = iW1/16 + I;
	p_prime = 16*N - iW1;

	m_prime = N + I + 1;

	/* n^W2 -> Z */
	z.left = 0;
	z.right = 0;

	{
		struct quad t_q;
		for (i = 6 ; i >= 0; i--)
		{
			float_to_quad(&q[i],&t_q);
			q_add(&t_q,&z);
			q_multiply(&q_w2,&z);
		}
			/*z += w2*(z + q[i]);*/
	}

	/*q_multiply(&q_w2,&z); not needed? */

	/* 15, zxB^m'xn^(-r'-p'/C) -> result */

	q_multiply(&A1[p_prime+1],&z);
	q_add(&A1[p_prime+1],&z);

	REFLOAT(&z,result);

	AUGMENTEXPONENT(result,m_prime);

	return (result);
}
#else

static double explog(x,y)
double x,y;
{
	return newpow(x,y);
	return( exp( y*log( x )));
}
#endif


extern double hardpow();


double pow( x ,y )
double x,y;
{
	double result;
	result = hardpow(x,y,explog);
	return (result);
}

#ifdef ANCIENT_CODE


double pow( x, y ) double x, y ; {
	/* calculate x^y */
    register double result ;
	int iy;

	if (y == 0.0)	return (1.0);

    if (floor(y) == y && ((-1000 < y && y < 1000) || x <= 0.0)) {
	if (x == 0.0)
	    if (y > 0)
		return(0.0);
	    else {
		return(1.0);
	    }
	result = 1.0;
	/* we know that -1000 < y < 1000 and it has no fraction */
	iy = y;		/* convert to an integer to be quicker */
	if (iy < 0) {
	    iy = -iy;
	    x = 1/x;
	}
	if (iy > 10)
	{	/* lets magnitude reduce */
		int a,b;
		a = iy>>1;
		b = iy-a;
		result = pow(x,(double)a);
		result *= result;
		if (a != b)	result *= x;	/* one more */
	}
	else
	{
		while (iy-- > 0)
    		result *= x ;
	}
	return( result );
    } else if ( x <= 0.0 ) {
	return( 0.0 );
    } else
	return( exp( y*log( x )));
}
#endif
