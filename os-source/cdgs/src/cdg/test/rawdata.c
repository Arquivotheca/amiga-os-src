/* Test CDG for CDTV-CR */

#include "exec/types.h"
#include "exec/io.h"
#include "dos/dos.h"
#include "hardware/dmabits.h"
#include "hardware/custom.h"
#include "graphics/gfxmacros.h"
#include "clib/exec_protos.h"
#include "clib/dos_protos.h"
#include "/cdg_cr_pragmas.h"
#include "/cdgprefs.h"
#include "cdtv.h"

struct Library *CDGBase;
UBYTE *fire = (UBYTE *)0xbfe001;
UBYTE *mouse = (UBYTE *)0xdff016;

BOOL MakeCDGPrefs(struct CDGPrefs *);
VOID FreeCDGPrefs(struct CDGPrefs *);

ULONG CDGDraw(ULONG);
BOOL CDGBegin();
VOID CDGEnd();
VOID CDGChannel(ULONG);
VOID CDGPlay(BOOL);
VOID CDGPause();
VOID CDGStop();
VOID CDGNextTrack();
VOID CDGPrevTrack();
VOID CDGRewind();
VOID CDGFastForward();
VOID CDGDiskRemoved();
BOOL CDGUserPack( UBYTE *);

UBYTE packsync[2];
UBYTE packdata[96];

UBYTE colorsync[2];
UBYTE colordata[98];

UBYTE testpack[] = {
	9,6,0,0,
	4,2,
	0,0,	/*row,col*/
	0x3A,0x3A,0x3A,0x3A,
	0x3B,0x3B,0x3B,0x3B,
	0x3A,0x3A,0x3A,0x3A,
	0,0,0,0
};

UBYTE testclr[] = {
	9,30,0,0,
	0x1,0x2,0x3,0x4,
	0x5,0x6,0x7,0x8,
	0x9,0xa,0xb,0xc,
	0xd,0xe,0xf,0x10,
	0,0,0,0
};

UBYTE testscroll[] = {
	9,24,0,0,
	0,0,
	0,0,
	0x3A,0x3A,0x3A,0x3A,
	0x3B,0x3B,0x3B,0x3B,
	0x3A,0x3A,0x3A,0x3A,
	0,0,0,0
};


UWORD *baud = (UWORD *)0xdff032;

UWORD *dma = (UWORD *)0xdff096;

