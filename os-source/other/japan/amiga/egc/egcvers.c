/************************************************************************/
/*                                                                      */
/*      EGCVERS : EGConvert Dictionary Versions check                   */
/*                                                                      */
/*              Designed    by  Y.Katogi        05/08/1991              */
/*              Coded       by  Y.Katogi        05/08/1991              */
/*                                                                      */
/*      (C) Copyright 1991 ERGOSOFT Corp.                               */
/*      All Rights REserved                                             */
/*                                                                      */
/*                          --- NOTE ---                                */
/*                                                                      */
/*      THIS PROGRAM BELONGS TO ERGO SOFT CORP.  IT IS CONSIDERED A     */
/*      TRADE SECRET AND IS NOT TO BE DIVULGED OR USED BY PARTIES       */
/*      WHO HAVE NOT RECEIVED WRITTEN AUTHORIZATION FROM THE OWNER.     */
/*                                                                      */
/************************************************************************/
#ifndef DOS_EGBRIDGE
#include <stdio.h>
#include "edef.h"
#include "egcenv.h"
#include "egcerr.h"
#include "egcdef.h"
#include "egckcnv.h"
#include "egcexp.h"
#include "egcudep.h"
#include "egcproto.h"

#ifdef DOS_EGC
#include <string.h>
#endif
/* change from char_type_version to long_type_version */
#define vers_ctol(cv) \
        ((LONG) ((LONG) cv[0] * 10000) + \
         ((LONG) cv[1] * 100) + \
         ((LONG) cv[2]))

#ifndef MAC
REG             egcverck = TRUE;       /* dict version check flag */
REG             egcsysck = TRUE;       /* system version check flag */
/*PAGE*/
/*--------------------------------------------------------------*/
NONREC
WORD egcdicverchk(a_path, b_path)
    UBYTE          *a_path;
    UBYTE          *b_path;
/*--------------------------------------------------------------*/
{
    WORD            st = NORMAL;
    PIE             a_gip, b_gip;
    PTR_MDICT       pdarry;

    if (a_path == NULL && b_path == NULL) {
        /* First check : version number */
        if (dicstat[0].pds->hbsegq != getint(dicprty[0].pds->hbuhdr.uesegq))
            return (ERVER02);
        if ((a_gip =
             (PIE) egcnewptr(GETIDXSIZE(dicstat[0].pds->hbsegq))) == NULL)
            return (ERVER00);
        b_gip = dicprty[0].pds->hbuhdr.udidxp;
        if (CmpIndex(a_gip, b_gip, dicstat[0].pds->hbsegq) == ERROR)
            st = ERVER02;
        (VOID) egcfreeptr((UBYTE *) a_gip);
        a_gip = NULL;
    }
    else if (a_path != NULL && b_path != NULL) {
        /* First check : version number */
        /* open main */
        pdarry = dicstat[0].pds;
        strcpy(pdarry->path, a_path);
        if (openmseg(0, pdarry) != NORMAL) {
            return (ERVER00);
        }
        if ((a_gip =
             (PIE) egcnewptr(GETIDXSIZE(pdarry->hbsegq))) == NULL) {
            closemseg(0, pdarry);
            return (ERVER00);
        }
        pdarry = dicprty[0].pds;
        if (readusfp(pdarry) != NORMAL ||
            readusidx(&pdarry->hbuhdr, 0) != NORMAL) {
            (VOID) egcfreeptr((UBYTE *) a_gip);
            a_gip = NULL;
            closemseg(0, pdarry);
            closeuseg(0, pdarry);
            return (ERVER00);
        }
        b_gip = pdarry->hbuhdr.udidxp;
        if (CmpIndex(a_gip, b_gip, dicstat[0].pds->hbsegq) == ERROR)
            st = ERVER02;
        (VOID) egcfreeptr((UBYTE *) a_gip);
        a_gip = NULL;
        closemseg(0, dicstat[0].pds);
        closeuseg(0, dicprty[0].pds);
    }
    return (st);                    /* Same dict */
}
#endif
/*PAGE*/
/*--------------------------------------------------------------*/
NONREC
WORD egcsysverchk(mpath)
    UBYTE          *mpath;
