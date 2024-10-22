/* -----------------------------------------------------------------------
 * CvtParms.c		handshake_src
 *
 * $Locker:  $
 *
 * $Id: CvtParms.c,v 1.1 91/05/09 16:12:58 bj Exp $
 *
 * $Revision: 1.1 $
 *
 * $Header: HOG:Other/inet/src/c/hs/RCS/CvtParms.c,v 1.1 91/05/09 16:12:58 bj Exp $
 *
 * $Log:	CvtParms.c,v $
 * Revision 1.1  91/05/09  16:12:58  bj
 * Initial revision
 * 
 *
 *------------------------------------------------------------------------
 */

#include    "stdio.h"
#include    "stdlib.h"
#include    "fcntl.h"
#include    "string.h"
#include    "dos.h"
#include    "term.def"

typedef   struct {
      unsigned long int
            rsrvdl0     ,
            rsrvdl1     ,
            lspeed      , /* Baud rate (line speed)        */
            rsrvdl2;
      unsigned char
            tabvec[132];  /* Tab stops                     */
      unsigned long int
            rsrvdl3     ,
            rsrvdl4     ,
            dialtime    , /* Auto redial timeout           */
            dialdelay   ,
            rsrvdw1;
      unsigned short int
            color3;
      unsigned char
            decinlm     , /* Screen format                 */
            bitplane    , /* Number of bitplanes.          */
            level       , /* Emulation level               */
            portnum     ,
            pfk_cr      , /* Fun. key carriage return char */
            pfk_nl      , /* Fun. key line feed char       */
            hangup      , /* Hangup before dial flag       */
            freqtype    , /* File requester type           */
            decscnm     , /* Screen mode (dark/light)      */
            decpff      , /* Print form feed               */
            decpex      , /* Print extent                  */
            ctype       , /* Cursor type                   */
            decanm      , /* Mode, (ANSI/VT52)             */
            marbell     , /* Margin Bell                   */
            ukset       , /* Char set (UK/US)              */
            bell_type   , /* Comm port, (Or a fine wine)   */
            echo        , /* Local echo                    */
            decawm      , /* Wrap mode                     */
            lnm         , /* New line mode                 */
            axonxoff    , /* Flow control                  */
            bpc         , /* Bits per character            */
            stpbits     , /* No. of stop bits              */
            parity      , /* Parity                        */
            printer     , /* Printer number                */
            protocol    , /* File transfer protocol        */
            direction   , /* Direction of file transfer    */
            termid      , /* Term ID (VT100/102)           */
            icons       , /* Turnaround character          */
            dial_mode   , /* Auto Redialing                */
            seven_wire  ,
            txcr        ,
            txnl        ,
            rxcr        ,
            rxnl        ,
            bkspdel     ,
            answrbck[21], /* Answerback message            */
            dial_str[64] ,/* Dial Pre-string               */
            dest_path[64],/* Dest path for multi file rec. */
            pc_file[64] , /* PC File name                  */
            rsrvdstuf[48];/* Some space for expansion      */
      unsigned short int smallmap[8];
      unsigned short int colormap[8];
      char  phonestr[20][32];
      char  keystr[6][64];
} OLD_NVM;

static OLD_NVM  OldNvmodes;
static NVM      Nvmodes;

void usage ( void );

