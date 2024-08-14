/* convert 15 bit pixels to double ham8 pixels */

#include <stdio.h>
#include <exec/types.h>
#include <intuition/intuition.h>

ULONG *thetable;

int color_idx(UWORD r,UWORD g,UWORD b)
{
	/* 555 -> idx */
	return((r>>3)*16+(g>>3)*4+(b>>3));
}

int realval(int gun)
{
	return((gun & 0x18));
}

int error(gunval)
{
	return(abs(gunval-realval(gunval)));
}


int correct(gun)
{
	int val;
	val=2*gun-realval(gun);
	if (val>32) val=31;
	if (val<0) val=0;
	return(val);

}

void init_table()
{
	if (thetable=(ULONG *) AllocMem(32768*4,0))
	{
		LONG r,g,b,delta_r,delta_g,delta_b,c1,c2,mod,val,total_error,error_delta;
		LONG r1,b1,g1,trycolor;
		for(r=0;r<32;r++)
			for(g=0;g<32;g++)
				for(b=0;b<32;b++)
				{
					int idx=b+g*32+r*1024;
					c1=color_idx(r,g,b);
					delta_r=5*error(r);
					delta_g=7*error(g);
					delta_b=3*error(b);
					r1=correct(r);
					g1=correct(g);
					b1=correct(b);
					total_error=delta_r+delta_g+delta_b;
					if (delta_r > delta_g) { mod=128; val=r1; error_delta=delta_r; }
					else { mod=128+64; val=g1; delta_r=delta_g; error_delta=delta_g; }
					if (delta_r < delta_b) { mod=64 ; val=b1; error_delta=delta_b; }
					trycolor=color_idx(r1,g1,b1);
					delta_r=abs(r-((realval(r1)+realval(r))/2));
					delta_g=abs(g-((realval(g1)+realval(g))/2));
					delta_b=abs(b-((realval(b1)+realval(b))/2));
					if (5*delta_r+7*delta_g+3*delta_b < (total_error-error_delta))
					{
						mod=0; val=trycolor;
					} else val *= 2;
					c2=mod+val;
/*					printf("rgb=%d %d %d values=%02x %02x\n",r,g,b,c1,c2); */
					thetable[idx]=(c1<<24)+(c2<<8);
				}
	}
}

void plotrgb(struct RastPort *rp,int x,int y,UBYTE red,UBYTE blue,UBYTE green)
{
	int idx=blue+green*32+red*1024;
	SetAPen(rp,(thetable[idx])>>24);
	printf("rgb=%d %d %d color=%02x",red,blue,green,GetAPen(rp));
	WritePixel(rp,x*2,y);
	SetAPen(rp,(thetable[idx])>>8);
	printf(" %02x\n",GetAPen(rp));
	WritePixel(rp,x*2+1,y);
}


void cleanup()
{
	if (thetable) FreeMem(thetable,32768*4);
}

struct Library *IntuitionBase;
struct Library *GfxBase;

main()
{
	int r,g,b,x,y;
	struct Screen *myscreen;
	IntuitionBase=(struct Library *) OpenLibrary("intuition.library",39);
	GfxBase=(struct Library *) OpenLibrary("graphics.library",39);
/*	for(x=0;x<32;x++) printf("realval=%d err=%d\n",realval(x),error(x)); */
	init_table();
	myscreen=OpenScreenTags(0,SA_Depth,8,SA_Width,320,SA_Height,200, SA_DisplayID,0x89824,TAG_END);
	for(x=0;x<64;x++)
		SetRGB32(&(myscreen->ViewPort),x,((x>>4)<<30),
			(((x & 6)>>2)<<30),(x<<30));


	for(x=0;x<32;x++)
		for(y=0;y<32;y++)
			plotrgb(&(myscreen->RastPort),x,y,x,x,x);
	getchar();
	CloseScreen(myscreen);


	cleanup();
}

