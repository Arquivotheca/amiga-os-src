/*
 * $Id: objectio.c,v 38.1 91/06/24 19:02:06 mks Exp $
 *
 * $Log:	objectio.c,v $
 * Revision 38.1  91/06/24  19:02:06  mks
 * Changed to V38 source tree - Trimmed Log
 * 
 */

#include <exec/types.h>
#include <exec/memory.h>
#include <libraries/dos.h>
#include <workbench/workbench.h>
#include <string.h>

#include "icon.h"
#include "internal.h"
#include "support.h"

putImage(LONG file, struct Image *im )
{
    int result = TRUE;

    MP(("putImage: enter, file=$%lx, image=$%lx\n", file, im));
    if (im)
    {
	im->PlanePick=(1L << im->Depth)-1;
	im->PlaneOnOff=0;

	if (result = IWrite(file, im, sizeof(struct Image)))
	{
	    result = IWrite(file, im->ImageData, RASSIZE(im->Width, im->Height) * im->Depth);
	}
    }
    MP(("putImage: exit, returning %ld\n", result));
    return(result);
}


struct Image *getImage(struct FreeList *free, LONG file, struct Image *im, struct Image *srcim )
{
    DP(("getImage: free=$%lx, file=$%lx, im=$%lx, srcim=$%lx\n",free, file, im, srcim));
    if (im)
    {
	im = TFLAlloc(free,sizeof(struct Image),MEMF_CLEAR|MEMF_PUBLIC,struct Image *);
	if (!im) im=(struct Image *)(-1);
	else
	{
	    /* David Berezowski - Jun/90 */
	    if (srcim)
	    {
	        DP(("GI: copying %ld bytes from $%lx to $%lx\n",sizeof(struct Image), srcim, im));
		*im=*srcim;
	    }
	    else if(!IRead(file,im,sizeof(struct Image))) im=(struct Image *)(-1);

	    if (im!=(struct Image *)-1)
	    {
            int len = RASSIZE( im->Width, im->Height ) * im->Depth;

		im->PlanePick=(1L << im->Depth)-1;
		im->PlaneOnOff=0;

                im->ImageData=TFLAlloc(free,len,MEMF_PUBLIC|MEMF_CHIP|MEMF_CLEAR,SHORT *);

                /* David Berezowski - Sept 12/88 */
                if (!im->ImageData) im=(struct Image *)(-1);
		else
		{
                    /* David Berezowski - Jun/90 */
                    if (srcim)
                    {
                        DP(("GI: copying %ld bytes from $%lx to $%lx\n",len, srcim->ImageData, im->ImageData));
                        memcpy(im->ImageData,srcim->ImageData,len);
                    }
                    else
                    {
                        if(!IRead(file,im->ImageData,len)) im=(struct Image *)(-1);
                        else
                        {
                            /*
                             * mks:  Make sure that the image offsets are valid
                             */
                            im->LeftEdge=0;
                            im->TopEdge=0;
                        }
                    }
                }
	    }
	}
    }
    return(im);
}
