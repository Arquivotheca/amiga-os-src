#ifdef AMIGA

typedef struct IOStdReq SCSIDEVICE;

#else

typedef struct { int fd, interface, addr, lun; } SCSIDEVICE;

#endif

extern SCSIDEVICE *InitSCSI(char *interface, int addr, int lun);
extern void QuitSCSI(SCSIDEVICE *ior);
extern unsigned char sensedata[255];
extern size_t SCSI_Actual, SCSI_CmdLength, SCSI_SenseActual;


extern int DoSCSI(SCSIDEVICE *ior,
		  void *command,
		  size_t clen,
		  void *data,
		  size_t dlen,
		  int flags);

#define SCSI_READ	0
#define SCSI_WRITE	1
