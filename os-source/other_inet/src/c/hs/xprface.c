/* -----------------------------------------------------------------------
 * xprface.c		handshake_src
 *
 * $Locker:  $
 *
 * $Id: xprface.c,v 1.1 91/05/09 16:20:49 bj Exp $
 *
 * $Revision: 1.1 $
 *
 * $Header: HOG:Other/inet/src/c/hs/RCS/xprface.c,v 1.1 91/05/09 16:20:49 bj Exp $
 *
 * $Log:	xprface.c,v $
 * Revision 1.1  91/05/09  16:20:49  bj
 * Initial revision
 * 
 *
 *------------------------------------------------------------------------
 */

#include "xproto.h"

unsigned char rcsid[] = "$Header: HOG:Other/inet/src/c/hs/RCS/xprface.c,v 1.1 91/05/09 16:20:49 bj Exp $";

extern long a_fopen ();
extern long a_fclose ();
extern long a_fread ();
extern long a_fwrite ();
extern long a_sread ();
extern long a_swrite ();
extern long a_sflush ();
extern long a_update ();
extern long a_chkabort ();
extern long a_chkmisc ();
extern long a_gets ();
extern long a_setserial ();
extern long a_ffirst ();
extern long a_fnext ();
extern long a_finfo ();
extern long a_fseek ();
extern long a_options ();
extern long a_unlink ();
extern long a_squery ();
extern long a_getptr ();

void __regargs xpr_setup ( struct XPR_IO *xpr_io )
  {
    xpr_io->xpr_filename  = NULL;
    xpr_io->xpr_fopen     = a_fopen;
    xpr_io->xpr_fclose    = a_fclose;
    xpr_io->xpr_fread     = a_fread;
    xpr_io->xpr_fwrite    = a_fwrite;
    xpr_io->xpr_sread     = a_sread;
    xpr_io->xpr_swrite    = a_swrite;
    xpr_io->xpr_sflush    = a_sflush;
    xpr_io->xpr_update    = a_update;
    xpr_io->xpr_chkabort  = a_chkabort;
    xpr_io->xpr_chkmisc   = a_chkmisc;
    xpr_io->xpr_gets      = a_gets;
    xpr_io->xpr_setserial = a_setserial;
    xpr_io->xpr_ffirst    = a_ffirst;
    xpr_io->xpr_fnext     = a_fnext;
    xpr_io->xpr_finfo     = a_finfo;
    xpr_io->xpr_fseek     = a_fseek;
    xpr_io->xpr_extension = 4L;
    xpr_io->xpr_options   = a_options;
    xpr_io->xpr_unlink    = a_unlink;
    xpr_io->xpr_squery    = a_squery;
    xpr_io->xpr_getptr    = a_getptr;
    xpr_io->xpr_data      = NULL;
  }
