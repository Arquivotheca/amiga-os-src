/************************************************************************/
/*                                                                      */
/*      EGCFIOS : EGConvert  OS File System                             */
/*                                                                      */
/*              Designed    by  Y.Katogi        21/05/1991              */
/*              Coded       by  Y.Katogi        21/05/1991              */
/*                                                                      */
/*      (C) Copyright 1991 ERGOSOFT Corp.                               */
/*      All Rights Reserved                                             */
/*                                                                      */
/*                          --- NOTE ---                                */
/*                                                                      */
/*      THIS PROGRAM BELONGS TO ERGO SOFT CORP.  IT IS CONSIDERED A     */
/*      TRADE SECRET AND IS NOT TO BE DIVULGED OR USED BY PARTIES       */
/*      WHO HAVE NOT RECEIVED WRITTEN AUTHORIZATION FROM THE OWNER.     */
/*                                                                      */
/************************************************************************/
#ifdef    UNIX
#include        <stdio.h>
#endif
#include        "edef.h"
#include        "egcenv.h"
#include        "egcerr.h"
#include        "egcdef.h"
#include        "egckcnv.h"
#include        "egcexp.h"
#include        "egcudep.h"
#include        "egcproto.h"

extern REG      egcerrno;

#if defined(UNIX) || defined(MAC)
/********************************************************************/
NONREC
EGCFILE    egcfopen(open_path, type)
    UBYTE          *open_path;
    UBYTE          *type;
/********************************************************************/
{
    return    (fopen(open_path, type));
}

/********************************************************************/
NONREC
BOOL egcfclose(close_fp)
    EGCFILE    close_fp;
/********************************************************************/
{
    return    (fclose(close_fp) == 0)? NORMAL : ERROR;
}
/********************************************************************/
NONREC
BOOL egcfseek(seek_fp, seek_len, seek_pos)
    EGCFILE         seek_fp;
    LONG            seek_len;
    REG             seek_pos;
/********************************************************************/
{
    if (fseek(seek_fp, seek_len, seek_pos) != 0) {
        egcerrno = ERR_FSEEK;
        return (ERROR);
    }
    return (NORMAL);
}
/********************************************************************/
NONREC
BOOL egcfread(read_buf, read_size, read_len, read_fp)
    UBYTE          *read_buf;
    REG             read_size;
    REG             read_len;
    EGCFILE         read_fp;
/********************************************************************/
{
    WORD    count;
    count = fread(read_buf, read_size, read_len, read_fp);
	if (count != read_len) {
        egcerrno = ERR_FREAD;
        return (ERROR);
    }
    if (ferror(read_fp) != 0 || feof(read_fp) != 0) {
        egcerrno = ERR_FREAD;
        return (ERROR);
    }
    return (NORMAL);
}
/********************************************************************/
NONREC
BOOL egcfwrite(write_buf, write_size, write_len, write_fp)
    UBYTE          *write_buf;
    REG             write_size;
    REG             write_len;
    EGCFILE         write_fp;
/********************************************************************/
{
    WORD    count;
    count = fwrite(write_buf, write_size, write_len, write_fp);
    if (count != write_len) {
        egcerrno = ERR_FWRITE;
        return (ERROR);
    }
    return (NORMAL);
}
/********************************************************************/
NONREC
LONG egcftell(write_fp)
    EGCFILE    write_fp;
/********************************************************************/
{
    LONG   lFileSize;
 
   lFileSize = ftell(write_fp);
    if (lFileSize == 0) {
        egcerrno = ERR_FSEEK;
        return (ERROR);
    }
    return (lFileSize);
}
#endif
/*PAGE*/

/*
 * MS-DOS EGConvert file I/O
 */
#ifdef DOS_EGC
/********************************************************************/
NONREC
EGCFILE    egcfopen(open_path, type)
    UBYTE          *open_path;
    WORD            type;
/********************************************************************/
{
    return    (bsfopen(open_path, type));
}

/********************************************************************/
NONREC
BOOL egcfclose(close_fp)
    EGCFILE    close_fp;
/********************************************************************/
{
    return    (bsfclose(close_fp) == 0)? NORMAL : ERROR;
}
/********************************************************************/
NONREC
BOOL egcfseek(seek_fp, seek_len, seek_pos)
    EGCFILE         seek_fp;
    LONG            seek_len;
    REG             seek_pos;
/********************************************************************/
{
    if (bsfseek(seek_fp, seek_len, seek_pos) != 0) {
        egcerrno = ERR_FSEEK;
        return (ERROR);
    }
    return (NORMAL);
}
/********************************************************************/
NONREC
BOOL egcfread(read_buf, read_size, read_len, read_fp)
    UBYTE          *read_buf;
    REG             read_size;
    REG             read_len;
    EGCFILE         read_fp;
/********************************************************************/
{
    WORD    status;
    status = bsfread(read_buf, read_size, read_len, read_fp);
    if (status != NORMAL) {
        egcerrno = ERR_FREAD;
        return (ERROR);
    }
    return (NORMAL);
}
/********************************************************************/
NONREC
BOOL egcfwrite(write_buf, write_size, write_len, write_fp)
    UBYTE          *write_buf;
    REG             write_size;
    REG             write_len;
    EGCFILE         write_fp;
/********************************************************************/
{
    WORD    status;
    status = bsfwrite(write_buf, write_size, write_len, write_fp);
    if (status != NORMAL) {
        egcerrno = ERR_FWRITE;
        return (ERROR);
    }
    return (NORMAL);
}
/********************************************************************/
NONREC
LONG egcftell(write_fp)
    EGCFILE    write_fp;
/********************************************************************/
{
    LONG     lFileSize;

    lFileSize = bsftell(write_fp);
    if (lFileSize == 0) {
        egcerrno = ERR_FSEEK;
        return (ERROR);
    }
    return (lFileSize);
}
#endif                                 /* DOS_EGC */
