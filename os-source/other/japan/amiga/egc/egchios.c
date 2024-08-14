/************************************************************************/
/*                                                                      */
/*      EGCHIOS : EGConvert pHysical I/O System                         */
/*                                                                      */
/*              Designed    by  Y.Katogi        10/12/1990              */
/*              Coded       by  Y.Katogi        10/12/1990              */
/*              Modified    by  H.Yanata        05/12/1991              */
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
#ifdef UNIX
#include        <stdio.h>
#include        <sys/types.h>
#include        <sys/stat.h>
#include        <pwd.h>
#include        <dirent.h>
#include        <fcntl.h>
#include        <sys/param.h>
#define         ENV_EXPDIC      "EGC_EXPDIC"
#endif

#ifdef DOS_EGC
#include        <string.h>
#endif

#ifndef MAXPATHLEN
#define MAXPATHLEN  128
#endif

#include        "edef.h"
#include        "egcenv.h"
#include        "egcerr.h"
#include        "egcdef.h"
#include        "egckcnv.h"
#include        "egcexp.h"
#include        "egcudep.h"
#include        "egcproto.h"

UBYTE          *grtp;
extern UBYTE    maxslt;
extern UBYTE    maxdso;
extern UBYTE   *dbheap;
extern UBYTE   *dbftbl[];
static int      dos_egb_on = FALSE;

#ifndef DOS_EGBRIDGE    /* No use on original DOS EGBridge */
UBYTE           EXPDIR[MAXPATH];
static int      egcdefaultdicflags = TRUE;
#ifdef  DOS_EGC
static UBYTE    egcuserdicname[DICNAMELEN] = "\\egdicu.dic";
static UBYTE    egcmaindicname[DICNAMELEN] = "\\egdicm.dic";
#else
static UBYTE    egcuserdicname[DICNAMELEN] = "egdicu.dic";
static UBYTE    egcmaindicname[DICNAMELEN] = "egdicm.dic";
#endif
REG             egcerrno = 0;
extern DICS     dics[MAXOPENDIC];
extern int      errno;
#endif  /* DOS_EGBRIDGE */

/*
*** PL/DS buffering mode definitions
*/
#define     NOBFR       0
#define     PLBFR       1
#define     DSBFR       2

/*
*** unique value not easily much
*/
#define     UVHIGH      2147483647
#define     UVLOW       -1L

/*
*** Disk I/O control definitions
*/
#define     RSEG        0
#define     WSEG        1

/*------------------------------------------------------------------*/
/* initdict : initialize dictionaru management structure            */
/*                                                                  */
/* return                                                           */
/*      NORMAL : initialize success                                 */
/*      ERROR  : memory allocated failure                           */
/*------------------------------------------------------------------*/
NONREC
BOOL    initdict()
{
    register UWORD  i;

#ifndef DOS_EGBRIDGE
    for (i = 0; i < MAXOPENDIC; i++) {
        dics[i].id = (UBYTE) (i + 1);
        dics[i].name[0] = '\0';
    }
#endif
    for (i = 0; i < MAXOPENDIC; i++) {
        dicstat[i].idno = -1;
        if ((dicstat[i].pds =
             (PTR_MDICT) egcnewptr(sizeof(MDICT_INFO))) == (PTR_MDICT)NULL)
            return (ERROR);
    }
    for (i = 0; i < MAXAPENDIC; i++) {
        dicprty[i].idno = -1;
        dicprty[i].pds = 0;
    }
    return (NORMAL);
}

/*------------------------------------------------------------------*/
/* opendict : open main dictionary (main procedure)                 */
/*                                                                  */
/* return                                                           */
/*      NORMAL : open success                                       */
/*      ERROR  : open failure                                       */
/*------------------------------------------------------------------*/
NONREC
BOOL    opendict(pdarry)
    PTR_MDICT       pdarry;
{
    if ((openmseg(0, pdarry) != NORMAL) || (getgseg(pdarry) != NORMAL)) {
        closemseg(0, pdarry);
        return (ERROR);
    }
    return (NORMAL);
}                                      /*--- end of opendict ---*/
/*PAGE*/

/*------------------------------------------------------------------*/
/* openmseg : open main dictionary (opendict sub procedure)         */
/*                                                                  */
/* return                                                           */
/*      NORMAL : open success                                       */
/*      ERROR  : memory allocation failure or no dictionary         */
/*------------------------------------------------------------------*/
NONREC
BOOL openmseg(idno, pdarry)
    WORD            idno;
    PTR_MDICT       pdarry;
{
    if (idno == 0) {
        if ((dbheap = (UBYTE*)egcnewptr(HEAPSIZE)) == (UBYTE*)NULL)
            return (ERROR);
        if (bfrinit(pdarry) != NORMAL) {
            return ERROR;
        }
    }
    if ((readmafp(pdarry) == NORMAL) &&
        (readmaidx(pdarry) == NORMAL) &&
        (readmapl(pdarry) == NORMAL)) {
        pdarry->suspended = (UBYTE)ACTIVE;
        return (NORMAL);
    }
    closemseg(idno, pdarry);
    return (ERROR);
}
/*PAGE*/

/*------------------------------------------------------------------*/
/* readmafp : get main dictionary file hundle (opendict sub)        */
/*                                                                  */
/* return                                                           */
/*      NORMAL : success                                            */
/*      ERROR  : cannot open dictionary                             */
/*------------------------------------------------------------------*/
NONREC
BOOL readmafp(pdarry)
    PTR_MDICT       pdarry;
{
#ifdef    UNIX
    struct stat     stbuf;
#endif

    if (pdarry->fp == (EGCFILE)0) {
#ifdef    UNIX
        if (stat(pdarry->path, &stbuf) == -1) {
#ifndef DOS_EGBRIDGE
            egcerrno = ERR_FNOTFOUND;
#endif
            return (ERROR);
        }
#endif
        if ((pdarry->fp = egcfopen((UBYTE*)pdarry->path, READMODE))
            == (EGCFILE)0) {
#ifndef DOS_EGBRIDGE
            egcerrno = ERR_CANTOPEN;
#endif
            return (ERROR);
        }
    }
    else {
        dos_egb_on = TRUE;
    }
    return (NORMAL);
}

