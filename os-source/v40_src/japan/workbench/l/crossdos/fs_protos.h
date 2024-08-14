/*** FS_protos.h ****************************************************************
** Copyright 1991 CONSULTRON
*
*       FileSystem porotypes header file
*
****************************************************************************/
               
#ifndef __NOPROTO
#ifndef __PROTO
#define __PROTO(a) a
#endif
#else
#ifndef __PROTO
#define __PROTO(a) ()
#endif
#endif

typedef unsigned char UCHAR;

/* Prototypes for functions defined in cache.c */

struct Cache *AllocCache __PROTO((ULONG numbuffs,
                        ULONG datasize));
ULONG More_Cache __PROTO((ULONG numbuffs,
                        ULONG datasize));
UBYTE *FindBlockInCache __PROTO((ULONG block));
UBYTE *GetBlock __PROTO((ULONG block));
UBYTE *ReadCluster __PROTO((ULONG cluster,
                            struct MLock *lock));
UBYTE *PutBlock __PROTO((ULONG block));
UBYTE *WriteCluster __PROTO((ULONG cluster,
                             struct MLock *lock));
void FlushCache __PROTO((void));
UBYTE *StoreDirEnt __PROTO((struct FS_dir_ent *));


/* Prototypes for functions defined in Cluster.c */

ULONG usedblocks __PROTO((void));
ULONG usedfileblocks __PROTO((LONG clust));
LONG AllocCluster __PROTO((LONG cluster));
LONG FreeClusters __PROTO((LONG cluster));
ULONG ConvertCluster __PROTO((LONG cluster,
                              WORD dirent));


/* Prototypes for functions defined in DeviceIO.c */

BYTE DevCmd __PROTO((UWORD command));
void Motor_Off __PROTO((void));
UBYTE Is_Disk_Out __PROTO((void));
UBYTE AutoReq __PROTO((struct IntuiText *ReqText,
                       struct IntuiText *PosText,
                       struct IntuiText *NegText));
char *FixStringBuff __PROTO((char *buffer));
ULONG Get_Default_Drive_Geometry __PROTO((void));
void Flush_Buffers __PROTO((void));
UBYTE DiskChange __PROTO((void));
BYTE GetBlockMem __PROTO((UBYTE *memory,
                          ULONG block,
                          ULONG num_blocks));
BYTE PutBlockMem __PROTO((UBYTE *memory,
                          ULONG block,
                          ULONG num_blocks));


/* Prototypes for functions defined in DirEnt_MFS.c */

struct FS_dir_ent *GetNextDirEnt __PROTO((struct MFileInfoBlock *mfib));
struct MFileInfoBlock *FindObject __PROTO((struct MLock *lock,
                                           UCHAR *name,
                                           struct MFileInfoBlock *mfib));
int ExamineObject __PROTO((register struct MLock *lock,
                           register struct MFileInfoBlock *mfib));
int ExamineNext __PROTO((register struct MLock *lock,
                         register struct MFileInfoBlock *mfib));
struct FS_dir_ent *FindVolEntry __PROTO((void));
int FindParent __PROTO((register struct MFileInfoBlock *mfib));


/* Prototypes for functions defined in DOS.c */

int SetDate __PROTO((register struct MLock *dirlock,
                     UCHAR *name,
                     struct DateStamp *date));
int Set_Comment __PROTO((register struct MLock *dirlock,
                         UCHAR *name,
                         UCHAR *comment));
int SetProtect __PROTO((register struct MLock *dirlock,
                        UCHAR *name,
                        ULONG attributes));
void SetDate_ParentDir __PROTO((register struct MFileInfoBlock *mfib));
struct FS_dir_ent *AllocDirEnt __PROTO((register struct MFileInfoBlock *mfib));
UBYTE *PutDirEnt __PROTO((register struct FS_dir_ent *dirent,
                       LONG cluster,
                       WORD direntnum));
int RenameObject __PROTO((register struct MLock *fromlock,
                          register UCHAR *fromname,
                          register struct MLock *tolock,
                          register UCHAR *toname));
struct MFileInfoBlock *CreateObject __PROTO((register struct MLock *lock,
                                             UCHAR *name,
                                             register struct MFileInfoBlock *mfib));
