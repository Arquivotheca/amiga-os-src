
#include <exec/types.h>
#include <intuition/intuition.h>
#include <libraries/dos.h>
#include "stdio.h"

char	m0[]	=	"Reset and Status";
char	m1[]	=	"Show cursor";
char	m2[]	=	"Hide cursor";
char	m3[]	=	"Get button stat&pos ";
char	m4[]	=	"set mouse curs. pos";
char	m5[]	=	"get button prs info";
char	m6[]	=	"get button rls info";
char	m7[]	=	"set min/max horiz";
char	m8[]	=	"set min/max vert";
char	m9[]	=	"set graphics block";
char	m10[]	=	"set text cursor";
char	m11[]	=	"read motion cntrs";
char	m12[]	=	"set int. sub. ";
char	m13[]	=	"light pen emu. on";
char	m14[]	=	"light pen emu. off";
char	m15[]	=	"set mickey/p ratio";
char	m16[]	=	"conditional off";
char	m19[]	=	"set double speed t";
char	m20[]	=	"swap interrupt subrs";
char	m21[]	=	"get mouse state req";
char	m22[]	=	"save mouse state";
char	m23[]	=	"restore mouse state";
char	m29[]	=	"set CRT pageno";
char	m30[]	=	"get CRT pageno";
char	mx[]	=	"not defined";

static	char *Commands[] =
	{
	m0,
	m1,
	m2,
	m3,
	m4,
	m5,
	m6,
	m7,
	m8,
	m9,
	m10,
	m11,
	m12,
	m13,
	m14,
	m15,
	m16,
	mx,
	mx,
	m19,
	m20,
	m21,
	m22,
	m23,
	mx,
	mx,
	mx,
	mx,
	mx,
	m29,
	m30,
	};

char	e0[] = "Read one register";
char	e1[] = "Write one register";
char	e2[] = "Read register range";
char	e3[] = "Write register range";
char	e4[] = "Read register set";
char	e5[] = "Write register set";
char	e6[] = "Revert to defaults";
char	e7[] = "Define defaults";
char	e10[] = "Interrogate";

char	*EGACommands[] =
	{
	e0,
	e1,
	e2,
	e3,
	e4,
	e5,
	e6,
	e7,
	mx,
	mx,
	e10,
	};

main(Count,Params)
int	Count;
char	*Params[];
	{
	register	int	x = -1;
	register	int	prev;
	register	int	count = 0;
	register	int	FileHandle;
	UBYTE	Buffer[14];
	int	Who;
	int	Do = -1;
	int	FileIndex = 1;		/* file name index */
	if( Count != 2 )	/* Do not done ! */
		{
		printf("usage:\n   %s <filename>\n",Params[0]);
		exit();
		}
	if( Count > 2 )
		{
		Do = Params[1][0] & 0x0f;
		FileIndex++;
		}
	FileHandle = Open(Params[FileIndex],MODE_OLDFILE);
	if( !FileHandle )	/* failed for any reason */
		{
		printf("Open fails %d\n",IoErr());
		exit();
		}
	while( Read(FileHandle,Buffer,14) == 14 )
		{
		Who = Buffer[0]*256+Buffer[1];
/*
		if( Who & Do )
*/
			switch( Who )
				{
case 0:		/* mouse call */
			prev = x;
			x = Buffer[2]*256+Buffer[3];
			if(	prev == x
				&&
				x == 3
			   )
				count++;
			else
				{
				if( count )
					{
					printf("Mouse : repeated %d time%s\n",
						count,
						(count==1 ? "." : "s.") );
					count = 0;
					}
	printf("Mouse : %-20s(%02x, %02x) %08x %08x %08x %08x %08x\n",
		(x > 30 ? "not known" : Commands[x]),
		Buffer[2],Buffer[3],
		Buffer[4]*256+Buffer[5],
		Buffer[6]*256+Buffer[7],
		Buffer[8]*256+Buffer[9],
		Buffer[10]*256+Buffer[11],
		Buffer[12]*256+Buffer[13]);
				}
			break;
case 1 :
	x = Buffer[2] - 0xf0;
	if( x < 0 )
		x = 999;	/* any value > max. valid */
	printf(" EGA  : %-20s(%02x, %02x) %08x %08x %08x %08x %08x\n",
		( x > 10 ? "not known" : EGACommands[x]),
		Buffer[2],Buffer[3],
		Buffer[4]*256+Buffer[5],
		Buffer[6]*256+Buffer[7],
		Buffer[8]*256+Buffer[9],
		Buffer[10]*256+Buffer[11],
		Buffer[12]*256+Buffer[13]);
	break;		
				}	/* end of switch on Who */
		}
	if( count )
		printf("Mouse : repeated %d times\n",count);
	Close(FileHandle);
	}