/*------------------------------------------------------------------*/
/* readmaidx : get main dictionary header index (opendict sub proc) */
/*                                                                  */
/* return                                                           */
/*      NORMAL : success                                            */
/*      ERROR  : disk I/O error                                     */
/*------------------------------------------------------------------*/
NONREC
BOOL readmaidx(pdarry)
    PTR_MDICT       pdarry;
{
    UBYTE   IObuf[DSBLKL];

    if (egcIOctrl(pdarry->fp, IObuf, 0L, 128, RSEG))
        return  ERROR;
    pdarry->hbsegq = ((PMH)IObuf)->mhsegq
        + ((UWORD)(((PMH)IObuf)->mhsegqh) << 8);
    pdarry->ploff = (LONG)getint(((PMH)IObuf)->mhploff) * 16;
    pdarry->dsoff = (LONG)getint(((PMH)IObuf)->mhdsoff) * 16;
    pdarry->gtoff = (LONG)(pdarry->hbsegq) * DSBLKL + pdarry->dsoff;
    egcmemmove(pdarry->strver, IObuf + MD_STR_OFFSET, EGCSYSVERLEN);
    egcmemmove(pdarry->dicver, IObuf + MD_VER_OFFSET, EGCSYSVERLEN);
    return (NORMAL);
}

/*------------------------------------------------------------------*/
/* readmapl : get PL record (opendict sub procedure)                */
/*                                                                  */
/* return                                                           */
/*      NORMAL : success                                            */
/*      ERROR  : memory allocation failure or disk I/O error        */
/*------------------------------------------------------------------*/
NONREC
BOOL readmapl(pdarry)
    PTR_MDICT       pdarry;
{
#ifdef UNIX
    if ((pdarry->plp = (UBYTE*)egcnewptr(pdarry->dsoff)) == (UBYTE*)NULL) {
        egcerrno = ERR_MALLOC;
        return (ERROR);
    }
    if (egcIOctrl(pdarry->fp, pdarry->plp, 0L, pdarry->dsoff, RSEG))
        return  ERROR;
#endif
    return (NORMAL);
}
/*PAGE*/

/*------------------------------------------------------------------*/
/* openuseg : open user dictionary (main procedure)                 */
/*                                                                  */
/* return                                                           */
/*      NORMAL : open success                                       */
/*      ERROR  : open failure                                       */
/*------------------------------------------------------------------*/
NONREC
BOOL openuseg(idno, pdarry)
    WORD            idno;
    PTR_MDICT       pdarry;
{
    UDDH           *hdrp;

    if (idno == 0) {
        dicprty[0].pds = pdarry;
        if (readusfp(pdarry) == ERROR)
            return (ERROR);
    }
    hdrp = &pdarry->hbuhdr;
    if (readusidx(hdrp, idno) == ERROR)
        return (ERROR);
    if (idno == 0) {
        if ((readusslt(hdrp, 0) == NORMAL) &&
            (readusplo(hdrp, 0) == NORMAL) &&
            (readusdso(hdrp) == NORMAL)) {
            pdarry->hbsegq = dicstat[0].pds->hbsegq;
            pdarry->ploff = dicstat[0].pds->ploff;
            pdarry->dsoff = dicstat[0].pds->dsoff;
            pdarry->gtoff = dicstat[0].pds->gtoff;
            egcmemmove(pdarry->strver, hdrp->udstrver, EGCSYSVERLEN);
            egcmemmove(pdarry->dicver, hdrp->uddicver, EGCSYSVERLEN);
            pdarry->suspended = (UBYTE)ACTIVE;
#ifdef  UNIX
            pdarry->plp = dicstat[0].pds->plp;
            pdarry->last = time(0);
#endif
        }
        else {
            closeuseg(0, pdarry);
            return (ERROR);
        }
    }
    else {
        hdrp->udsltp = hdrp->udplop = hdrp->uddsop = (UBYTE*)NULL;
#ifdef    UNIX
#ifdef    NDG
        pdarry->last = 0;
#else
        pdarry->last = 0L;
#endif
#endif
    }
#ifndef DOS_EGBRIDGE
    CheckUI341(pdarry);
#endif
    return (NORMAL);
}
/*PAGE*/

/*------------------------------------------------------------------*/
/* readusfp : get user dictionary file hundle (openuseg sub proc)   */
/*                                                                  */
/* return                                                           */
/*      NORMAL : success                                            */
/*      ERROR  : cannot open dictionary                             */
/*------------------------------------------------------------------*/
NONREC
BOOL readusfp(pdarry)
    PTR_MDICT       pdarry;
{
   if (pdarry->fp == (EGCFILE)0) {
        if ((pdarry->fp = egcfopen((UBYTE *) pdarry->path, READMODE))
            == (EGCFILE)0) {
#ifndef DOS_EGBRIDGE
            egcerrno = ERR_CANTOPEN;
#endif
            return (ERROR);
        }
    }
    return (NORMAL);
}

/*------------------------------------------------------------------*/
/* readusidx : get global index (openuseg sub procedure)            */
/*                                                                  */
/* return                                                           */
/*      NORMAL : open success                                       */
/*      ERROR  : memory allocation failure or disk I/O error        */
/*------------------------------------------------------------------*/
NONREC
BOOL readusidx(hdrp, idno)
    WORD            idno;
    UDDH           *hdrp;
{
    UBYTE           iobuf[128];

    if (getuseg(idno, iobuf, (UWORD) 1, 0) != NORMAL)
        return (ERROR);
    egcmemmove((UBYTE *) hdrp, iobuf, sizeof(UDDH));
    if (hdrp->udsegq < 0xFF)
        setint(hdrp->uesegq, (UWORD) (hdrp->udsegq) & 0x00FF);
    if (hdrp->busyNum > DEFNUM)
        hdrp->busyNum = DEFNUM;
    if ((hdrp->udidxp =
         (PIE) egcnewptr((REG) hdrp->udidxl * UDBLKL)) == (PIE)NULL) {
#ifndef DOS_EGBRIDGE
        egcerrno = ERR_MALLOC;
#endif
        return (ERROR);
    }
    if (getuseg(idno, (UBYTE *) hdrp->udidxp, hdrp->udidxl,
                (UWORD) hdrp->udidxs))
        return (ERROR);
    return (NORMAL);
}

/*------------------------------------------------------------------*/
/* readusdso : alloc memory for DSO pointer (openuseg sub proc)     */
/*                                                                  */
/* return                                                           */
/*      NORMAL : memory allocation success                          */
/*      ERROR  : memory allocation failure                          */
/*------------------------------------------------------------------*/
NONREC
BOOL readusdso(hdrp)
    UDDH           *hdrp;
{
    if ((hdrp->uddsop = (UBYTE*)egcnewptr(DSBLKL)) == (UBYTE*)NULL) {
#ifndef DOS_EGBRIDGE
        egcerrno = ERR_MALLOC;
#endif
        return (ERROR);
    }
    return (NORMAL);
}

