/* misc_MFS.c **************************************************************
** Copyright 1991 CONSULTRON
*
*      misc functions.
*
*************************************************************************/
#include "FS:FS.h"

extern struct ExecBase  *SysBase;
extern struct DOSBase *DOSBase;
extern struct Library *UtilityBase;
extern struct FS         *fsys;
extern int DOSerror;
extern struct FS_dir_ent *AllocDirEnt();
extern struct trans_table *trans_tbl;


/**********************************************************************
*   ConvertVolumeName() -- Convert MSDOS Volume name to BSTR.
**********************************************************************/
int ConvertVolumeName(mfile, bstr)
UCHAR *mfile;
UCHAR *bstr;
{
    UCHAR *i = &bstr[1], *j, c;
    BYTE k;
  
    for(k=0, j=i; k < FNAME_EXT_SIZE; k++)
    {       /* copy MSDOS volume name */
        c=mfile[k];
        if((k==0) && (c == 0x05)) c = FN_ERASE;     /* change 0xE5 to 0x05 for first char only */
        *(i++) = Trans_M_A(c,trans_tbl);
        if(( c != ' ') && ( c != '\0')) j=i;
    }
    i = min(j,i);
    *(i) = '\0';     /* For future compatability (I guess) */

    return((int)(bstr[0] = i-(&bstr[1])));
}

#define STRLEN_BACKGROUND 11


UCHAR *info = ".info";
#define STRLEN_INFO 5
/* For a possible future feature
static const *background = ".background";
#define STRLEN_BACKGROUND 11
*/

/**********************************************************************
*   ConvertFileName() -- Convert MSDOS file name and extension to BSTR.
**********************************************************************/
int ConvertFileName(mfile, bstr)
UCHAR *mfile;
UCHAR *bstr;
{
F();
    register UCHAR *i= &bstr[1], *j, c;
    BYTE k;
  
    for(k=0, j=i; k < FNAME_SIZE; k++)    /* copy MSDOS file name */
    {
/* This one is kind of strange.  In order to use the 0xE5 char for international chars
   the 0x05 char is used because the 0xE5 char is used to describe FileName Erased (FN_ERASE) 
   in only the first char location */
        c=mfile[k];
        if((k==0) && (c == 0x05)) c = FN_ERASE;     /* change 0xE5 to 0x05 for first char only */
        *(i++) = Trans_M_A(c,trans_tbl);
        if( c != ' ') j=i;
    }
    i = min(j,i);
        
    if((*((ULONG *)&(mfile[FNAME_SIZE])) & 0xFFFFFF00) != 0x20202000 )  /* extension not blank */
    {
        *(i++) = '.';                       /* add period */

/* Using the utility.library equivalent to stricmp() because with localization
it should handle case for international characters properly */
/* Support the ".info" file ext */
        if( !strnicmp(&mfile[FNAME_SIZE], "INF",FEXT_SIZE))
        {
D();
            strcpy(i,&info[1]);             /* change .INF to .info */
            i += STRLEN_INFO-1;             /* MUST be lowercase !! */
        }
/* For a possible future feature
/* Support the ".background" file ext */
        else if( !strnicmp(&mfile[FNAME_SIZE], "BAC",FEXT_SIZE))
        {
            strcpy(i,&background[1]);           /* change .BAC to .background */
            i += STRLEN_BACKGROUND-1;           /* MUST be lowercase !! */
        }
*/
         else
        {
            for(; k < FNAME_EXT_SIZE; k++)
            {
                *(i++) = Trans_M_A(c=mfile[k],trans_tbl);
                if( c != ' ') j=i;
            }
            i = min(j,i);
        }
    }
    *(i) = '\0';     /* For future compatability (I guess) */

    return((int)(bstr[0] = i-(&bstr[1])));
}


#define FEB         2   /* # Month */
#define MINS_HR     60

#define SECS_PER_DAY    86400
#define DAYS_78to70     2922

#include "utility/date.h"
#include "clib/utility_protos.h"
#include "pragmas/utility_pragmas.h"
/**********************************************************************
*   ConvertFileDate() -- Convert MSDOS file date to DateStamp struct.
**********************************************************************/
void ConvertFileDate(mtimestamp, datestamp)
UBYTE *mtimestamp;
struct DateStamp *datestamp;
{
    UWORD volatile mtime;
    UWORD volatile mdate;

    struct ClockData date = {0,0,0};

    SWAPWORD(mtime,mtimestamp[0]);
    SWAPWORD(mdate,mtimestamp[2]);

        /* convert hours into  # minutes */
    datestamp->ds_Minute = ((mtime&TIMEM_HOURS)>>TIMEB_HOURS) * MINS_HR;

        /* get # minutes */
    datestamp->ds_Minute += ((mtime&TIMEM_MINS)>>TIMEB_MINS);

        /* convert each 2 secs into # ticks */
    datestamp->ds_Tick = (mtime&TIMEM_2SECS)* (2 * TICKS_PER_SECOND);

