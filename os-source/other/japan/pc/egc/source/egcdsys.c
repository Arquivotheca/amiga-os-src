/************************************************************************/
/*                                                                      */
/*      EGCDSYS : EGConvert  MS-DOS system                              */
/*                                                                      */
/*              Designed    by  Y.Katogi        10/12/1990              */
/*              Coded       by  Y.Katogi        10/12/1990              */
/*                                                                      */
/*      (C) Copyright 1990-1991 ERGOSOFT Corp.                          */
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
#include        <pwd.h>
#include        <sys/types.h>
#include        <sys/stat.h>
#include        <sys/dir.h>
#include        <fcntl.h>
#else
#include        <string.h>
#endif
#include        "edef.h"
#include        "egcenv.h"
#include        "egcerr.h"
#include        "egcdef.h"
#include        "egckcnv.h"
#include        "egcexp.h"
#include        "egcudep.h"
#include        "egcproto.h"
#ifdef  DOS_EGBRIDGE
#include        "egcdctrl.h"
#endif

DICIDTBL        dicstat[MAXOPENDIC] = {0};
DICIDTBL        dicprty[MAXAPENDIC] = {0};
UDICT_INFO      user_dicinfo;

#ifndef DOS_EGBRIDGE
DICS            dics[MAXOPENDIC] = {0};
extern REG      egcerrno;
extern REG      egcverck;
extern REG      egcsysck;
extern UBYTE    EXPDIR[];
#ifdef DOS_EGC
REG             egcdosrootdic = (REG)FALSE;
#endif
#endif

#ifdef UNIX
REG             EGCDictID = 0;
REG             EGCUID    = 0;
#endif

#ifdef  DOS_EGBRIDGE
/*------------------------------------------------------------------*/
/*  egcinit : initialize EGConvert                                  */
/*                                                                  */
/*  return                                                          */
/*      NORMAL  : initialize success                                */
/*      another : something error                                   */
/*------------------------------------------------------------------*/
NONREC
WORD    egcinit(mfile, mf, ufile, uf)
    UBYTE*  mfile;
    UWORD    mf;
    UBYTE*  ufile;
    UWORD    uf;
{
    initmem();
    if (initdict() != NORMAL) {
        return (ERINI01);
    }
    strcpy(dicstat[0].pds->path, mfile);
    dicstat[MDIC_ID].pds->fp = mf;
    if (opendict(dicstat[0].pds) != NORMAL) {
        return (ERINI01);
    }
    dicstat[0].idno = 0;

    dicprty[0].pds = &user_dicinfo; /* set info end of initdict */
    strcpy(dicprty[0].pds->path, ufile);
    dicprty[UDIC_ID].pds->fp = uf;

    if (openuseg(0, dicprty[0].pds) != NORMAL) {
        closemseg(0, dicstat[0].pds);
        return (ERINI01);
    }
    dicprty[0].idno = 0;

    CashFuncInit();
    return  NORMAL;
}
/*PAGE*/
/*------------------------------------------------------------------*/
/*  egcfinish : finish EGConvert                                    */
/*                                                                  */
/*  return                                                          */
/*      NORMAL                                                      */
/*------------------------------------------------------------------*/
NONREC
WORD    egcfinish()
{
    closedict();
    closeuseg(0, dicprty[0].pds);
    return (NORMAL);
}
/*PAGE*/
#else
/*------------------------------------------------------------------*/
/*  egcinit : initialize EGConvert                                  */
/*                                                                  */
/*  return                                                          */
/*      NORMAL  : initialize success                                */
/*      another : something error                                   */
/*------------------------------------------------------------------*/
NONREC
WORD egcinit()
{
    UBYTE          *mpath;
    UBYTE          *upath;
    WORD            chkret;
    UBYTE           fullpath[MAXPATH];
    UBYTE           maindictpath[MAXPATH];
    UBYTE           userdictpath[MAXPATH];
    WORD            wret;

#ifdef  DOS_EGC
    initmem();
#endif
    mpath = egcgetmaindic(maindictpath);
    upath = egcgetuserdic(userdictpath);
    strcpy(EXPDIR, EXPTDIR);
    if (initdict() != NORMAL)
        return (ERINI01);
    dicprty[0].pds = &user_dicinfo; /* set info end of initdict */

    /* Find main dictionary path */
    if (findpath(mpath, fullpath, (int) MAINDICT, (UBYTE *) MAINDIR,
                 (UBYTE *) EGC_MPATH, (UBYTE *) ENV_MAINDIC)
        == ERROR)
        return (ERINI01);
    strcpy(dicstat[0].pds->path, fullpath);

    /* Find user dictionary path */
    if (findpath(upath, fullpath, (int) USERDICT, (UBYTE *) USERDIR,
                 (UBYTE *) EGC_UPATH, (UBYTE *) ENV_USERDIC)
        == ERROR)
        return (ERINI01);
    strcpy(dicprty[0].pds->path, fullpath);

#ifdef VER
    /* dict version check */
    if ((egcverck == TRUE) &&
        (wret = egcdicverchk(dicstat[0].pds->info.path,
                             dicprty[0].pds->path)) != NORMAL) {
        egcerrno = ERR_MISDICVER;
        return (ERINI01);
    }
#endif
    /* open main dict */
    if (opendict(dicstat[0].pds) != NORMAL)
        return (ERINI01);

    dicstat[0].idno = 0;

    /* open user dict */
    if (openuseg(0, dicprty[0].pds) != NORMAL) {
        closemseg(0, dicstat[0].pds);
        return (ERINI01);
    }
    dicprty[0].idno = 0;
    /* system version check */
    wret = NORMAL;
    if (egcsysck == TRUE) {
        wret = egcsysverchk(NULL);
        switch (wret) {
        case ERSVC02:               /* EGConvert init OK */
            wret = NORMAL;
            break;
        case ERSVC03:               /* Use dict of new version */
            wret = ERINI03;
            goto init_err;
        case ERSVC04:               /* EGConvert init OK dict of old ver */
            wret = NORMAL;
            break;
        case ERSVC05:               /* get version error */
            wret = ERINI05;
            goto init_err;
        case NORMAL:
            goto init_err;          /* version number something error */
        }
    }
    /* dict version check */
    if ((egcverck == TRUE) &&
        (chkret = egcdicverchk(NULL, NULL)) != NORMAL) {
        wret = ERINI06;
        goto init_err;
    }
#ifdef UNIX                         /* Convert Stand alone type */
    expinit((PDICS) & dics[0]);
#endif

    CashFuncInit();

    return (wret);

init_err:
    closedict();
    closeuseg(0, dicprty[0].pds);
    egcerrno = ERR_MISDICVER;
    return (wret);
}
/*PAGE*/