int main(int argc, char **argv[])
{
BOOL loop;
UBYTE channel;
UBYTE lastfire,lastmouse;
struct CDGPrefs cdp;
ULONG chars;
UBYTE border,preset;
UBYTE *temp;
int x;
UBYTE highcolor,lowcolor;
UBYTE row,col;
UBYTE pen0,pen1;
struct MsgPort *mp;
struct IOStdReq *ior;
BOOL fail;
ULONG track;
BOOL playing;
UWORD *zero;
ULONG types;
UBYTE coph,copv;
BPTR	fp;
UBYTE packet[26];

/*	*baud = 0x0070; */


	types =3;

	lastfire = 0;
	packdata[0] = 0x9;	/* TV Graphics */
	packdata[24] = 0x0;
	packdata[48] = 0x0;
	packdata[72] = 0x0;

	border = 0x0;
	preset = 0x0;

	colordata[0] = 0x9;	/* TV Graphics */
	colordata[24] = 0x0;
	colordata[48] = 0x0;
	colordata[72] = 0x0;

	highcolor=0;
	lowcolor=0;

	row =0;
	col =0;
	pen0 = 0;
	pen1 = 1;

	fail = FALSE;

	track = 1;
	playing = FALSE;

	coph = 0;
	copv = 0;

	kprintf("STARTCDG\n");
	if(CDGBase = OpenLibrary("cdg.library",38L))
	{
		kprintf("Library open\n");

		if(MakeCDGPrefs(&cdp))
		{
			cdp.cdgp_DisplayX=0;
			cdp.cdgp_DisplayY=0;

			cdp.cdgp_Reserved0=0;
			cdp.cdgp_Reserved1=0;

			cdp.cdgp_SigMask=0L;
			cdp.cdgp_SigTask=FindTask(0L);

			cdp.cdgp_Control=CDGF_ALTSPRITES;

			kprintf("Prefs made\n");
			if(mp=CreateMsgPort())
			{
				kprintf("message port obtained");
				if(ior=CreateIORequest(mp,sizeof(struct IOStdReq)))
				{
					kprintf("IO request obtained");
					if(OpenDevice("cdtv.device",0,ior,0))
					{
						kprintf("Open Device failed");
						fail = TRUE;
					}
					if(fail) DeleteIORequest(ior);
				}
				if(fail) DeleteMsgPort(mp);
			}

			if(fail) return(10);

			if(CDGBegin(&cdp))
			{
				*dma = BITCLR|DMAF_SPRITE;

				CDGFront();
				loop = TRUE;
				channel = 1;

				if(fp=Open("df0:temp",MODE_OLDFILE))
				{
					while(24L==Read(fp,&packet[0],24L))
					{
						CDGUserPack(&packet[0]);
					}

					Close(fp);
				}
				while(loop)
				{
					CDGDraw(types);
					if(*fire != lastfire)
					{
						lastfire = *fire;

						if((*fire & 0x40) == 0x0)
						{
							CDGChannel((channel & 0x0f));

							channel++;

							if(channel > 15) channel = 1;
						}
					}
					lastmouse = *mouse;
					if((lastmouse & 0x04) == 0x0)
					{
						loop=FALSE;
					}

					chars=KMayGetChar();


				if(chars)
				{
					if(chars == (ULONG)'g')
					{
						kprintf("Request Play\n");

						if(!playing)
						{
							ior->io_Command = CDTV_PLAYTRACK;
							ior->io_Data = NULL;
							ior->io_Offset = track;
							ior->io_Length = track+1;

							CDGPlay(TRUE);
							SendIO(ior);
							playing = TRUE;
							kprintf("Play Started\n");
						}
					}

					if(chars == (ULONG)'a')
					{
						if(playing)
						{
							CDGStop();
							AbortIO(ior);
							WaitIO(ior);
							GetMsg(mp);
							kprintf("Play aborted\n");
							playing = FALSE;
						}
					}

					if(chars == (ULONG)'t')
					{
						track++;
						if(track > 10) track=1;
						kprintf("Track=%ld\n",track);
					}

					if(chars == (ULONG)'u')
					{
						CDGPause();
					}
					if(chars == (ULONG)'p')
					{
						CDGPlay(TRUE);
					}
					if(chars == (ULONG)'s')
					{
						CDGStop();
					}
					if(chars == (ULONG)'<')
					{
						CDGPrevTrack();
					}
					if(chars == (ULONG)'>')
					{
						CDGNextTrack();
					}
					if(chars == (ULONG)'f')
					{
						CDGFastForward();
					}
					if(chars == (ULONG)'r')
					{
						CDGRewind();
					}
					if(chars == (ULONG)'e')
					{
						CDGUserPack(testclr);
						for(row=0;row<10;row++)
						{
							for(col=0;col<30;col++)
							{
								testpack[6]=row;
								testpack[7]=col;
								CDGUserPack(testpack);
							}
						}
					}
					if(chars == (ULONG)'-')
					{
						types++;
						if(types > 3) types = 0;
						kprintf("Enabled %ld types\n",(ULONG)types);
					}
					if(chars == (ULONG)'n')
					{
						CDGDiskRemoved();
					}
					if(chars == (ULONG)'3')
					{
						CDGBack();
					}
					if(chars == (ULONG)'4')
					{
						CDGFront();
					}

					if(chars == (ULONG)'5')
					{
						coph++;
						if(coph>2) coph=0;
						kprintf("coph=%ld\n",(long)coph);
					}
					if(chars == (ULONG)'6')
					{
						copv++;
						if(copv>2) copv=0;
						kprintf("copv=%ld\n",(long)copv);
					}
					if(chars == (ULONG)'7')
					{
						testscroll[5] = coph<<4;
						testscroll[6] = copv<<4;
						CDGUserPack(testscroll);
					}
				}
				}
				CDGEnd();
			}
			CloseDevice(ior);
			DeleteIORequest(ior);
			DeleteMsgPort(mp);
			FreeCDGPrefs(&cdp);
		}		
		CloseLibrary(CDGBase);
	}	
	return(TRUE);
}