    date.month = (mdate&DATEM_MONTH)>>DATEB_MONTH;
    date.mday  = mdate&DATEM_DAY;
    date.year  = ((mdate&DATEM_YEAR)>>DATEB_YEAR)+1980;

    datestamp->ds_Days = Date2Amiga(&date)/SECS_PER_DAY;

}

/**********************************************************************
*   ConvertFromDateStamp() -- Convert DateStamp struct to MSDOS file date and time
**********************************************************************/
ULONG ConvertFromDateStamp(datestamp)
struct DateStamp *datestamp;
{
    ULONG volatile mtimestamp;
    UWORD volatile mtime;
    UWORD volatile mdate;
    struct ClockData time;

    mtime =   (((datestamp->ds_Minute)/60)<<TIMEB_HOURS) /* convert hours */
            + (((datestamp->ds_Minute)%60)<<TIMEB_MINS)  /* convert minutes */
            + (((datestamp->ds_Tick)/TICKS_PER_SECOND)>>TIMEB_2SECS);
                                                        /* convert seconds */

    Amiga2Date((datestamp->ds_Days)*SECS_PER_DAY,&time);

    mdate =   ((time.year-1980)<<DATEB_YEAR)        /* convert years */
            + (time.month<<DATEB_MONTH)             /* convert months */
            + (time.mday);                          /* convert days */
 
      SWAPWORD(((UBYTE *)&mtimestamp)[0],mtime);
      SWAPWORD(((UBYTE *)&mtimestamp)[2],mdate);
    return(mtimestamp);
}


/**********************************************************************
*   SetCurrentTime() -- Set current date and time to MSDOS directory entry
**********************************************************************/
ULONG SetCurrentTime()
{
    struct DateStamp ds;

    DateStamp(&ds);
    return(ConvertFromDateStamp(&ds));
}


/**********************************************************************
*   RenameDisk()  --- Rename Disk.  Optional setdate flag
*
*   If newname = 0, then use the current name.
*   This routine is also use to support ACTION_FORMAT and ACTION_SERIALIZE_DISK
**********************************************************************/
int RenameDisk(newname,setdate)
UCHAR *newname;    /* BADDR(BSTR) */
LONG setdate;       /* setdate flag. NO=DOSFALSE; YES=DOSTRUE */
{
    register struct MFileInfoBlock *mfib = (struct MFileInfoBlock *)fsys->f_scratch;
    register struct FS_dir_ent    *dirent;
    UBYTE   strsize=0;
    UCHAR *VolNodeName;
    int i;

    if(!CompareVolNode_Write(0)) return(DOSFALSE);

/* find if new object name already exists */
    if( !(dirent = (struct FS_dir_ent *)FindVolEntry()) )
    {
        if(newname == 0)    /* No name supplied */
        {
            newname = fsys->f_sys_id;
        }
/* Volume Label NOT found in root directory. Create volume label */
        mfib->mfib_DiskCluster = ROOTDIR_CLUST;
        mfib->mfib_DirEntStatus = MLDE_DIR;
        mfib->mfib_DiskDirEntry = -1L;  /* start at the beginning of the directory */

        if( !( dirent = AllocDirEnt(mfib)) )
        {       /* no more directory entries available in root dir */
            return(DOSFALSE);
        }
            /* dir entry found */
        dirent->fde_protection = ATTF_VOL_LBL|ATTF_ARCHIVE; /* make entry Volume label attrib */
    }
/* Put new volume name in directory entry but first clear name with spaces */
    if(newname)
    {
        strsize = min(FNAME_TOTALSZ,newname[0]);
        memset( dirent->fde_filename, ' ', FNAME_TOTALSZ); /* set to all spaces */

        for(i=0; i<strsize; i++)
        {
                /* Set newname to uppercase.  MSDOS does not like lowercase */
            (dirent->fde_filename)[i] = Trans_A_M(toupper(newname[i+1]),trans_tbl);
        }
    }

/* set NULL at end of volname and new volume date (if option set) */
/**** Set Current Date and Time on Directory ****/
    if(setdate)
    {
        DateStamp(&(fsys->f_vol_date));    /* set volume date in FS struct */
        *(ULONG *)(dirent->fde_time) = ConvertFromDateStamp(&(fsys->f_vol_date));
    }

    if( !StoreDirEnt(dirent))
    {
       return(DOSFALSE);
    }

/* correct DosList VolumeNode if available */
    if(fsys->f_VolumeNode  && (fsys->f_VolumeNode->dol_Type == DLT_VOLUME))
    {
        VolNodeName = (UCHAR *)BADDR(fsys->f_VolumeNode->dol_Name);
    /* Put new volume name in Volume Node */
        CopyMem( dirent->fde_filename, (UCHAR *)&(VolNodeName[1]),
            VolNodeName[0] = strsize);

        VolNodeName[strsize+1] = '\0';  /* make sure NULL is at the end of the volume name */

        if(setdate != DOSFALSE)
        {
        /* Put new volume date in Volume Node */
            ConvertFileDate( dirent->fde_time,
                &(fsys->f_VolumeNode->dol_misc.dol_volume.dol_VolumeDate));
        }
    }
    return(DOSTRUE);
}
