/*
 * A quck program to disable the clicking of drives under 2.0
 *
 * Sunday 09-Sep-90 21:03:36 Bryce Nesbitt
 *
 *  lc -csq -Ltlhsv -ms -O -v noclick
 */
#include <exec/types.h>
#include <exec/libraries.h>
#include <devices/trackdisk.h>
#include <libraries/dos.h>
#include <proto/exec.h>

int CXBRK(void) { return(0); }  /* Disable Lattice CTRL-C handling */

extern struct Library * SysBase;

void _main()
{
struct MsgPort *trackport;
struct IOExtTD *trackIO;
int             i;

if( SysBase->lib_Version >= 36 )
{
    if ( trackport=CreatePort(0L,0L) )
    {
        if ( trackIO=(struct IOExtTD *)
        CreateExtIO(trackport,(long)sizeof(struct IOExtTD)) )
        {
            for (i=0; i<4; i++)
                {
                if ( !OpenDevice("trackdisk.device",i,trackIO,0) )
                    {
                    ((struct TDU_PublicUnit *)trackIO->iotd_Req.io_Unit)->tdu_PubFlags |= TDPF_NOCLICK;
                    CloseDevice(trackIO);
                    }
                }

            DeleteExtIO(trackIO);
        }
        DeletePort(trackport);
    }
}
}