/*------------------------------------------------------------------*/
/* readusslt : get SLT block (openuseg sub procedure)               */
/*                                                                  */
/* return                                                           */
/*      NORMAL : success                                            */
/*      ERROR  : memory allocation failure or disk I/O error        */
/*------------------------------------------------------------------*/
NONREC
BOOL readusslt(hdrp, idno)
    WORD            idno;
    UDDH           *hdrp;
{
    REG             usltl, uslts;
#ifndef DOS_EGBRIDGE
    LONG            sys_ver_nbr, dic_ver_nbr;
#endif

    if (hdrp->udslts == 255 && hdrp->udsltl == 255) {
        usltl = (UWORD) getint(hdrp->uesltl);
        uslts = (UWORD) getint(hdrp->ueslts);
    }
    else {
        usltl = (REG) hdrp->udsltl;
        uslts = (REG) hdrp->udslts;
        setint(hdrp->uesltl, (UWORD) usltl);
        setint(hdrp->ueslts, (UWORD) uslts);
    }
#ifndef DOS_EGBRIDGE
    sys_ver_nbr = egcgetsysverno();
    dic_ver_nbr = egcgetmainstrver(NULL);
#endif
    if (idno == 0) {
        maxslt = (UBYTE) ((UDBLKL * usltl) /
            (2 * getint(dicprty[0].pds->hbuhdr.uesegq)));
#ifdef CANON
        if ( !(maxdso = hdrp->udmaxdso) || maxdso > DSOEXT)
            maxdso = DSOEXT;
#endif
    }

#ifdef    PMSLT
    if ((hdrp->udsltp = 
        (UBYTE*)egcnewptr(MAXSLTVAL * 2)) == (UBYTE*)NULL) {
#ifndef DOS_EGBRIDGE
        egcerrno = ERR_MALLOC;
#endif
        return (ERROR);
    }
#else
    if ((hdrp->udsltp = (UBYTE*) egcnewptr(usltl * UDBLKL)) == (UBYTE*)NULL) {
#ifndef DOS_EGBRIDGE
        egcerrno = ERR_MALLOC;
#endif
        return (ERROR);
    }
    if (getuseg(idno, (UBYTE *) hdrp->udsltp, usltl, (UWORD) uslts))
        return (ERROR);
#endif
    return (NORMAL);
}

/*------------------------------------------------------------------*/
/* readusplo : get PLO block (openuseg sub procedure)               */
/*                                                                  */
/* return                                                           */
/*      NORMAL : success                                            */
/*      ERROR  : memory allocation failure or disk I/O error        */
/*------------------------------------------------------------------*/
NONREC
BOOL readusplo(hdrp, idno)
    WORD            idno;
    UDDH           *hdrp;
{
    REG             uplol, uplos;

    if ((hdrp->udplos == 255) && (hdrp->udplol == 255)) {
        uplol = (REG) getint(hdrp->ueplol);
        uplos = (REG) getint(hdrp->ueplos);
    }
    else {
        uplol = (REG) hdrp->udplol;
        uplos = (REG) hdrp->udplos;
        setint(hdrp->ueplol, (UWORD) uplol);
        setint(hdrp->ueplos, (UWORD) uplos);
    }

#ifdef  PMPLO
    if ((hdrp->udplop = (UBYTE*)egcnewptr(MAXPLOL)) == (UBYTE*)NULL) {
#ifndef DOS_EGBRIDGE
        egcerrno = ERR_MALLOC;
#endif
        return (ERROR);
    }
    dicprty[0].pds->plosof = UVHIGH;;
    dicprty[0].pds->ploeof = UVLOW;
#else
    if ((hdrp->udplop = (UBYTE*)egcnewptr(uplol * UDBLKL)) == (UBYTE*)NULL) {
#ifndef DOS_EGBRIDGE
        egcerrno = ERR_MALLOC;
#endif
        return (ERROR);
    }
    if (getuseg(idno, (UBYTE*)hdrp->udplop, uplol, (UWORD)uplos) != NORMAL)
        return (ERROR);
#endif
    return (NORMAL);
}
/*PAGE*/

/*------------------------------------------------------------------*/
/* closedict : close main or expert dictionary (main procedure)     */
/*                                                                  */
/* return                                                           */
/*      VOID                                                        */
/*------------------------------------------------------------------*/
NONREC
BOOL    bfrinit(pdarry)
    PTR_MDICT   pdarry;
{
    if ((pdarry->bfrp = (UBYTE*)egcnewptr(PLBLKL * 2)) == (UBYTE*)NULL)
        return  ERROR;
    pdarry->bfrmod = NOBFR;
    pdarry->bfrsof = UVHIGH;
    pdarry->bfreof = UVLOW;
    pdarry->bfrid  = -1;
    return  NORMAL;
}

/*------------------------------------------------------------------*/
/* closedict : close main or expert dictionary (main procedure)     */
/*                                                                  */
/* return                                                           */
/*      VOID                                                        */
/*------------------------------------------------------------------*/
NONREC
VOID    closedict()
{
    WORD            idno;
    PTR_MDICT       pdarry;

    for (idno = 0; idno < MAXOPENDIC; idno++) {
        pdarry = dicstat[idno].pds;
        if (pdarry) {
            closemseg(idno, pdarry);
            (VOID) egcfreeptr((UBYTE *) pdarry);
            pdarry = (PTR_MDICT)NULL;
        }
    }
}                                      /*--- end of closedict ---*/
/*PAGE*/

/*------------------------------------------------------------------*/
/* closemseg : free memory and close dictionary (closedict sub proc)*/
/*                                                                  */
/* return                                                           */
/*      VOID                                                        */
/*------------------------------------------------------------------*/
NONREC
VOID    closemseg(idno, pdarry)
    WORD            idno;
    PTR_MDICT       pdarry;
{
    if (idno == 0) {
        if (dbheap) {
            (VOID) egcfreeptr(dbheap);
            dbheap = (UBYTE*)NULL;
        }
        if (grtp) {
            (VOID) egcfreeptr(grtp);
            grtp = (UBYTE*)NULL;
        }
        if (pdarry->bfrp) {
            (VOID) egcfreeptr(pdarry->bfrp);
            pdarry->bfrp = (UBYTE*)NULL;
        }
    }
#ifdef  UNIX
    if (pdarry->plp) {
        (VOID) egcfreeptr((UBYTE *) pdarry->plp);
        pdarry->plp = (UBYTE*)NULL;
    }
#endif
    if (dos_egb_on == FALSE && pdarry->fp) {
        (VOID)egcfclose(pdarry->fp);
        pdarry->fp = (EGCFILE)0;
    }
}

