/* driver.c
 *
 * Purpose: To access the hddisk device
 *
 */

#include <intuition/intuition.h>
extern	char	rpending;	/* I/O reply still pending */
struct IOStdReq hdrequest;
struct MsgPort HdMsgPort = {0};
char	HDActive = 0;		/* TRUE if the HD is already active */

int HdOpen(unit)
int unit;
{
   int error;
   char *CmdBlkAddr;

/* Open the hddisk device */
   if ((error = OpenDevice("hddisk.device",unit,&hdrequest,0)) != 0) {
      printf("\nhddisk OpenDevice Error: %d.\n",error);
      return(error);
      }
   CmdBlkAddr = (char *)(hdrequest.io_Device);
   CmdBlkAddr += 0x24;
   puts("\nCommand Block address = ");
   putlhex(*(long *)CmdBlkAddr);
   puts("\n");

/* Set up the message port in the IO request */
   HdMsgPort.mp_Node.ln_Type = NT_MSGPORT;
   HdMsgPort.mp_Node.ln_Name = "HDDISK";
   HdMsgPort.mp_Flags = 0;
   HdMsgPort.mp_SigBit = AllocSignal(-1);
   HdMsgPort.mp_SigTask = (struct Task *) FindTask((char *) NULL);
   AddPort(&HdMsgPort);
   hdrequest.io_Message.mn_ReplyPort = &HdMsgPort;
   return(0);
}

void HdClose()		/* Quit using hddisk device */
{
   if (rpending)	/* If previous I/O not complete, wait for it */
	while (GetMsg(&HdMsgPort) == NULL);
   rpending = 0;
   CloseDevice(&hdrequest);
   while (GetMsg(&HdMsgPort) != NULL);
   FreeSignal(HdMsgPort.mp_SigBit);
   RemPort(&HdMsgPort);
}
