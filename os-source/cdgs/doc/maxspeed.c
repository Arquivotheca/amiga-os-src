
/*

    The AmigaCD has an added feature that allows the CD-ROM drive to
perform in a double-speed mode of operation (300Kb/s) (150 frames/s).  If
you wish to have the drive operate in double-speed, you need to do two
things.  The first thing is very important.  You need to put your data as
close to the outermost tracks as possible.  Double-speed works best when
your data is on the outer tracks.  Carl Sassenrath's ISO tools give you the
ability to pad the beginning of the disk allowing you to place your data as
far out as possible.  The drive will not prohibit you from using
double-speed on the inner tracks of the disk, but you should really use
double-speed on the outer tracks for best results.  The second thing you
need to do is tell the drive to do double-speed.  This is done by the
using the following examples.  Normally, the only command that you may want
to configure for double-speed is the CD_READ command; however, if you are
doing CDXL, you may want to configure the CD_READXL command for
double-speed.

    The MaxReadSpeed() function is used to configure the CD_READ command to
the maximum speed the drive is capable of.  The SetReadXLSpeed() function
is used to configure the CD_READXL command to a desired speed.  If the only
thing you wish to use double-speed for is an XL sequence, then you only
need to use the SetReadXLSpeed() example.  You do not NEED to configure the
CD_READ command to do double-speed as well.  Just make sure that you XL
sequence is on the outer tracks of the disk.

    We hope this will help you in getting the desired performance from your
application.



*/



#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <exec/io.h>
#include <exec/memory.h>
#include <utility/tagitem.h>
#include <devices/cd.h>

#include <clib/exec_protos.h>
#include <pragmas/exec_pragmas.h>

extern struct SysBase *SysBase;

struct IOStdReq *IOR;
struct MsgPort  *Port;

struct CDInfo    CDInfo;
struct TagItem   ConfigList[] = {

    { 0,       0 },
    { TAG_END, 0 }
    };


/***************************************************************************
 *                                                                         *
 *  int MaxReadSpeed(void) -- Set the speed of the READ command to the     *
 *                            maximum that the drive can handle.           *
 *                                                                         *
 ***************************************************************************/

int MaxReadSpeed(void) {

int success = 0;

    if (Port = CreateMsgPort()) {                                           /* Create reply port                */

        if (IOR = CreateIORequest(Port, sizeof(struct IOStdReq))) {         /* Create IORequest                 */

            if (!OpenDevice("cd.device", 0L, IOR, 0L)) {                    /* Open cd.device                   */

                IOR->io_Command = CD_INFO;                                  /* Get maximum drive speed          */
                IOR->io_Data    = (APTR)&CDInfo;
                IOR->io_Offset  = 0;
                IOR->io_Length  = sizeof(struct CDInfo);
                DoIO(IOR);

                if (!IOR->io_Error) {                                       /* Success?                         */

                    ConfigList[0].ti_Tag = TAGCD_READSPEED;                 /* Modify TagList                   */
                    ConfigList[0].ti_Data = CDInfo.MaxSpeed;

                    IOR->io_Command = CD_CONFIG;                            /* Configure drive to maximum speed */
                    IOR->io_Data    = (APTR)&ConfigList;
                    IOR->io_Length  = 0;
                    DoIO(IOR);

                    if (!IOR->io_Error) success = 1;                        /* Report success                   */
                    }

                CloseDevice(IOR);                                           /* Close cd.device                  */
                }

            DeleteIORequest(IOR);                                           /* Delete IORequest                 */
            }

        DeleteMsgPort(Port);                                                /* Delete reply port                */
        }

    return(success);                                                        /* Return success                   */
    }




/***************************************************************************
 *                                                                         *
 *  int SetReadSpeed(int Speed) -- Set the speed of the READXL command to  *
 *                                 the desired frame rate.                 *
 *                                                                         *
 ***************************************************************************/

int SetReadXLSpeed(int Speed) {

int success = 0;

    if (Port = CreateMsgPort()) {                                           /* Create reply port                */
                                                                                                                  
        if (IOR = CreateIORequest(Port, sizeof(struct IOStdReq))) {         /* Create IORequest                 */
                                                                                                                  
            if (!OpenDevice("cd.device", 0L, IOR, 0L)) {                    /* Open cd.device                   */

                ConfigList[0].ti_Tag = TAGCD_READXLSPEED;                   /* Modify TagList                   */
                ConfigList[0].ti_Data = Speed;

                IOR->io_Command = CD_CONFIG;                                /* Configure drive to desired speed */
                IOR->io_Data    = (APTR)&ConfigList;
                IOR->io_Length  = 0;
                DoIO(IOR);

                if (!IOR->io_Error) success = 1;                            /* Report success                   */
                                                                                                                  
                CloseDevice(IOR);                                                                                 
                }                                                           /* Close cd.device                  */
                                                                                                                  
            DeleteIORequest(IOR);                                                                                 
            }                                                               /* Delete IORequest                 */
                                                                                                                  
        DeleteMsgPort(Port);                                                                                      
        }                                                                   /* Delete reply port                */
                                                                                                                  
    return(success);                                                                                              
    }                                                                       /* Return success                   */





/***************************************************************************
 *                                                                         *
 *  Try setting the READ speed to the maximum the drive can handle and try *
 *  setting the speed of the READXL command to 300Kb/s.                    *
 *                                                                         *
 ***************************************************************************/

void main (int argc, char **argv) {

    if (MaxReadSpeed()) printf("Read commands are at maximum speed\n");     /* Set READ commands to max speed   */

    if (!SetReadXLSpeed(150)) printf("Drive can't support double-speed\n"); /* Try to set READXL frame rate to  */
                                                                            /* 150 frames/second (300Kb/s)      */
    }