void main ( int argc, char **argv )
  {
    int oldfh;
    int newfh;
    unsigned long int file_size;
    
    if ( argc < 3 || ( argc == 2 && *argv[1] == '?' ) )
      {
        usage ();
        exit ( 0 );
      }
      
    if ( -1 == ( oldfh = _dopen ( argv[1], MODE_OLDFILE ) ) )
      {
        printf ( "Could not open input file <%s>\n", argv[1] );
        exit ( 0 );
      }

    file_size = _dseek ( oldfh, 0L, 2 );

    if ( file_size != sizeof(OLD_NVM) )
      {
        printf ( "Invalid parameter file\n" );
        _dclose ( oldfh );
        exit ( 0 );
      }

    _dseek  ( oldfh, 0L, 0 );
    _dread  ( oldfh, (char *) &OldNvmodes, sizeof ( OLD_NVM ) );
    _dclose ( oldfh );

    if ( -1 == ( newfh = _dopen ( argv[2], MODE_NEWFILE ) ) )
      {
        printf ( "Could not open output file <%s>\n", argv[2] );
        exit ( 0 );
      }
    Nvmodes.lspeed    = OldNvmodes.lspeed;
    strcpy ( Nvmodes.tabvec, OldNvmodes.tabvec );
    Nvmodes.dialtime   = OldNvmodes.dialtime;
    Nvmodes.dialdelay  = OldNvmodes.dialdelay;
    Nvmodes.color3     = OldNvmodes.color3;
    Nvmodes.decinlm    = OldNvmodes.decinlm;  
    Nvmodes.bitplane   = OldNvmodes.bitplane; 
    Nvmodes.level      = OldNvmodes.level;     
    Nvmodes.portnum    = OldNvmodes.portnum;
    Nvmodes.pfk_cr     = OldNvmodes.pfk_cr; 
    Nvmodes.pfk_nl     = OldNvmodes.pfk_nl;    
    Nvmodes.hangup     = OldNvmodes.hangup;
    Nvmodes.freqtype   = OldNvmodes.freqtype;
    Nvmodes.decscnm    = OldNvmodes.decscnm;
    Nvmodes.decpff     = OldNvmodes.decpff;
    Nvmodes.decpex     = OldNvmodes.decpex;
    Nvmodes.ctype      = OldNvmodes.ctype;
    Nvmodes.decanm     = OldNvmodes.decanm;
    Nvmodes.marbell    = OldNvmodes.marbell;
    Nvmodes.ukset      = OldNvmodes.ukset;
    Nvmodes.bell_type  = OldNvmodes.bell_type;
    Nvmodes.echo       = OldNvmodes.echo;
    Nvmodes.decawm     = OldNvmodes.decawm;
    Nvmodes.lnm        = OldNvmodes.lnm;
    Nvmodes.axonxoff   = OldNvmodes.axonxoff;
    Nvmodes.bpc        = OldNvmodes.bpc;
    Nvmodes.stpbits    = OldNvmodes.stpbits;
    Nvmodes.parity     = OldNvmodes.parity;
    Nvmodes.protocol   = OldNvmodes.protocol;
    Nvmodes.direction  = OldNvmodes.direction;
    Nvmodes.termid     = OldNvmodes.termid;
    Nvmodes.icons      = OldNvmodes.icons;
    Nvmodes.dial_mode  = OldNvmodes.dial_mode;
    Nvmodes.seven_wire = OldNvmodes.seven_wire;
    Nvmodes.txcr       = OldNvmodes.txcr;
    Nvmodes.txnl       = OldNvmodes.txnl;
    Nvmodes.rxcr       = OldNvmodes.rxcr;
    Nvmodes.rxnl       = OldNvmodes.rxnl;
    Nvmodes.bkspdel    = OldNvmodes.bkspdel;
    strcpy ( Nvmodes.answrbck  , OldNvmodes.answrbck  ); 
    strcpy ( Nvmodes.dial_str  , OldNvmodes.dial_str  ); 
    strcpy ( Nvmodes.dest_path , OldNvmodes.dest_path );
    strcpy ( Nvmodes.pc_file   , OldNvmodes.pc_file   ); 
    memcpy ( (char *)Nvmodes.smallmap  , (char *)OldNvmodes.smallmap, 8  * sizeof(short int) );
    memcpy ( (char *)Nvmodes.colormap  , (char *)OldNvmodes.colormap, 8  * sizeof(short int) );
    memcpy ( (char *)Nvmodes.phonestr  , (char *)OldNvmodes.phonestr, 20 * 32 );
    memcpy ( (char *)Nvmodes.keystr    , (char *)OldNvmodes.keystr  ,  6 * 64 );

    _dwrite ( newfh, (char *) &Nvmodes, sizeof(NVM) );
    _dclose ( newfh );
  }
  
  
void usage ()
  {
    printf ( "Handshake Parameter file conversion utility v1.0\n" );
    printf ( "  This utility is used to convert '.parms' files from the\n" );
    printf ( "  format used up to and including version 2.12a of Handshake\n" );
    printf ( "  to the new format.\n\n" );
    printf ( "Syntax:\n  CvtParms old_parm_file new_parm_file\n" );
    printf ( "    This would create a new parm file called 'new_parm_file'\n" );
    printf ( "    from 'old_parm_file\n\n" );
    printf ( "  For instance:\n    CvtParms Handshake.parms NewHandshake.parms\n\n" );
  }