struct MLock *CreateDirectory __PROTO((register struct MLock *lock,
                                       UCHAR *name));
int DeleteObject __PROTO((register struct MLock *lock,
                          UCHAR *name));

/* Prototypes for functions defined in DOSinit.c */

int InitDev __PROTO((register struct FileSysStartupMsg *fssm));
void RemDisk __PROTO((void));
void RemDev __PROTO((void));
int ValidateDisk __PROTO((void));


/* Prototypes for functions defined in DOSinitBB_MFS.c */

UBYTE *GetBootBlock __PROTO((register UBYTE *data,
                             register struct DosEnvec *fssm_Env));
int InitDisk __PROTO((void));


/* Prototypes for functions defined in DPswitch_Status.c */
void FileSystem_Status __PROTO((void));


/* Prototypes for functions defined in DPswitch.c */

void FileSystem __PROTO((void));


/* Prototypes for functions defined in fileIO.c */

int OpenFile __PROTO((register struct FileHandle *filehandle,
                      register struct MLock *dirlock,
                      UCHAR *name,
                      int mode));
LONG Set_File_Size __PROTO((register struct MLock *lock,
                            LONG offset,
                            LONG mode));
int CloseFile __PROTO((register struct MLock *lock));
LONG ReadData __PROTO((register struct MLock *lock,
                       register UBYTE *buffer,
                       LONG size));
LONG WriteData __PROTO((register struct MLock *lock,
                        register UBYTE *buffer,
                        LONG size));
LONG SeekFilePos __PROTO((register struct MLock *lock,
                          LONG offset,
                          LONG mode));


/* Prototypes for functions defined in Format_MFS.c */

int FormatDisk __PROTO((UCHAR *diskname,
                        ULONG format));


/* Prototypes for functions defined in handler.c */

void Install_on_FileSysRes __PROTO((BPTR seglist));
BYTE DeleteVolNode __PROTO((register struct DosList *volnode));
void InsertVolNode __PROTO((register struct DosList *newvolnode));
struct DosList *AllocVolNode __PROTO((char *volstr,
                                     struct DateStamp *voldate));
struct DosList *MakeVolNode __PROTO((void));


/* Prototypes for functions defined in Lock.c */

struct MLock *MakeLock __PROTO((struct MLock *lock, 
                                register struct MFileInfoBlock *mfib,
                                LONG mode));
int CompareVolNode __PROTO((register struct MLock *lock));
int CompareVolNode_Write __PROTO((register struct MLock *lock));
int Find_File_Mod __PROTO((register BPTR locklist));
struct MLock *GetLock __PROTO((register struct MLock *lock,
                               UCHAR *name,
                               LONG mode));
struct MLock *CopyLock __PROTO((register struct MLock *lock));
int FreeLock __PROTO((register struct MLock *lock));
struct MLock *Parent __PROTO((register struct MLock *lock));
LONG Change_Mode __PROTO((LONG type, LONG obj, LONG mode));


/* Prototypes for functions defined in misc_MFS.c */

int ConvertVolumeName __PROTO((UCHAR *mfile,
                               UCHAR *bstr));
int ConvertFileName __PROTO((UCHAR *mfile,
                             UCHAR *bstr));
void ConvertFileDate __PROTO((UBYTE *mtimestamp,
                              struct DateStamp *datestamp));
void ConvertToDateStamp __PROTO((register  ULONG seconds,
                                 register struct DateStamp *datestamp));
ULONG ConvertFromDateStamp __PROTO((struct DateStamp *datestamp));
ULONG SetCurrentTime __PROTO((void));
int RenameDisk __PROTO((UCHAR *newname,
                        LONG setdate));


/* Prototypes for functions defined in Sema4_MFS.c */

void Check_Sema4 __PROTO((void));


/* Prototypes for functions defined in debug.lib */
void __stdargs KPrintF( char *fmtstring, ... );


/* Prototypes for functions defined in FAT_MFS.a */
LONG readFATentry __PROTO((struct FS *fsys, LONG fentry));
LONG writeFATentry __PROTO((struct FS *fsys, LONG fentry, LONG newentry));

/* Prototypes for functions defined in string_functions.a */
unsigned char toupper __PROTO((unsigned char));
unsigned char tolower __PROTO((unsigned char));