/*------------------------------------------------------------------*/
/* closeuseg : close user dictionary (main procedure)               */
/*                                                                  */
/* return                                                           */
/*      VOID                                                        */
/*------------------------------------------------------------------*/
NONREC
VOID    closeuseg(idno, pdarry)
    WORD            idno;
    PTR_MDICT       pdarry;
{
    if (pdarry->hbuhdr.udidxp) {
        (VOID) egcfreeptr((UBYTE *) pdarry->hbuhdr.udidxp);
        pdarry->hbuhdr.udidxp = (PIE)NULL;
    }
    if (idno == 0) {
        if (pdarry->hbuhdr.udsltp) {
            (VOID) egcfreeptr((UBYTE *) pdarry->hbuhdr.udsltp);
            pdarry->hbuhdr.udsltp = (UBYTE*)NULL;
        }
        if (pdarry->hbuhdr.udplop) {
            (VOID) egcfreeptr((UBYTE *) pdarry->hbuhdr.udplop);
            pdarry->hbuhdr.udplop = (UBYTE*)NULL;
        }
        if (pdarry->hbuhdr.uddsop) {
            (VOID) egcfreeptr((UBYTE *) pdarry->hbuhdr.uddsop);
            pdarry->hbuhdr.uddsop = (UBYTE*)NULL;
        }
    }
    if (dos_egb_on == FALSE && pdarry->fp) {
        (VOID) egcfclose(pdarry->fp);
        pdarry->fp = (EGCFILE)0;
    }
    return;
}
/*PAGE*/

/*------------------------------------------------------------------*/
/* getgseg : get GT block (opendict sub procedure)                  */
/*                                                                  */
/* return                                                           */
/*      NORMAL : success                                            */
/*      ERROR  : memory allocation failure or disk I/O error        */
/*------------------------------------------------------------------*/
NONREC
BOOL    getgseg(pdarry)
    PTR_MDICT       pdarry;
{
    WORD            GTlen;
    UBYTE           IObuf[512];
    UBYTE          *p, *q;
    register WORD   i;

    if (egcIOctrl(pdarry->fp, IObuf, pdarry->gtoff, 512, RSEG))
        return  ERROR;
    GTlen = (WORD)getint(IObuf + FHLAST * 2);
    if ((grtp = (UBYTE*)egcnewptr(GTlen)) == (UBYTE*)NULL) {
#ifndef DOS_EGBRIDGE
        egcerrno = ERR_MALLOC;
#endif
        return  ERROR;
    }
    if (egcIOctrl(pdarry->fp, grtp, pdarry->gtoff, GTlen, RSEG))
        return  ERROR;
    p = q = grtp;
    for (i = 0; i < FHNUM; i++) {
        dbftbl[i] = q + getint(p);
        p += 2;
    }
    return  NORMAL;
}
/*PAGE*/

/*------------------------------------------------------------------*/
/* getlseg : get PL pointer                                         */
/*                                                                  */
/* return                                                           */
/*      pointer to PL record                                        */
/*------------------------------------------------------------------*/
NONREC
UBYTE *
#ifdef  DOS_EGBRIDGE
PASCAL
#endif
getlseg(idno, dsp, len)
    WORD            idno;
    UWORD           dsp;
    UWORD           len;
{
#ifdef DOS_EGC
    LONG            readOff, seekOff, endOff, retOff;
    PTR_MDICT       pdarry;
    PTR_MDICT       bfarry = dicstat[0].pds;

    pdarry  = (idno) ? dicstat[idno].pds : dicstat[0].pds;
    readOff = pdarry->ploff + (LONG)dsp * 16;
    endOff  = readOff + len * 16;

    if (bfarry->bfrmod != PLBFR
        || bfarry->bfrid != idno
        || readOff < bfarry->bfrsof
        || endOff > bfarry->bfreof) {
        if (endOff > pdarry->ploff + DSBLKL)
            seekOff = endOff - DSBLKL;
        else
            seekOff = pdarry->ploff;
        if (egcIOctrl(pdarry->fp, bfarry->bfrp, seekOff, DSBLKL, RSEG))
            return  (UBYTE*)NULL;
        bfarry->bfrmod = PLBFR;
        bfarry->bfrsof = seekOff;
        bfarry->bfreof = endOff;
        bfarry->bfrid  = idno;
    }
    retOff = readOff - bfarry->bfrsof;

    return  (bfarry->bfrp + retOff);
#endif                              /* DOS_EGC */

#ifdef UNIX
    ULONG           plseg;
    PTR_MDICT       pdarry;
    UBYTE          *retval;

    pdarry = (idno) ? dicstat[idno].pds : dicstat[0].pds;
    plseg = (ULONG) pdarry->ploff + (ULONG) ((ULONG) dsp * 16);
    retval = (UBYTE *) ((UBYTE *) pdarry->plp + (ULONG) plseg);
    return (retval);
#endif
}

/*------------------------------------------------------------------*/
/* getwseg : get word block pointer                                 */
/*                                                                  */
/* return                                                           */
/*      pointer to word block                                       */
/*------------------------------------------------------------------*/
UBYTE*
#ifdef  DOS_EGBRIDGE
PASCAL
#endif
getwseg(idno, segno)
    WORD    idno;
    UWORD   segno;
{
    LONG            segOff;
    PTR_MDICT       pdarry;
    PTR_MDICT       bfarry = dicstat[0].pds;
    EGCFILE         fp;
#ifdef  DEBUG_CT
    static UWORD        ct = 0;
#endif

    pdarry = (idno != 0)?   dicstat[idno].pds : dicprty[0].pds;
    segOff = pdarry->dsoff + (LONG)(segno - 1) * DSBLKL;
    if (bfarry->bfrid != idno || bfarry->bfrsof != segOff) {
        fp = (idno != 0)?   pdarry->fp : dicstat[0].pds->fp;
        if (egcIOctrl(fp, bfarry->bfrp, segOff, DSBLKL, RSEG))
            return (UBYTE*)NULL;
        bfarry->bfrmod = DSBFR;
        bfarry->bfrsof = segOff;
        bfarry->bfrid  = idno;
#ifdef  DEBUG_CT
        dbgmsg(10,1,"wseg : ",++ct);
#endif
    }
    return  bfarry->bfrp;
}
/*PAGE*/

