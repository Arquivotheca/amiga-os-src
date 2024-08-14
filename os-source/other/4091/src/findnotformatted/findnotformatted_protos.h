/* Prototypes for functions defined in
FindNotFormatted.c
 */

void main(int argc,
          int argv);

int FindUnformattedDrive(char * name);

int IsDrive(struct IOStdReq * ior);

int HasRDB(struct IOStdReq * ior);

int CheckCheckSum(struct RigidDiskBlock * rdb);

int ReadBlock(register struct IOStdReq * ior,
              APTR dest,
              LONG size,
              LONG block);

extern UBYTE sensedata[255];

extern struct SCSICmd cmdblk;

int DoSCSI(register struct IOStdReq * ior,
           UWORD * command,
           ULONG clen,
           UWORD * data,
           ULONG dlen,
           ULONG flags);

void SendSCSI(register struct IOStdReq * ior,
              UWORD * command,
              ULONG clen,
              UWORD * data,
              ULONG dlen,
              ULONG flags);