/********************************************************************/
NONREC
BOOL findpath(srcpath, dstpath, dictmode, dictdir, dictpath, dictenv)
    UBYTE          *srcpath;
    UBYTE          *dstpath;
    int             dictmode;
    UBYTE          *dictdir;
    UBYTE          *dictpath;
    UBYTE          *dictenv;
/********************************************************************/
{
#ifdef UNIX
    int             ret;
    UBYTE          *env_name, *p;
    struct stat     fstbuf;

    if (*srcpath == NULL) {         /* use default path & name */
        ret = find_dict2(dictmode, dictdir, dictpath);
        if ((ret == ERROR) && ((p = (UBYTE *) getenv(dictenv)) == NULL)) {
            return (ERROR);
        }
        if ((dictmode == USERDICT) &&
            ((env_name = (UBYTE *) getenv("USER")) == NULL)) {
            return (ERROR);
        }
        else {                      /* main dict */
            env_name = dictdir;
        }
        sprintf(dstpath, "%s/%s.dic", dictdir, env_name);
    }
    else if (*srcpath == '/') {     /* inputed fullpath & name */
        strcpy(dstpath, srcpath);
    }
    else {
        sprintf(dstpath, "./%s", srcpath);
        if ((stat(dstpath, &fstbuf) == NULL) && (fstbuf.st_mode & S_IFMT)) {
            ;
        }
        else {                      /* inputed user-dict-name only */
            sprintf(dstpath, "%s/%s", dictdir, srcpath);
        }
    }
#endif

#ifdef DOS_EGC
    if (egcdosrootdic == TRUE) {
        strcpy(dstpath, dictdir);
        strcat(dstpath, srcpath);
    }
    else {
        strcpy(dstpath, srcpath);
    }
#endif
    return (NORMAL);
}

/********************************************************************/
NONREC
WORD egcexpdicinfo(hdics)
    char          **hdics;
/********************************************************************/
{
    PDICS           tpdic;

    if ((tpdic = (PDICS) egcnewptr(sizeof(DICS) * MAXOPENDIC)) == NULL)
        return (ERROR);
    egcmemmove((UBYTE *) tpdic,
               (UBYTE *) & dics[0], sizeof(DICS) * MAXOPENDIC);
    *hdics = (char *) tpdic;
    return (NORMAL);
}
/*PAGE*/
/********************************************************************/
NONREC
WORD egcusoff()
/********************************************************************/
{
    closedict();
    closeuseg(0, dicprty[0].pds);
    return (NORMAL);
}

/********************************************************************/
NONREC
WORD egcfinish()
/********************************************************************/
{
    closedict();
    closeuseg(0, dicprty[0].pds);
    return (NORMAL);
}
#endif  /* DOS_EGBRIDGE */