/*------------------------------------------------------------------*/
/* getuseg : block read from user dictionary                        */
/*                                                                  */
/* return                                                           */
/*      NORMAL : read success                                       */
/*      ERROR  : disk I/O error                                     */
/*------------------------------------------------------------------*/
NONREC
BOOL
#ifdef  DOS_EGBRIDGE
PASCAL
#endif
getuseg(idno, buf, cnt, blkno)
    WORD            idno;
    UBYTE          *buf;
    UWORD           cnt;
    UWORD           blkno;
{
    LONG        expoff;
    EGCFILE     fp;

    expoff = (idno) ? dicstat[idno].pds->gtoff : 0;
    fp = (idno) ? dicstat[idno].pds->fp : dicprty[0].pds->fp;
    if (egcIOctrl(fp, buf, (LONG)blkno * UDBLKL + expoff, cnt * UDBLKL, RSEG))
        return (ERROR);
    return (NORMAL);
}
/*PAGE*/

/*------------------------------------------------------------------*/
/* putuseg : block write to user dictionary                         */
/*                                                                  */
/* return                                                           */
/*      NORMAL : write success                                      */
/*      ERROR  : disk I/O error                                     */
/*------------------------------------------------------------------*/
NONREC
BOOL
#ifdef  DOS_EGBRIDGE
PASCAL
#endif
putuseg(idno, buf, cnt, blkno)
    WORD            idno;
    UBYTE          *buf;
    UWORD           cnt;
    UWORD           blkno;
{
    PTR_MDICT       pdarry;

    if (idno != 0)  return (ERROR);
    pdarry = (idno) ? dicstat[idno].pds : dicprty[0].pds;

    if (egcIOctrl(pdarry->fp, buf, (LONG)blkno * UDBLKL, cnt * UDBLKL, WSEG))
        return (ERROR);
    return (NORMAL);
}
/*PAGE*/
#ifdef UNIX
/*------------------------------------------------------------------*/
/* finddict : find dictionary                                       */
/*                                                                  */
/* return                                                           */
/*      status of search dictionary                                 */
/*------------------------------------------------------------------*/
NONREC
UWORD find_dict(dict, path, dictname)
    REG             dict;
    UBYTE          *path;
    UBYTE          *dictname;
{
    return (1);
}

NONREC
UWORD find_dict2(dict, path, dictname)
    REG             dict;              /* switch MAINDICT or USERDICT */
    UBYTE          *path;
    UBYTE          *dictname;
{
    UBYTE          *p;
    UBYTE           cdir[MAXPATHLEN], fullpath[MAXPATHLEN];
    struct passwd  *pswd, *getpwuid();
    struct stat     stbuf;
#ifdef  BSD42
    char           *getwd();
#endif
    /* Path search process */
    /* First  (1): Parameter path directory ;then path exist */
    /* Second (2): Currnet directory ;then NULL */
    /* Third  (3): Enviroment directory ;do setenv */
    /* Forth  (4): Home directory */

    switch (dict) {
    case MAINDICT:
    case USERDICT:
        /* First  (1): Parameter path directory ;then path exist */
        if ((path != (UBYTE*)NULL) && (*path != 0)) {
            /* Path name check. */
            /* Type 1 : Full path */
            /* Type 2 : File name only "udict" */
            strcpy(fullpath, path); /* Type 1 */
            if (stat(fullpath, &stbuf) == 0) {
                /* File's type check */
                if (!(stbuf.st_mode & S_IFDIR)) {
                    return (strlen(fullpath));
                }
            }
            if (*dictname == '/')   /* Type 2 */
                dictname++;
            sprintf(fullpath, "%s/%s", path, dictname);
            if (stat(fullpath, &stbuf) == 0) {
                /* File's type check */
                if (stbuf.st_mode & S_IFMT) {
                    return (strlen(fullpath));
                }
            }

        }
        /* Second (2): Currnet directory ;then NULL */
#ifdef  BSD42
        getwd(cdir);
        sprintf(fullpath, "%s/%s", cdir, dictname);
#endif                              /* BSD42 */
#ifdef  SVR2
        sprintf(fullpath, "./%s", dictname);
#endif                              /* SVR2 */
        if (stat(fullpath, &stbuf) == 0) {
            /* File's type check */
            if (stbuf.st_mode & S_IFMT) {
                return (strlen(fullpath));
            }
        }
        /* Third  (3): Enviroment directory ;do setenv */
        if ((p = (UBYTE *) getenv(ENV_USERDIC)) != (UBYTE*)NULL) {
            strcpy(fullpath, p);
            if (stat(fullpath, &stbuf) == 0) {
                /* File's type check */
                if (!(stbuf.st_mode & S_IFMT)) {
                    return (strlen(fullpath));
                }
            }
        }
        /* Forth  (4): Home directory */
        if ((pswd = getpwuid(getuid())) == NULL) {
            return (ERR_USRNOTFOUND);
        }
        sprintf(fullpath, "%s/%s", pswd->pw_dir, EGC_UPATH);
        if (stat(fullpath, &stbuf) == 0) {
            /* File's type check */
            if (stbuf.st_mode & S_IFMT) {
                return (strlen(fullpath));
            }
        }
        return (ERR_FNOTFOUND);
    }
    return (ERROR);
}
#endif
/*PAGE*/

#ifdef UNIX
/*------------------------------------------------------------------*/
/* expinit : initialize before use expert dictionary                */
/*                                                                  */
/* return                                                           */
/*      VOID                                                        */
/*------------------------------------------------------------------*/
NONREC
VOID    expinit(pdics)
    PDICS           pdics;
{
    struct dirent  *dir;               /* directory entry      */
    char            fullpath[128];
    char            spath[128];
    struct stat     stbuf;
    DIR            *dfd;
    REG             idno;
    idno = 1;

    strcpy(fullpath, EXPDIR);
    strcpy(spath, fullpath);
    strcat(spath, "/.");
    if (stat(spath, &stbuf) == -1) {
        return;
    }
    if ((dfd = opendir(fullpath)) == 0) {
        return;
    }
    while ((dir = readdir(dfd)) != 0) {
        if ((strcmp(dir->d_name, ".") == 0) ||
            (strcmp(dir->d_name, "..") == 0)) {
            continue;
        }
        strcpy(spath, fullpath);
        strcat(spath, "/");
        strcat(spath, dir->d_name);
        if (stat(spath, &stbuf) == -1)
            continue;
        else if (!(stbuf.st_mode & S_IFREG)) {
            continue;
        }
        strcpy(dicstat[idno].pds->path, spath);
        if (expopen(idno) == NORMAL) {
            dicstat[idno].idno = idno;
            pdics->id = idno;
            strcpy(pdics->name, dir->d_name);
            pdics++;
        }
        idno++;
    }                               /* while */
    egcerrno = NORMAL;
    return;
}
#endif
/*PAGE*/

