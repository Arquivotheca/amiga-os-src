/*
	SetCPU V1.6
	by Copyright 1989 by Dave Haynie

	DEVICE I/O MODULE

	This module contains functions related to device (specifically
	the floppy) I/O.

*/

#include "setcpu.h"

/* ====================================================================== */

/* This routine turns off the motor. */

void MotorOff(req)
struct IOStdReq *req;
{
   req->io_Length = 0L;
   req->io_Command = TD_MOTOR;
   (void)DoIO((struct IORequest *)req);
}

/* This function fills the buffer "buf" with the contents of the given
   sector number.  Returns 0 if there's no error.  */

BYTE ReadBuf(buf,sect,req)
char *buf;
LONG sect;
struct IOStdReq *req;
{
   req->io_Length = 512L;
   req->io_Data = (APTR) buf;
   req->io_Command = CMD_READ;
   req->io_Offset = (512L * sect);
   (void)DoIO((struct IORequest *)req);
   return req->io_Error;
}

/* This function takes in a DOS name, and returns either the 
   trackdisk.device unit that it corresponds to, or -1L. */
   
LONG CheckTDDev(name)
char *name;
{
   char *devname;
   struct DosLibrary *dl;
   struct RootNode *dr;
   struct DosInfo *di;
   struct DeviceNode *dev;
   struct FileSysStartupMsg *start;
   struct DosEnvec *env;

   dl = (struct DosLibrary *)OpenLibrary("dos.library",0L);
   dr = (struct RootNode *)dl->dl_Root;
   di = (struct DosInfo *)BADDR(dr->rn_Info);
   dev = (struct DeviceNode *)BADDR(di->di_DevInfo);
   CloseLibrary((struct Library *)dl);

  /* Next we find the device */
   if (name[strlen(name)-1] == ':') name[strlen(name)-1] = '\0';

   for (; dev != NULL; dev = (struct DeviceNode *)BADDR(dev->dn_Next)) {
      if (dev->dn_Type != DLT_DEVICE) continue;
      devname = (char *)BADDR(dev->dn_Name);
      if (strniequ(name,devname+1,(unsigned)devname[0])) break;
   }

  /* Is it a valid trackdisk.device? */
   if (!dev) return -1L;
   if (!(start = (struct FileSysStartupMsg *)BADDR(dev->dn_Startup))) return -1L;
   env = (struct DosEnvec *)BADDR(start->fssm_Environ);
   if (env->de_BlocksPerTrack != 11L || env->de_LowCyl != 0L) return -1L;
   devname = (char *)BADDR(start->fssm_Device);
   if (!strniequ(devname+1,"trackdisk.device",16)) return -1L;

   return start->fssm_Unit;
}