/*--------------------------------------------------------------*/
{
    LONG            egc_sys_ver_nbr, egc_dic_ver_nbr;

    egc_sys_ver_nbr = egcgetsysverno();
    if ((egc_dic_ver_nbr = egcgetmainstrver(mpath)) == -1)
        return (ERSVC05);

    if (egc_sys_ver_nbr == egc_dic_ver_nbr)
        return (ERSVC02);
    else if (egc_sys_ver_nbr < egc_dic_ver_nbr)
        return (ERSVC03);
    else if (egc_sys_ver_nbr > egc_dic_ver_nbr)
        return (ERSVC04);
    else if (egc_sys_ver_nbr < 0 || egc_dic_ver_nbr < 0)
        return (ERSVC00);
    return (NORMAL);
}
/*PAGE*/
/*--------------------------------------------------------------*/
NONREC
LONG egcgetsysverno()
/*--------------------------------------------------------------*/
{
    return ((LONG) EGCSYSVERNO);
}
/*PAGE*/
/*--------------------------------------------------------------*/
NONREC
BOOL CmpIndex(a_gie, b_gie, msegq)
    PIE             a_gie;
    PIE             b_gie;
    UWORD           msegq;
/*--------------------------------------------------------------*/
{
    MakeGIndex((UBYTE *) a_gie, msegq);
    for (; a_gie->ieyomi[0] != 0xff; a_gie++, b_gie++) {
        if ((memcmp(b_gie->ieyomi, a_gie->ieyomi, sizeof(a_gie->ieyomi)) == 0)
            && (memcmp(b_gie->ieplp, a_gie->ieplp, sizeof(a_gie->ieplp)) == 0))
            continue;
        return (ERROR);             /* Different dict */
    }
    return (NORMAL);
}
/*PAGE*/

/*--------------------------------------------------------------*/
NONREC
LONG egcgetmainstrver(path)
    UBYTE          *path;
/*--------------------------------------------------------------*/
{
    return (egc_ver_read(path, (LONG) MD_STR_OFFSET, 1));
}

/*PAGE*/
/*--------------------------------------------------------------*/
NONREC
LONG egcgetuserstrver(path)
    UBYTE          *path;
/*--------------------------------------------------------------*/
{
    return (egc_ver_read(path, (LONG) UD_STR_OFFSET, 0));
}
/*PAGE*/
/*--------------------------------------------------------------*/
NONREC
LONG egcgetmaindicver(path)
    UBYTE          *path;
/*--------------------------------------------------------------*/
{
    return (egc_ver_read(path, (LONG) MD_VER_OFFSET, 1));
}

/*PAGE*/
/*--------------------------------------------------------------*/
NONREC
LONG egcgetuserdicver(path)
    UBYTE          *path;
/*--------------------------------------------------------------*/
{
    return (egc_ver_read(path, (LONG) UD_VER_OFFSET, 0));
}
/*PAGE*/
/*--------------------------------------------------------------*/
NONREC
LONG egcsetmainstrver(path, lver)
    UBYTE          *path;
    LONG            lver;
/*--------------------------------------------------------------*/
{
    return (egc_ver_write(path, lver, (LONG) MD_STR_OFFSET, 1));
}
/*--------------------------------------------------------------*/
NONREC
LONG egcsetuserstrver(path, lver)
    UBYTE          *path;
    LONG            lver;
/*--------------------------------------------------------------*/
{
    return (egc_ver_write(path, lver, (LONG) UD_STR_OFFSET, 0));
}
/*--------------------------------------------------------------*/
NONREC
LONG egcsetmaindicver(path, lver)
    UBYTE          *path;
    LONG            lver;
/*--------------------------------------------------------------*/
{
    return (egc_ver_write(path, lver, (LONG) MD_VER_OFFSET, 1));
}
/*--------------------------------------------------------------*/
NONREC
LONG egcsetuserdicver(path, lver)
    UBYTE          *path;
    LONG            lver;