/*------------------------------------------------------------------*/
/* egcopenexpdic : open expert dictionary (main procedure)          */
/*                                                                  */
/* return                                                           */
/*      status of open dictionary                                   */
/*------------------------------------------------------------------*/
NONREC
WORD    egcopenexpdic(idno, path)
    WORD            idno;
    UBYTE          *path;
{
    if (idno <= 0 || idno >= MAXOPENDIC)
        return (EROED01);

#ifdef UNIX
    if (dicprty[EXPID].idno == (WORD)-1) {
        dicprty[EXPID].idno = idno;
        dicprty[EXPID].pds = dicstat[idno].pds;
        return (NORMAL);
    }
#endif

#ifdef DOS_EGC
    if (dicprty[EXPID].idno == (WORD)-1) {
        strcpy(dicstat[idno].pds->path, path);
        if (expopen(EXPID)) {
            return (EROED01);       /* open error */
        }
        else {
            dicprty[EXPID].idno = idno;
            dicprty[EXPID].pds  = dicstat[idno].pds;
            return (NORMAL);
        }
    }
#endif
    return (EROED02);
}
/*PAGE*/

/*------------------------------------------------------------------*/
/* expopen : open expert dictionary (egcopenexpdic sub procedure)   */
/*                                                                  */
/* return                                                           */
/*      NORMAL : open success                                       */
/*      ERROR  : open failure                                       */
/*------------------------------------------------------------------*/
NONREC
BOOL    expopen(idno)
    WORD            idno;
{
    PTR_MDICT       pdarry;
    pdarry = dicstat[idno].pds;
#ifndef DOS_EGBRIDGE
    egcerrno = NORMAL;
#endif
    if (openmseg(idno, pdarry) != NORMAL) {
        return (ERROR);
    }
    if (openuseg(idno, pdarry) != NORMAL) {
        closemseg(idno, pdarry);
        return (ERROR);
    }
    return (NORMAL);
}
/*PAGE*/

/*------------------------------------------------------------------*/
/* egccloseexpdic : close expert dictionary                         */
/*                                                                  */
/* return                                                           */
/*      status of close dictionary                                  */
/*------------------------------------------------------------------*/
NONREC
WORD    egccloseexpdic(idno)
    WORD            idno;
{
    if ((idno <= 0 || MAXOPENDIC <= idno)
        || (dicprty[EXPID].idno == (WORD)-1)) {
        return (ERCED01);
    }
    dicprty[EXPID].idno = (WORD)-1;
#ifdef UNIX
	dicprty[1].pds = NULL;
#else
	closeuseg(EXPID, dicprty[EXPID].pds);
#endif
    return (NORMAL);
}
/*PAGE*/

#ifndef DOS_EGBRIDGE
/*------------------------------------------------------------------*/
/* egcgetuserdic : get user dictionary file name                    */
/*                                                                  */
/* return                                                           */
/*      pointer to dictionary name                                  */
/*------------------------------------------------------------------*/
NONREC
UBYTE*  egcgetuserdic(dstdicname)
    UBYTE*  dstdicname;
{
    strcpy(dstdicname, &egcuserdicname[0]);
    return (dstdicname);
}

/*------------------------------------------------------------------*/
/* egcgetmaindic : get main dictionary file name                    */
/*                                                                  */
/* return                                                           */
/*      pointer to dictionary name                                  */
/*------------------------------------------------------------------*/
NONREC
UBYTE*  egcgetmaindic(dstdicname)
    UBYTE*  dstdicname;
{
    strcpy(dstdicname, &egcmaindicname[0]);
    return (dstdicname);
}

/*------------------------------------------------------------------*/
/* egcsetuserdic : set user dictionary file name                    */
/*                                                                  */
/* return                                                           */
/*      pointer to new dictionary name                              */
/*------------------------------------------------------------------*/
NONREC
UBYTE*  egcsetuserdic(newdicname)
    UBYTE*  newdicname;
{
    egcdefaultdicflags = FALSE;
    strcpy(&egcuserdicname[0], newdicname);
    return (newdicname);
}

/*------------------------------------------------------------------*/
/* egcsetmaindic : set main dictionary file name                    */
/*                                                                  */
/* return                                                           */
/*      pointer to new dictionary name                              */
/*------------------------------------------------------------------*/
NONREC
UBYTE*  egcsetmaindic(newdicname)
    UBYTE*  newdicname;
{
    egcdefaultdicflags = FALSE;
    strcpy(&egcmaindicname[0], newdicname);
    return (newdicname);
}
/*PAGE*/
#endif  /* DOS_EGBRIDGE */

#ifndef DOS_EGBRIDGE
/*------------------------------------------------------------------*/
/* CheckUI341 : adjust upper index                                  */
/*                                                                  */
/* return                                                           */
/*      VOID                                                        */
/*------------------------------------------------------------------*/
NONREC
VOID    CheckUI341(pdarry)
    PTR_MDICT   pdarry;
{
    UWORD           i;
    UBYTE          *ui;
    LONG            ver;

    ver = egcgetuserstrver(pdarry->path);
    if (ver != 30401L)
        return;
    ui = pdarry->hbuhdr.uduidx;
    for (i = 0; i < 44; i++) {
        if (i > 0 && ui[i - 1] > ui[i])
            ui[i] = 0xFF;
    }
}
#endif

#ifdef  DOS_EGC
#ifdef	PMSLT
/*------------------------------------------------------------------*/
/* GetSltp : Get slt segment value.                                 */
/*                                                                  */
/* return                                                           */
/*      success     : slt pointer                                   */
/*      failure     : NULL                                          */
/*------------------------------------------------------------------*/
#ifdef  DOS_EGBRIDGE
#define SLTCSL   3
#else
#define SLTCSL  30
#endif
static SLTTBL   slt[SLTCSL] = {0};

UBYTE*
#ifdef  DOS_EGBRIDGE
PASCAL
#endif
GetSltp(pdarry, segOff)
    PTR_MDICT    pdarry;
    UWORD        segOff;
{
    register UWORD  i, busy, minor = 0;
    LONG            sltOff = 0L;
#ifdef  DEBUG_CT
    static UWORD        ct = 0;
#endif

    if (pdarry == (PTR_MDICT)NULL && segOff == 0xFFFF) {
        for (i = 0; i < SLTCSL; i++) {
            slt[i].busy = 0xFFFF;
            slt[i].seg  = 0xFFFF;
            egcmemset(slt[i].buf, 0x00, MAXSLTVAL * 2);
        }
        return  (UBYTE*)NULL;
    }

    busy = slt[0].busy;
    for (i = 0; i < SLTCSL; i ++) {
    	if (slt[i].seg == segOff) {
    	    slt[i].busy ++;
            pdarry->hbuhdr.udsltp = (UBYTE*)slt[i].buf;
    	    return    slt[i].buf;
    	}
        if (busy < slt[i].busy) {
            busy = slt[i].busy; minor = i;
        }
    	if (slt[i].seg == 0xFFFF)
    	    break;
    }
#ifdef  DEBUG_CT
    dbgmsg(15, 1, "sltp : ", ++ct);
#endif
    sltOff = (LONG)getint(pdarry->hbuhdr.ueslts) * UDBLKL + segOff;
    if (egcIOctrl(pdarry->fp, slt[minor].buf, sltOff, maxslt * 2, RSEG)) {
        return    (UBYTE*)NULL;
    }
    pdarry->hbuhdr.udsltp = (UBYTE*)slt[minor].buf;
    slt[minor].seg = segOff;
    slt[minor].busy = (UWORD)1;

    return   slt[minor].buf;
}

/*------------------------------------------------------------------*/
/* PutSltp : Put slt segment value.                                 */
/*                                                                  */
/* return                                                           */
/*      success     : NORMAL                                        */
/*      failure     : ERROR                                         */
/*------------------------------------------------------------------*/
BOOL
#ifdef  DOS_EGBRIDGE
PASCAL
#endif
PutSltp(pdarry, segno)
    PTR_MDICT    pdarry;
    UWORD        segno;
{
    LONG	        sltOff;
    UBYTE*          sltbuf;
    register UWORD  i;

    sltOff = (LONG)getint(pdarry->hbuhdr.ueslts) * UDBLKL + segno;
    sltbuf = (UBYTE*)pdarry->hbuhdr.udsltp;
    if (egcIOctrl(pdarry->fp, sltbuf, sltOff, maxslt * 2, WSEG)) {
        return    ERROR;
    }
    for (i = 0; i < SLTCSL; i ++) {
    	if (slt[i].seg == segno) {
    	    egcmemmove(slt[i].buf, sltbuf, maxslt * 2);
    	    slt[i].busy ++;
    	    break;
    	}
    	if (slt[i].seg == 0xFFFF)
    	    break;
    }
    return    NORMAL;
}
#endif    /* PMSLT */

#ifdef  PMPLO
/*------------------------------------------------------------------*/
/* GetPlop : Get plo segment value.                                 */
/*                                                                  */
/* return                                                           */
/*      success     : plo pointer                                   */
/*      failure     : NULL                                          */
/*------------------------------------------------------------------*/
UBYTE*
#ifdef  DOS_EGBRIDGE
PASCAL
#endif
GetPlop(pdarry, segno)
    PTR_UDICT    pdarry;
    UWORD        segno;
{
    LONG        stoff;                          /* current PLO start offset */
    LONG        enoff;                          /* current PLO end offset   */
    LONG        plosof = pdarry->plosof;        /* previous PLO start offset*/
    LONG        ploeof = pdarry->ploeof;        /* previous PLO end offset  */
    LONG        retoff = 0L;                    /* return PLO offset        */
    UBYTE*      p = pdarry->hbuhdr.udplop;      /* pointer to PLO record    */
#ifdef  DEBUG_CT
    static UWORD    ct = 0;
#endif

    if (segno == 0)
        return  (UBYTE*)NULL;

    stoff = (LONG)getint(pdarry->hbuhdr.ueplos) * UDBLKL
        + getint(pdarry->hbuhdr.udidxp[segno - 1].ieplop);
    enoff = (LONG)getint(pdarry->hbuhdr.ueplos) * UDBLKL
        + getint(pdarry->hbuhdr.udidxp[segno].ieplop);

    if (stoff < plosof || ploeof < enoff) {
        if (egcIOctrl(pdarry->fp, p, stoff, MAXPLOL, RSEG)) {
            pdarry->plosof = UVHIGH;
            pdarry->ploeof = UVLOW;
            return  (UBYTE*)NULL;
        } else {
            pdarry->plosof = stoff;
            pdarry->ploeof = stoff + MAXPLOL;
        }
#ifdef  DEBUG_CT
        dbgmsg(20, 1, "plop : ", ++ct);
#endif
    } else {
        retoff = stoff - plosof;
    }

    return  (p + retoff);
}

/*------------------------------------------------------------------*/
/* PutPlop : Put plo segment value.                                 */
/*                                                                  */
/* return                                                           */
/*      success     : NORMAL                                        */
/*      failure     : ERROR                                         */
/*------------------------------------------------------------------*/
BOOL
#ifdef  DOS_EGBRIDGE
PASCAL
#endif
PutPlop(pdarry, segno, label, llen)
    PTR_UDICT   pdarry;
    UWORD       segno;
    UBYTE*      label;
    UBYTE       llen;
{
    LONG            ploOff = 0L;

    if (segno == 0)
        return  ERROR;

    ploOff = (LONG)getint(pdarry->hbuhdr.ueplos) * UDBLKL
        + getint(pdarry->hbuhdr.udidxp[segno - 1].ieplop);
    if (egcIOctrl(pdarry->fp, label, ploOff, llen, WSEG))
        return    ERROR;
    pdarry->plosof = UVHIGH;
    pdarry->ploeof = UVLOW;
    return  NORMAL;
}