/*--------------------------------------------------------------*/
{
    return (egc_ver_write(path, lver, (LONG) UD_VER_OFFSET, 0));
}
/*PAGE*/
/*--------------------------------------------------------------*/
NONREC
LONG egc_ver_write(path, new_ver, sekofs, dtype)
    UBYTE          *path;              /* path name of dict */
    LONG            new_ver;           /* pointer of new version */
    LONG            sekofs;            /* file seek version offset */
    BOOL            dtype;             /* dict type 1=main, 0=user */

/*      path    null pointer or null string                     */
/*              this case is dicstat[0]                         */
/*              not case is fopen.                              */
/*      lver    number of new version.                          */
/*      return  -1L       : ERROR                               */
/*              otherwise : number of main version              */
/*--------------------------------------------------------------*/
{
    EGCFILE         filep;
    UBYTE           ver[EGCSYSVERLEN];

#ifndef MAC
    vers_ltoc(new_ver, ver);
    if (path == NULL) {
        filep = (dtype) ? dicstat[0].pds->fp : dicprty[0].pds->fp;
    }
    else if ((filep = egcfopen((UBYTE *) path, READMODE)) == (EGCFILE)0) {
        return (-1L);
    }
    if (egcfseek(filep, sekofs, 0) == ERROR)
        goto err_write;
    if (egcfwrite(ver, 1, EGCSYSVERLEN, filep) == ERROR)
        goto err_write;
    if (path != NULL) {
        (VOID) egcfclose(filep);
    }
    return (new_ver);

err_write:
    if (path != NULL) {
        (VOID) egcfclose(filep);
        filep = (EGCFILE)0;
    }
    return (-1L);
#else
    return (new_ver);
#endif

}

/*--------------------------------------------------------------*/
NONREC
LONG egc_ver_read(path, sekofs, dtype)
    UBYTE          *path;
    LONG            sekofs;            /* file seek version offset */
    BOOL            dtype;             /* dict type 1=main, 0=user */
/*      path    null pointer or null string                     */
/*              this case is dicstat[0]                         */
/*              not case is fopen.                              */
/*      return  -1L       : ERROR                               */
/*              otherwise : number of main system version       */
/*--------------------------------------------------------------*/
{
    EGCFILE         filep;
    UBYTE           buf[EGCSYSVERLEN];

#ifndef MAC
    if (path == NULL) {
        filep = (dtype) ? dicstat[0].pds->fp : dicprty[0].pds->fp;
    }
    else if ((filep = egcfopen((UBYTE *) path, OPENMODE)) == (EGCFILE)0) {
        return (-1L);
    }
    if (egcfseek(filep, sekofs, 0) == ERROR)
        goto err_read;
    if (egcfread(buf, 1, EGCSYSVERLEN, filep) == ERROR)
        goto err_read;
    return (vers_ctol(buf));

err_read:
    if (path != NULL) {
        (VOID) egcfclose(filep);
        filep = (EGCFILE)0;
    }
    return (-1L);
#else
	if (dtype) {
		if (sekofs == MD_STR_OFFSET)
    		egcmemmove(buf, dicstat[0].pds->strver, EGCSYSVERLEN);
		else if (sekofs == MD_VER_OFFSET)
    		egcmemmove(buf, dicstat[0].pds->dicver, EGCSYSVERLEN);
	}
	else {
		if (sekofs == UD_STR_OFFSET)
    		egcmemmove(buf, dicprty[0].pds->strver, EGCSYSVERLEN);
		else if (sekofs == UD_VER_OFFSET)
    		egcmemmove(buf, dicprty[0].pds->dicver, EGCSYSVERLEN);
	}
    return (vers_ctol(buf));
#endif
}
/*PAGE*/
/*--------------------------------------------------------------*/
NONREC
VOID vers_ltoc(lv, cv)
    LONG            lv;
    UBYTE          *cv;
/* change from long_type_version to char_type_version */
/*--------------------------------------------------------------*/
{
    *cv = (UBYTE) (((LONG) lv / 10000) % 100);
    *(cv + 1) = (UBYTE) ((lv / 100) % 100);
    *(cv + 2) = (UBYTE) (lv % 100);
}
#endif  /* DOS_EGBRIDGE */