/*------------------------------------------------------------------*/
/* MovePlop : Move plo segment                                      */
/*                                                                  */
/* return                                                           */
/*      success     : NORMAL                                        */
/*      failure     : ERROR                                         */
/*------------------------------------------------------------------*/
BOOL
#ifdef  DOS_EGBRIDGE
PASCAL
#endif
MovePlop(pdarry, segno, movefr, moveto, movel)
    PTR_UDICT   pdarry;
    UWORD       segno;
    UWORD       movefr;
    UWORD       moveto;
    UWORD       movel;
{
    register WORD   i;
    register LONG   seekofs;
    WORD            cnt, mod;
    UBYTE           plobuf[UDBLKL + MAXPLOF];
    UWORD           mlen;

    if (movel <= 0 || moveto == movefr)
        return  ERROR;

    cnt = (UWORD)(movel / UDBLKL);
    mod = (UWORD)(movel % UDBLKL);
	seekofs = (LONG)getint(pdarry->hbuhdr.ueplos) * UDBLKL +
       	(LONG)getint(pdarry->hbuhdr.udidxp[segno - 1].ieplop) + movefr;

    if (movefr < moveto) {
		BOOL	loopf = FALSE;
		seekofs = seekofs + movel - UDBLKL;
		mlen = moveto - movefr;
		for (i = 0; i < cnt; i++) {
			loopf = TRUE;
			egcIOctrl(pdarry->fp, plobuf, seekofs, UDBLKL, RSEG);
            egcIOctrl(pdarry->fp, plobuf, seekofs + mlen, UDBLKL, WSEG);
			seekofs -= UDBLKL;
        }
		if (mod) {
			egcIOctrl(pdarry->fp, plobuf, seekofs, UDBLKL, RSEG);
			egcIOctrl(pdarry->fp, plobuf + (UDBLKL - mod),
					seekofs + mlen + (UDBLKL - mod), mod, WSEG);
		}
    } else {
		mlen = movefr - moveto;
		for (i = 0; i < cnt + 1; i++) {
			UWORD	wlen = (i == cnt) ? mod : UDBLKL;
			egcIOctrl(pdarry->fp, plobuf, seekofs, UDBLKL, RSEG);
			if (i == cnt) {
				egcmemset(plobuf + wlen, 0x00, UDBLKL + MAXPLOF - wlen);
            	egcIOctrl(pdarry->fp, plobuf, seekofs - mlen, wlen+mlen, WSEG);
            } else {
            	egcIOctrl(pdarry->fp, plobuf, seekofs - mlen, wlen, WSEG);
            }
			seekofs += UDBLKL;
        }
    }
    pdarry->plosof = UVHIGH;
    pdarry->ploeof = UVLOW;

    return  NORMAL;
}
#endif  /* PMPLO */
#endif  /* DOS_EGC */

/*------------------------------------------------------------------*/
/*  egcIOctrl : Disk I/O control, seek, read and write.             */
/*                                                                  */
/*  return                                                          */
/*      NORMAL : success                                            */
/*      ERORR  : disk I/O error                                     */
/*------------------------------------------------------------------*/
BOOL
#ifdef  DOS_EGBRIDGE
PASCAL
#endif
egcIOctrl(fp, buf, sl, rwl, mode)
    EGCFILE     fp;
    UBYTE*      buf;
    LONG        sl;
    REG         rwl;
    UBYTE       mode;
{
    BOOL   st = NORMAL;

    if (egcfseek(fp, sl, 0) == ERROR)
        return  ERROR;
    switch (mode) {
    case    RSEG:
        st = egcfread(buf, rwl, 1, fp);
        break;
    case    WSEG:
        st = egcfwrite(buf, rwl, 1, fp);
        break;
    }
    return  st;
}

/*PAGE*/

#ifndef DOS_EGBRIDGE
/*------------------------------------------------------------------*/
/*  SuspendDic : suspend dictionary                                 */
/*                                                                  */
/*  return                                                          */
/*      NORMAL : suspend success                                    */
/*      ERORR  : dictionary is not connected                        */
/*------------------------------------------------------------------*/
WORD    SuspendDic(idno)
    WORD    idno;
{
    WORD    ret = NORMAL;

    if (idno & MAINDIC) {
        if (dicstat[0].pds->suspended == (UBYTE)ACTIVE)
            dicstat[0].pds->suspended = (UBYTE)SUSPEND;
        else
            ret = ERROR;
    }
    if (idno & USERDIC) {
        if (dicprty[0].pds->suspended == (UBYTE)ACTIVE)
            dicprty[0].pds->suspended = (UBYTE)SUSPEND;
        else
            ret = ERROR;
    }
    if (idno & EXPTDIC) {
        if (dicstat[1].pds->suspended == (UBYTE)ACTIVE)
            dicstat[1].pds->suspended = (UBYTE)SUSPEND;
        else
            ret = ERROR;
    }
    if (ret == NORMAL) {
#ifdef  BHREAD
        egcreconv(1);
#endif
        CashFuncInit();
    }

    return  ret;
}

/*------------------------------------------------------------------*/
/*  ResumeDic : resume dictionary                                   */
/*                                                                  */
/*  return                                                          */
/*      NORMAL : resume success                                     */
/*      ERORR  : dictionary is not suspended                        */
/*------------------------------------------------------------------*/
WORD    ResumeDic(idno)
    WORD    idno;
{
    WORD    ret = NORMAL;
    if (idno & MAINDIC) {
        if (dicstat[0].pds->suspended == (UBYTE)SUSPEND)
            dicstat[0].pds->suspended = (UBYTE)ACTIVE;
        else
            ret = ERROR;
    }
    if (idno & USERDIC) {
        if (dicprty[0].pds->suspended == (UBYTE)SUSPEND)
            dicprty[0].pds->suspended = (UBYTE)ACTIVE;
        else
            ret = ERROR;
    }
    if (idno & EXPTDIC) {
        if (dicstat[1].pds->suspended == (UBYTE)SUSPEND)
            dicstat[1].pds->suspended = (UBYTE)ACTIVE;
        else
            ret = ERROR;
    }
    if (ret == NORMAL) {
        CashFuncInit();
#ifdef  BHREAD
        egcreconv(1);
#endif
    }

    return  ret;
}
#endif  /* DOS_EGBRIDGE */

/*------------------------------------------------------------------*/
/* CashFuncInit : Initialize cashing functions                      */
/* return    void                                                   */
/*------------------------------------------------------------------*/
VOID    CashFuncInit()
{
    PTR_MDICT       pdarry = dicstat[0].pds;

    pdarry->bfrmod = NOBFR;
    pdarry->bfrsof = UVHIGH;
    pdarry->bfreof = UVLOW;
    pdarry->bfrid  = -1;
#ifdef  DOS_EGC
#ifdef  PMSLT
    GetSltp((PTR_MDICT)NULL, 0xFFFF);
#endif
#endif
}

#ifdef  BHREAD
/*------------------------------------------------------------------*/
/*  egcreconv : set reconvert flag                                  */
/*                                                                  */
/*  return                                                          */
/*      VOID                                                        */
/*------------------------------------------------------------------*/
VOID  egcreconv(cmd)
    UBYTE   cmd;
{
    dicprty[0].pds->reconv = (UBYTE)((cmd)? TRUE : FALSE);
}
#endif

#ifndef DOS_EGBRIDGE
/*------------------------------------------------------------------*/
/*  Canlearn : Is main and user connect ?                           */
/*                                                                  */
/*  return     TRUE  : dictionary is not active.                    */
/*             FALSE : dictionary is active.                        */
/*------------------------------------------------------------------*/
BOOL    Canlearn()
{
    return  (dicstat[0].pds->suspended && dicprty[0].pds->suspended);
}
#endif  /* DOS_EGBRIDGE */
