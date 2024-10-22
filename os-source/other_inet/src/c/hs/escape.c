/* -----------------------------------------------------------------------
 * escape.c		handshake_src
 *
 * $Locker:  $
 *
 * $Id: escape.c,v 1.1 91/05/09 16:30:17 bj Exp $
 *
 * $Revision: 1.1 $
 *
 * $Header: HOG:Other/inet/src/c/hs/RCS/escape.c,v 1.1 91/05/09 16:30:17 bj Exp $
 *
 * $Log:	escape.c,v $
 * Revision 1.1  91/05/09  16:30:17  bj
 * Initial revision
 * 
 *
 *------------------------------------------------------------------------
 */

/***
*
* Escape sequence handling code.
*
***/

#include    "termall.h"

static unsigned char rcsid[] = "$Header: HOG:Other/inet/src/c/hs/RCS/escape.c,v 1.1 91/05/09 16:30:17 bj Exp $";

static unsigned char   decanm    = VT100;
static unsigned char   controls  = 7;
static unsigned char   ocontrols = 7;

/*---------------------------------------------------------------------------*/

/***
*
* Check to see if an escape sequence is over
*
***/
unsigned short int ESCOver (e, last_char)
register char   *e,
                *last_char;
  {
    register unsigned short int esc_length;

    esc_length = last_char - e + 1;

    if ( esc_length < 2 )
        return(0);

    if (*last_char == SUB ||
        *last_char == CAN )
      {
        ChangeFont ( DSCSFont );
        Vputc ( '?' );
        return ( TRUE );
      }

    if ( esc_length > MAXESCSEQ ) 
        return( TRUE );

    switch( e[1] )
      {
        case '#' :
        case '*' :
        case '+' :
        case '(' :
        case ')' :
        case ' ' :
            return((unsigned short int) (esc_length >= 3 && *last_char > 0x2f ));
        case '[' :
            return((unsigned short int) IsTerminator ( *last_char ) );
        case 'Y' :
            return( (unsigned short int)((e[1]=='Y') ? (unsigned short int) (esc_length == 4) :
                                  (unsigned short int) (esc_length == 2)) );
        default  : return( (unsigned short int) TRUE );
      }       
  }

/*---------------------------------------------------------------------------*/

unsigned short int IsTerminator ( ch )
char   ch;
  {
    return ( (unsigned short int)(isalpha ( ch ) || ch == '@' ));
  }

/*---------------------------------------------------------------------------*/

unsigned short int CSTCDI ( s , k )
char   *s;
int    k;
{
    char t[80];
    unsigned int i;
    
    if (!k) return(0);          /* if NULL string return */

    for ( i=0; i < k; i++ )
        t[i]=s[i];
    
    t[k] = '\0';
    stcd_i ( t, &i );
    return ( (unsigned short int) i );
}

/*---------------------------------------------------------------------------*/

void PnParse ( e, ep )
char               *e;
unsigned short int *ep;
{
    unsigned char   *tc,*ic;
    
    if ( !*e ) return;
    ic = e;                               /* Point to start of string */
    tc = e + strlen ( e );                /* Point past end of string */

    /* Scan for semi-colon or end of string */
    for (; ( (*e != ';' )  && ( e != tc ) ); e++ );
    
    ep[0]++;                              /* Increment parameter count   */
    ep[ ep[0] ] = CSTCDI ( ic, (int)(e - ic) );/* Convert Par and store */

    /* Return if done.             */
    if ( e == tc )
        return;
    PnParse ( e + 1, ep );           /* Recursively descend         */
}

/*---------------------------------------------------------------------------*/

void DefaultP (n,a,b)
register int n,a,b;
{
    if ( n > 2 ) return;
    ep[1] = (ep[0] ?
                   ( ep[1] ? ep[1] : a )
                   : a
            );
    ep[2] = ( ( n == 2) ?
                        ( ( ep[0] >= 2 ) ?
                                         ( ep[2] ? ep[2] : b )
                                         : b
                        )
                        : ep[2]
            );
    ep[0] = (ep[0] > n ) ? ep[0] : n;
}

/*---------------------------------------------------------------------------*/

void ResetP ()
{
    int n;

    for ( n = 0; n < MAXPN - 1; ep[n++] = 0 );
}

/*---------------------------------------------------------------------------*/

void Reverse (s)
register char    *s;
{
    register int c,i,j;

    for (i=0,j=strlen(s)-1;i<j;i++,j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

/*---------------------------------------------------------------------------*/

/***
*
* Handle dcs sequence
*
***/

void Dcs ( e )
register unsigned char  *e;
  {
    unsigned char   *sfnptr;
    unsigned char   *dcsptr;
    unsigned char   dcstype;
    unsigned short int count;
    unsigned char   oldname[4];
    
    if ( !Screen )
        return;

    strcpy ( oldname, SoftFontName );
    
    for ( dcsptr = e; *dcsptr && *dcsptr != '{' && *dcsptr != '|'; dcsptr++ );
    dcstype = *dcsptr;
    *dcsptr++ = '\0';
    
    switch ( dcstype )
      {
        case '|':
            return;
        case '{':
            ep[0] = 0;
            ep[1] = 1;
            ep[2] = 1;
            ep[3] = 1;
            ep[4] = 0;
            ep[5] = 0;
            ep[6] = 0;
            PnParse ( e+2, ep );
            
            for  ( count = 0, sfnptr = SoftFontName;; dcsptr++ )
              {
                if ( *dcsptr < 0x20 || ++count == 3 )
                    return;
                    
                *sfnptr++ = *dcsptr;
                if ( *dcsptr >= 0x30 )
                  {
                    *sfnptr = '\0';
                    break;
                  }
              }
              
            if ( strcmp ( oldname, SoftFontName ) != 0 )
                ClearSoftFont ();
                
            BuildSoftFont ( ++dcsptr );
            return;
        case '\0':
            return;
      }
  }

/*---------------------------------------------------------------------------*/

/***
*
* Handle Escape sequence
*
***/

void Escape (e)
register char   *e;
  {
    if ( !Screen )
        return;

    switch ( nvmodes.decanm )
      {
        case VT200_7:
        case VT200_8:
        case VT100  :
            VT100ESC ( e );
            break;
        case VT52   :
            VT52ESC ( e );
            break;
        default     :
            break;
      }
  }

/*---------------------------------------------------------------------------*/

void VT100ESC (e)
register char   *e;
  {
    switch (e[1])
      {
        case '[' :
            AnsiESC (e);
            break;
        case '*' :
        case '+' :
        case '(' :
        case ')' :
            ChstESC (e);
            break;
        case '#' :
            BigLetters (e);
            break;
        case ' ' :
            SC1TESC (e);
            break;
        case 'N' :
            tmodes.ss2 = glset;
            glset=2;
            break;
        case 'O' :
            tmodes.ss3 = glset;
            glset = 3;
            break;
        case 'n' : 
            glset=2;
            break;
        case 'o' :
            glset=3;
            break;
        case '|' :
            grset=3;
            break;
        case '}' :
            grset=2;
            break;
        case '~' :
            grset=1;
            break;
        default  :
            MiscESC (e);
            break;
      }
  }


/*---------------------------------------------------------------------------*/

void VT52ESC (e)
register char *e;
  {
    ResetP ();
    DefaultP (1,0,0);
    switch (e[1]) {
        case 'A' :
            Cuu ();
            break;
        case 'B' :
            Cud ();
            break;
        case 'C' :
            Cuf ();
            break;
        case 'D' :
            Cub ();
            break;
        case 'F' :
            g[0] = &vt52sgcsfont;
            g[1] = &vt52sgcsfont;
                   break;
        case 'G' :
            g[0] = &NormalFont;
            g[1] = &NormalFont;
            break;
        case 'H' :
            Cup ();
            break;
        case 'I' :
            Ri ();
            break;
        case 'J' :
            Ed (e);
            break;
        case 'K' :
            El (e);
            break;
        case 'V' :
            VT52Mc (e);
            break;
        case 'W' :
            VT52Mc (e);
            break;
        case 'X' :
            VT52Mc (e);
            break;
        case 'Y' :
            ep[0]=2;
            ep[1]=e[2]-'\037';
            ep[2]=e[3]-'\037';
            Cup ();
            break;
        case 'Z' :
            Ca (e);
            break;
        case '^' :
            VT52Mc (e);
            break;
        case '-' :
            VT52Mc (e);
            break;
        case ']' :
            VT52Mc (e);
            break;
        case '=' :
            tmodes.deckpm=APPLIC;
            SwitchKeyboard ();
            break;
        case '>' :
            tmodes.deckpm=NUMERIC;
            SwitchKeyboard ();
            break;
        case '<' :
            nvmodes.decanm = decanm;
            tmodes.controls = controls;
            tmodes.ocontrols = ocontrols;
            SwitchKeyboard ();
            if ( nvmodes.ukset )
                g[0] = &UKFont;
            else
                g[0] = &NormalFont;
            g[1] = &DSCSFont;
            glset = 0;
            grset = 1;
            vcb.linelen = 80;
            RefreshMenu ();
            break;
      }
  }

/*---------------------------------------------------------------------------*/

void AnsiESC (e)
register char *e;
  {
    int i;
    
    ep[0] = 0;
    i = ( ( e[2] == '?' ) ? 3 : 2 );
    PnParse ( e+i ,ep );
    switch ( e[ strlen( e ) - 1 ] )
      {
        case 'H' :
            Cup ();
            break;
        case 'A' :
            Cuu ();
            break;
        case 'B' : 
            Cud ();
            break;
        case 'C' :
            Cuf ();
            break;
        case 'D' :
            Cub ();
            break;
        case 'E' :
            Cnl ();
            break; /* ANSI only */
        case 'F' :
            Cpl ();
            break; /* ANSI only */
        case 'J' :
            Ed (e);
            break;
        case 'K' :
            El (e);
            break;
        case 'L' :
            Il ();
            break;
        case 'M' :
            Dl ();
            break;
        case 'P' :
            Dch ();
            break;
        case '@' :
            Ich ();
            break; /* ANSI & VT200 only */
        case 'c' :
            Ca (e);
            break;
        case 'f' :
            Cup ();
            break;
        case 'g' :
            Tbc ();
            break;
        case 'h' :
            ModeSet (e,   SET);
            break;
        case 'i' :
            Mc (e);
            break;
        case 'l' :
            ModeSet (e, RESET);
            break;
        case 'm' :
            Sgr (e);
            break;
        case 'n' :
            Dsr (e);
            break;
        case 'p' : /* VT200 only */
            DecSCL (e);
            break;
        case 's' :
            DecSC ();
            break;
        case 'u' :
            DecRC ();
            break;
        case 'q' : /* VT200 only */
            if ( nvmodes.decanm >= VT200_7 )
                DecSCA (e);
            else
                DecLL ();
            break;
        case 'r' :
            DecSTBM ();
            break;
        case 'x' :
            if ( nvmodes.decanm >= VT200_7 )
                Ech ();
            else  /* VT200 only */
                DecReqtParm ();
            break;
        case 'y' :
            DecTst ();
            break;
        default  :
            break;
      }
  }

/*---------------------------------------------------------------------------*/

void ChstESC (e)
register char *e;
  {
    int d;
    
    switch (e[1])
      {
        case '(' :
            d=0;
            break;
        case ')' :
            d=1;
            break;
        case '*' :
            d=2;
            break;
        case '+' :
            d=3;
            break;
        default  :
            return;
      }
      
    if ( strcmp ( e + 2, SoftFontName ) == 0 )
      {
        g[d] = &SoftFont;
      }
    else switch (e[2])
      {
        case 'A' :
            g[d] = &UKFont;
            nvmodes.ukset = SET;
            break;
        case '1' :
        case 'B' :
            g[d] = &NormalFont;
            nvmodes.ukset = RESET;
            break;
        case '2' :
        case '0' :
            g[d] = &SGCSFont;
            break;
        case '<' :
            g[d] = &DSCSFont;
            break;
      }
  }

/*---------------------------------------------------------------------------*/

void SC1TESC (e)
register char *e;
  {
    if ( nvmodes.decanm < VT200_7 )
       return;
       
    switch (e[2])
      {
        case 'F' :
            tmodes.ocontrols = 7;
            SwitchKeyboard ();
            break;
        case 'G' :
            tmodes.ocontrols = 8;
            SwitchKeyboard ();
            break;
      }
  }

/*---------------------------------------------------------------------------*/

void MiscESC (e)
register char *e;
  {
    switch (e[1])
      {
        case 'D' :
            Ind ();
            break;
        case 'E' :
            Nel ();
            break;
        case 'H' :
            Hts ();
            break;
        case 'M' :
            Ri () ;
            break;
        case '=' :
            tmodes.deckpm=APPLIC;
            SwitchKeyboard ();
            break;
        case '>' :
            tmodes.deckpm=NUMERIC;
            SwitchKeyboard ();
            break;
        /*
        case '<' :
            nvmodes.decanm=VT100;
            SwitchKeyboard ();
            g[0] = &NormalFont;
            g[1] = &SGCSFont;
            break;
        */
        case 'Z' :
            Ca (e);
            break;
        case '7' :
            DecSC ();
            break;
        case '8' :
            DecRC ();
            break;
        case 'c' :
            Ris ();
            break;
        default  :
            break;
      }
  }

/*---------------------------------------------------------------------------*/

void Cup ()
  {
    DefaultP (2,1,1);
    if ((ep[1]<1)||(ep[1]>vcb.bm-vcb.tm+1)||(ep[2]<1)||(ep[2]>vcb.linelen))
        return;

    /*
      {
        unsigned char message[81];

        sprintf ( message, "\x01\x14\x19<%d, %d>\x00\x00",
                  (long)ep[1], (long)ep[2] );
        *message = '\x00';
        DisplayAlert ( AG_NoMemory, message, 60L );
      }
    */
    
    Vlocate (ep[1]+vcb.tm-1,ep[2]);
  }

/*---------------------------------------------------------------------------*/

void Cuf ()
  {
    DefaultP (1,1,0);
    vcb.col = min(vcb.col+ep[1],vcb.linelen);
    Vsetcup ();
  }

/*---------------------------------------------------------------------------*/

void Cud ()
  {
    DefaultP (1,1,0);
    vcb.ln = min(vcb.ln+ep[1],vcb.sbm);
    Vsetcup ();
  }

/*---------------------------------------------------------------------------*/

void Cub ()
  {
    DefaultP (1,1,0);
    vcb.col = max( (((int)vcb.col)-((int)ep[1])),1);
    Vsetcup ();
  }

/*---------------------------------------------------------------------------*/

void Cuu ()
  {
    DefaultP (1,1,0);
    vcb.ln = max( (((int)vcb.ln)-((int)ep[1])),(int)vcb.stm);
    Vsetcup ();
  }

/*---------------------------------------------------------------------------*/

void Cnl ()
  {
    DefaultP (1,1,0);
    vcb.ln  = min(vcb.ln+ep[1],vcb.sbm);
    vcb.col = 1;
    Vsetcup ();
  }

/*---------------------------------------------------------------------------*/

void Cpl ()
  {
    DefaultP (1,1,0);
    vcb.ln  = max( (((int)vcb.ln)-((int)ep[1])),(int)vcb.stm);
    vcb.col = 1;
    Vsetcup ();
  }

/*---------------------------------------------------------------------------*/

void DecTst ()                                   
  {
  }

/*---------------------------------------------------------------------------*/

static unsigned char saved_fs,
                     saved_ea;

void DecSC ()
  {
    temp_g[0] = g[0];
    temp_g[1] = g[1];
    temp_g[2] = g[2];
    temp_g[3] = g[3];
    saved_fs  = tmodes.fontstyle;
    saved_ea  = tmodes.eraseattr;
    tmodes.savedrc.r = vcb.ln;
    tmodes.savedrc.c = vcb.col;
  }

/*---------------------------------------------------------------------------*/

void DecRC ()
  {
    g[0] = temp_g[0];
    g[1] = temp_g[1];
    g[2] = temp_g[2];
    g[3] = temp_g[3];
    tmodes.fontstyle = saved_fs;
    tmodes.eraseattr = saved_ea;
    vcb.ln = tmodes.savedrc.r;
    vcb.col= tmodes.savedrc.c;
    Vsetcup ();
    SetFontAttributes ();
  }

/*---------------------------------------------------------------------------*/

void Il ()
  {
    register int    scroll_max;

    if ( ( vcb.ln < vcb.stm ) ||
         ( vcb.ln > vcb.sbm )
       ) return;

    if ( !ep[1] )
        ++ep[1];

    VscrollDownAttr ( ep[1], (short)(vcb.ln - 1), (short)(vcb.sbm - 1) );
    
    scroll_max = min ( ep[1], vcb.sbm - vcb.ln + 1 );
    
    if ( Screen->BitMap.Depth != 2 )
        WaitTOF ();
        
    ScrollRaster ( RPort, 0L,   -scroll_max * 8L,
                          0L,   (vcb.ln  - 1L) * 8L + TOP_OFFSET,
                          ( SCR_WIDTH - 1L ), (vcb.sbm - 1L) * 8L + TOP_OFFSET + 7L );
  }

/*---------------------------------------------------------------------------*/

void Dl ()
  {
    register int    scroll_max;

    if ( ( vcb.ln < vcb.tm ) ||
         ( vcb.ln > vcb.bm )
       ) return;

    if ( !ep[1] )
        ++ep[1];

    VscrollUpAttr ( ep [1], (short)(vcb.ln - 1), (short)(vcb.sbm - 1) );
    
    scroll_max = min ( ep[1], vcb.sbm - vcb.ln + 1 );
    if ( Screen->BitMap.Depth != 2 )
        WaitTOF ();
        
    ScrollRaster ( RPort, 0L,   scroll_max * 8L,
                          0L,   (vcb.ln  - 1L) * 8L + TOP_OFFSET,
                          SCR_WIDTH - 1L, (vcb.sbm - 1L ) * 8L + TOP_OFFSET + 7L );
  }

/*---------------------------------------------------------------------------*/

void Ech ()
  {
    unsigned short int          FontSize ();
    register unsigned short int delete_size,
                                fs;

    if ( !ep[1] )
        ++ep[1];

    delete_size = ep[1] * ( fs = FontSize () );
    
    memset ( CharAttr    + ((vcb.ln - 1) * 132 + (vcb.col - 1)),
             FASCII, ep[1] );
    memset ( ScreenImage + ((vcb.ln - 1) * 132 + (vcb.col - 1)),
             ' ', ep[1] );

    if ( Screen->BitMap.Depth != 2 )
        WaitTOF ();
        
    ScrollRaster ( RPort, 0L,     8L,
                          (vcb.col-1L) * fs, (vcb.ln-1L) * 8L + TOP_OFFSET,
                          (vcb.col-1L) * fs + (long)delete_size,
                          (vcb.ln-1L) * 8L + TOP_OFFSET + 7L );
  }

/*---------------------------------------------------------------------------*/

void Ich ()
  {
    unsigned short int FontSize ();
    register short int insert_size,
                       fs;

    if ( !ep[1] )
        ++ep[1];

    VscrollRightAttr ( ep [ 1 ] );
    
    insert_size = ep[1] * ( fs = FontSize () );
    
    if ( Screen->BitMap.Depth != 2 )
        WaitTOF ();
        
    ScrollRaster ( RPort, -(long) insert_size,0L,
                          (vcb.col-1L) * fs, (vcb.ln-1L) * 8L + TOP_OFFSET,
                          ( SCR_WIDTH - 1L ),
                          ( vcb.ln-1L) * 8L + TOP_OFFSET + 7L );
  }

/*---------------------------------------------------------------------------*/

void Dch ()
  {
    unsigned short int          FontSize ();
    register unsigned short int delete_size,
                                fs;

    if ( !ep[1] )
        ++ep[1];

    VscrollLeftAttr ( ep[1] );
    
    delete_size = ep[1] * ( fs = FontSize () );
    
    if ( Screen->BitMap.Depth != 2 )
        WaitTOF ();
        
    ScrollRaster ( RPort, (long) delete_size,     0L,
                          (vcb.col-1L) * fs, (vcb.ln-1L) * 8L + TOP_OFFSET,
                          ( SCR_WIDTH - 1L ),             (vcb.ln-1L) * 8L + TOP_OFFSET + 7L );
  }

/*---------------------------------------------------------------------------*/

void Ind ()
  {
    if ( vcb.ln == vcb.sbm )
      {
        VscrollUp ();
        vcb.ln = vcb.sbm;
      }
    else vcb.ln += 1;
    Vsetcup ();
  }

/*---------------------------------------------------------------------------*/

void Nel ()
  {
    vcb.col = 1;
    Ind ();
  }

/*---------------------------------------------------------------------------*/

void Ri ()
  {
    if ( vcb.ln == vcb.stm )
      {
        VscrollDn ();
        vcb.ln = vcb.stm;
      }
    else
        vcb.ln -= 1;
    Vsetcup ();
  }

/*---------------------------------------------------------------------------*/

void DecSCA (e)
unsigned char *e;
  {
    if ( e[ strlen( e ) - 2 ] != '"' )
       return;
    
    switch ( ep[1] )
      {
        case 0:
        case 2:
           tmodes.eraseattr = ERASE;
           break;
        case 1:
           tmodes.eraseattr = NON_ERASE;
           break;
      }
  }

/*---------------------------------------------------------------------------*/

void Tbc ()
  {
    int i;

    switch (ep[1])
      {
        case 0 :    nvmodes.tabvec[vcb.col-1] = ' ';
                    TabSet ();
                    break;
        case 3 :    for (i=0;i<vcb.linelen;i++) nvmodes.tabvec[i] = ' ' ;
                    TabSet ();
                    break;
        default:    break;
      }
  }

/*---------------------------------------------------------------------------*/

void Hts ()
  {
    nvmodes.tabvec[vcb.col-1] = 'T';
    TabSet ();
  }

/*---------------------------------------------------------------------------*/
  
void Ris ()
  {
    *SoftFontName = '\0';
    ClearSoftFont ();
    Vcls ( 1, vcb.screenlen );
    vcb.ln =  1;
    vcb.col = 1;
    Vsetcup ();
    si2main_msg.opcode = RESET_TERMINAL;
    PutMsg ( main_task_port, (struct Message *) &si2main_msg );
    WaitPort ( si2main_msg.vt_msg.mn_ReplyPort );
    GetMsg ( si2main_msg.vt_msg.mn_ReplyPort );
    SetColors ();
  }

/*---------------------------------------------------------------------------*/

void PrintIt ( unsigned short int from, unsigned short int to,
               unsigned char      terminator )
  {
    if ( FindTask ( 0 ) == main_task )
        return;
        
    Print_from_line = from;
    Print_to_line   = to;
    Print_form_feed = terminator;
    si2main_msg.opcode = PRINT_LINES;
    PutMsg ( main_task_port, (struct Message *) &si2main_msg );
    WaitPort ( si2main_msg.vt_msg.mn_ReplyPort );
    GetMsg ( si2main_msg.vt_msg.mn_ReplyPort );
  }

/*---------------------------------------------------------------------------*/

void ControlPrint ( unsigned char c )
  {
    if ( tmodes.prtcntrl == 1 && !tmodes.printerhandle )
        return;
        
    Print_char = c;
    si2main_msg.opcode = CONTROL_PRINT;
    PutMsg ( main_task_port, (struct Message *) &si2main_msg );
    WaitPort ( si2main_msg.vt_msg.mn_ReplyPort );
    GetMsg ( si2main_msg.vt_msg.mn_ReplyPort );
  }

/*---------------------------------------------------------------------------*/

void Mc (e)
register char *e;
  {
    switch (e[2])
      {
        case '?':
            switch(ep[1])
              {
                case 1  :
                    PrintIt ( vcb.ln, vcb.ln, 0 );
                    break;
                case 4  :
                    tmodes.printing = FALSE;
                    break;
                case 5  :
                    tmodes.printing = TRUE;
                    break;
              }
            break;
        case 'i':
            if ( nvmodes.decpex )
                PrintIt ( 1, vcb.screenlen,
                          (unsigned char)( nvmodes.decpff ? 0x0c : 0 ) );
            else
                PrintIt ( vcb.stm, vcb.sbm,
                          (unsigned char)( nvmodes.decpff ? 0x0c : 0 ) );
            break;
        default :
            switch(ep[1])
              {
                case 0  :
                    if ( nvmodes.decpex )
                        PrintIt ( 1, vcb.screenlen,
                                  (unsigned char)( nvmodes.decpff ? 0x0c : 0 ) );
                    else
                        PrintIt ( vcb.stm, vcb.sbm,
                                  (unsigned char)( nvmodes.decpff ? 0x0c : 0 ) );
                    break;
                case 4  :
                    tmodes.prtcntrl = FALSE;
                    break;
                case 5  :
                  {
                    unsigned char       last_four[5];
                    unsigned short int  last_idx;
                    unsigned char       c;
                    
                    last_idx = 0;
                    tmodes.prtcntrl = 3;        /* Open Printer */
                    ControlPrint ( '\x00' );
                    
                    while (tmodes.prtcntrl )
                      {
                        if ( !tmodes.prtcntrl || !tmodes.printerhandle )
                            break;
                            
                        c = Lgetc ();
                        if ( c == NULL || c == XON || c == XOFF )
                            continue;
                        else if ( c == CSI )
                          {
                            last_four[0] = ESC;
                            last_four[1] = '[';
                            last_idx = 2;
                          }
                        else if ( c == ESC )
                          {
                            last_four[0] = ESC;
                            last_idx = 1;
                          }
                        else if ( last_idx )
                          {
                            last_four[ last_idx++ ] = c;
                            if ( last_idx == 4 )
                              {
                                if ( strncmp ( last_four, "\x1b[4i", 4 ) == 0 )
                                  {
                                     tmodes.prtcntrl = 2;  /* Close Printer */
                                     ControlPrint ( '\x00' );
                                     tmodes.prtcntrl = FALSE;
                                     break;
                                  }
                                else
                                  {
                                    tmodes.prtcntrl = 1;
                                    for ( last_idx = 0; last_idx < 4; last_idx++ )
                                        ControlPrint ( last_four[ last_idx ] );
                                    last_idx = 0;
                                  }
                              }
                          }
                        else
                          {
                            tmodes.prtcntrl = 1;
                            ControlPrint ( c );
                          }
                      }
                  }
                break;
              }
      }
  }

/*---------------------------------------------------------------------------*/

void VT52Mc (e)
register char *e;
  {
    switch (e[2])
      {
        case 'V':
            PrintIt ( vcb.ln, vcb.ln, 0 );
            break;
        case 'W':
            tmodes.prtcntrl = FALSE;
            break;
        case 'X':
            tmodes.prtcntrl = TRUE;
            break;
        case '-':
            tmodes.printing = FALSE;
            break;
        case '^':
            tmodes.printing = TRUE;
            break;
        case ']':
            PrintIt ( 1, vcb.screenlen,
                      (unsigned char)( nvmodes.decpff ? 0x0c : 0 ) );
            break;
      }
  }

/*---------------------------------------------------------------------------*/

void Dsr (e)
register char *e;
  {
    switch (e[2])
      {
        case '?':
            switch (ep[1])
              {
                case 15 :
                    if ( tmodes.ocontrols == 7 )
                        Respond ( "\x1b[?10n" );
                    else
                        Respond ( "\x9b?10n" );
                    /*
                    prt_report();
                    */
                    break;
                default:
                    break;
              }
            break;
        default :
            switch (ep[1])
              {
                case 5  :
                    if ( tmodes.ocontrols == 7 )
                        Respond ( "\x1b[0n" );
                    else
                        Respond ( "\x9b\x30n" );
                    break;
                case 6  :   Cpr ();
                            break;
              }
            break;
      }
  }

/*---------------------------------------------------------------------------*/

void DecLL ()
  {
    register unsigned short int i;

    DefaultP ( 1, 0, 0 );
    for ( i = 1; i <= ep[0]; i++ )
      {
        switch ( ep[i] )
          {
            case 0:
                Title_line [ 50 ] = 0xb0;
                Title_line [ 52 ] = 0xb0;
                Title_line [ 54 ] = 0xb0;
                Title_line [ 56 ] = 0xb0;
                break;
            case 1:
                Title_line [ 50 ] = 0xa4;
                break;
            case 2:
                Title_line [ 52 ] = 0xa4;
                break;
            case 3:
                Title_line [ 54 ] = 0xa4;
                break;
            case 4:
                Title_line [ 56 ] = 0xa4;
                break;
          }
      }
    si2main_msg.opcode = SHOW_TITLE;
    PutMsg ( main_task_port, (struct Message *) &si2main_msg );
    WaitPort ( si2main_msg.vt_msg.mn_ReplyPort );
    GetMsg ( si2main_msg.vt_msg.mn_ReplyPort );
  }

/*---------------------------------------------------------------------------*/

void Cpr ()
  {
    char strng[6];

    if ( tmodes.ocontrols == 7 )
        Respond ( "\x1b[" );
    else
        Respond ( "\x9b" );
    stcu_d ( strng, (unsigned int) vcb.ln);
    Respond (strng);
    Respond ( ";" );
    stcu_d ( strng, (unsigned int) vcb.col);
    Respond ( strng );
    Respond ( "R" );
  }

/*---------------------------------------------------------------------------*/

void DecReqtParm ()
  {
   
    if (ep[1] == 2) return;
    tmodes.sol = ep[1];
    DecReptParm ();
  }

/*---------------------------------------------------------------------------*/

void DecReptParm ()
  {
    if ( tmodes.sol == 0 )
        Respond ( (tmodes.ocontrols == 7 ) ? "\x1b[2;" : "\x9b\x32;" );
    else
        Respond ( (tmodes.ocontrols == 7 ) ? "\x1b[3;" : "\x9b\x33;" );

    switch ( nvmodes.parity )
      {
        case 0 :
        case 3 :
        case 4 : Respond ("1;"); break;
        case 1 : Respond ("4;"); break;
        case 2 : Respond ("5;"); break;
      }
    switch (nvmodes.bpc)
      {
        case 7 : Respond ("2;"); break;
        case 8 : Respond ("1;"); break;
      }
    switch (nvmodes.lspeed)
      {
        case  110 :    Respond ("16;16;"); break;     /*  110   */
        case  300 :    Respond ("48;48;"); break;     /*  300   */
        case  450 :    Respond ("52;52;"); break;     /*  450   */
        case  600 :    Respond ("56;56;"); break;     /*  600   */
        case 1200 :    Respond ("64;64;"); break;     /* 1200   */
        case 1800 :    Respond ("72;72;"); break;     /* 1800   */
        case 2400 :    Respond ("88;88;"); break;     /* 2400   */
        case 3600 :    Respond ("96;96;"); break;     /* 3600   */
        case 4800 :    Respond ("104;104;"); break;   /* 4800   */
        case 9600 :    Respond ("112;112;"); break;   /* 9600   */
        case 19200:    Respond ("120;120;"); break;   /* 19200  */
      }
    Respond ("1;0x");
  }

/*---------------------------------------------------------------------------*/

void SetFontAttributes ()
  {
   unsigned long int ss_bits;
   
   ss_bits = (tmodes.fontstyle >> 3) & ( FSF_BOLD | FSF_UNDERLINED );
   SetSoftStyle ( RPort, ss_bits, FSF_BOLD | FSF_UNDERLINED );
   SetAPen ( RPort, (tmodes.fontstyle & GRF_BLINK) ?
        ( ( 1L << Screen->BitMap.Depth )  - 1 )
        : vcb.fgcolor );
   SetDrMd ( RPort, (long)((tmodes.fontstyle & GRF_REVERSE) ? JAM2 | INVERSVID
                                                     : JAM2));
  }

/*---------------------------------------------------------------------------*/

void Sgr (e)
register char   *e;
  {
    register unsigned short int i;

    DefaultP ( 1, 0, 0 );
    for ( i = 1; i <= ep[0]; i++ )
      {
        switch ( ep[i] )
          {
            case 0:
                tmodes.fontstyle = GR_NORMAL;
                break;
            case 1:
                tmodes.fontstyle |= GRF_BOLD;
                break;
            case 4:
                tmodes.fontstyle |= GRF_UNDERLINED;
                break;
            case 5:
                tmodes.fontstyle |= GRF_BLINK;
                break;
            case 7:
                tmodes.fontstyle |= GRF_REVERSE;
                break;
            case 22:
                tmodes.fontstyle &= ~GRF_BOLD;
                break;
            case 24:
                tmodes.fontstyle &= ~GRF_UNDERLINED;
                break;
            case 25:
                tmodes.fontstyle &= ~GRF_BLINK;
                break;
            case 27:
                tmodes.fontstyle &= ~GRF_REVERSE;
                break;
          }
        if ( ep[i] >= 30 &&Screen->BitMap.Depth != 2 )
            switch ( ep[i] )
              {
                case 30:
                    SetAPen ( RPort, (long )( vcb.fgcolor = 0 ) );
                    break;
                case 31:
                    SetAPen ( RPort, (long )( vcb.fgcolor = 1 ) );
                    break;
                case 32:
                    SetAPen ( RPort, (long )( vcb.fgcolor = 2 ) );
                    break;
                case 33:
                    SetAPen ( RPort, (long )( vcb.fgcolor = 3 ) );
                    break;
                case 34:
                    SetAPen ( RPort, (long )( vcb.fgcolor = 4 ) );
                    break;
                case 35:
                    SetAPen ( RPort, (long )( vcb.fgcolor = 5 ) );
                    break;
                case 36:
                    SetAPen ( RPort, (long )( vcb.fgcolor = 6 ) );
                    break;
                case 37:
                    SetAPen ( RPort, (long )( vcb.fgcolor = 7 ) );
                    break;
                    
                case 40:
                    SetBPen ( RPort, (long )( vcb.bgcolor = 0 ) );
                    break;
                case 41:
                    SetBPen ( RPort, (long )( vcb.bgcolor = 1 ) );
                    break;
                case 42:
                    SetBPen ( RPort, (long )( vcb.bgcolor = 2 ) );
                    break;
                case 43:
                    SetBPen ( RPort, (long )( vcb.bgcolor = 3 ) );
                    break;
                case 44:
                    SetBPen ( RPort, (long )( vcb.bgcolor = 4 ) );
                    break;
                case 45:
                    SetBPen ( RPort, (long )( vcb.bgcolor = 5 ) );
                    break;
                case 46:
                    SetBPen ( RPort, (long )( vcb.bgcolor = 6 ) );
                    break;
                case 47:
                    SetBPen ( RPort, (long )( vcb.bgcolor = 7 ) );
                    break;
              }

      }
    SetFontAttributes ();
  }

/*---------------------------------------------------------------------------*/

void Ed (e)
unsigned char *e;
  {
    DefaultP (1,0,0);
    
    if ( e[2] == '?' )
      {
        DecSED ();
        return;
      }
      
    switch (ep[1])
      {
        case 0 :    VwipeEOL ();
                    LineAttr [ vcb.ln - 1 ] = SWSH;
                    if (vcb.ln < vcb.screenlen)
                         Vcls ( vcb.ln+1, vcb.screenlen );
                    break;
        case 1 :    VwipeSOL ();
                    LineAttr [ vcb.ln - 1 ] = SWSH;
                    if (vcb.ln > 1)
                         Vcls ( 1,vcb.ln-1);
                    break;
        case 2 :    Vcls ( 1, vcb.screenlen );
                    break;
      }
    Vsetcup ();
  }

/*---------------------------------------------------------------------------*/

void El (e)
unsigned char *e;
  {
    register unsigned short int x;
    
    DefaultP (1,0,0);
    
    if ( e[2] == '?' )
      {
        DecSEL ();
        return;
      }
      
    switch (ep[1])
      {
        case 0 : VwipeEOL ();
                 break;
        case 1 : VwipeSOL ();
                 break;
        case 2 : x = vcb.col;
                 Vlocate ( vcb.ln, 1 );
                 VwipeEOL ();
                 Vlocate ( vcb.ln, x );
      }
    Vsetcup ();
  }

/*---------------------------------------------------------------------------*/

void DecSED ()
  {
    DefaultP (1,0,0);
    switch (ep[1])
      {
        case 0 :    VswipeEOL ();
                    if (vcb.ln < vcb.screenlen)
                         Vscls ( vcb.ln+1, vcb.screenlen );
                    break;
        case 1 :    VswipeSOL ();
                    if (vcb.ln > 1)
                         Vscls ( 1,vcb.ln-1);
                    break;
        case 2 :    Vscls ( 1, vcb.screenlen );
                    break;
      }
    Vsetcup ();
  }

/*---------------------------------------------------------------------------*/

void DecSEL ()
  {
    register unsigned short int x;
    
    DefaultP (1,0,0);
    switch (ep[1])
      {
        case 0 : VswipeEOL ();
                 break;
        case 1 : VswipeSOL ();
                 break;
        case 2 : x = vcb.col;
                 Vlocate ( vcb.ln, 1 );
                 VswipeEOL ();
                 Vlocate ( vcb.ln, x );
      }
    Vsetcup ();
  }

/*---------------------------------------------------------------------------*/

void Ca (e)
register char *e;
  {
    if (e[2] == 'c' || ep[1] == 0 || e[1] == 'Z')
      {
        if ( nvmodes.decanm >= VT100 )
            Respond ((tmodes.ocontrols == 7) ? "\x1b[?62;1;2;6;7;8c" :
                                              "\x9b?62;1;2;6;7;8c" );
        /*
        else if ( nvmodes.decanm == VT100 && nvmodes.termid == RESET)
            Respond ((tmodes.ocontrols == 7) ? "\x1b[?1;2c" : "\x9b?1;2c" );
        else if ( nvmodes.decanm == VT100 && nvmodes.termid == SET )
            Respond ((tmodes.ocontrols == 7) ? "\x1b[?6c" : "\x9b?6c" );
        */
        else if ( nvmodes.decanm == VT52 && e[1] == 'Z')
            Respond ( "\x1b/Z" );
      }
    else if ( e[2] == '>' )
      {
        if ( nvmodes.decanm >= VT100 )
            Respond ((tmodes.ocontrols == 7) ? "\x1b[>1;10;0c" :
                                              "\x9b>1;10;0c" );
      }
  }

/*---------------------------------------------------------------------------*/

void BigLetters (e)
register char *e;
  {
    if ( e[2] == '8' )
        DecALN ();
    else
        VsetLAttr ( e[2] );
  }

/*---------------------------------------------------------------------------*/

void DecALN ()
  {
    register unsigned short int i,
                                j;
    Vcls ( 1, vcb.screenlen );
    for ( i = 1; i < vcb.screenlen+1; i ++ )
      {
        Vlocate ( i, 1 );
        for ( j = 1; j <= vcb.linelen; j++ )
            VtDspCh ( 'E' );
      }
    Vlocate ( 1, 1 );
  }

/*---------------------------------------------------------------------------*/

void DecSTBM ()
  {
    DefaultP ( 2, 1, vcb.screenlen );
    ep[1] = max ( ep[1], 1 );
    ep[2] = max ( ep[2], 1 );
    if ( ( ep[1] < 1  ) ||
         ( ep[1] > vcb.screenlen ) ||
         ( ep[2] < 1  ) ||
         ( ep[2] > vcb.screenlen ) ||
         ( ep[1] >= ep[2] )
       ) return;

    vcb.stm = ep[1];
    vcb.sbm = ep[2];

    if (tmodes.decom==SET)
      {
        vcb.tm = vcb.stm;
        vcb.bm = vcb.sbm;
      }
    Vlocate (vcb.tm,1);
  }

/*---------------------------------------------------------------------------*/

void DecSCL (e)
unsigned char *e;
  {
    unsigned short int tep;
    
    if ( e[ strlen( e ) - 2 ] != '"' )
       return;
       
    if ( ep[0] == 0 )
        return;
        
    tep = ep[0];
    DefaultP ( 1, 0, 0 );
    ep[0] = tep;
    
    switch ( ep[1] )
      {
        case 61:
            nvmodes.decanm  = VT100;
            tmodes.controls = 7;
            break;
        case 62:
           if (ep[0] == 1 )
             {
               nvmodes.decanm  = VT200_8;
               tmodes.controls = 8;
               break;
             }
           switch ( ep[2] )
             {
                case 0:
                case 2:
                    nvmodes.decanm  = VT200_8;
                    tmodes.controls = 8;
                    break;
                case 1:
                    nvmodes.decanm  = VT200_7;
                    tmodes.controls = 7;
                    break;
             }
           break;
        default:
           break;
      }
    SwitchKeyboard ();
    RefreshMenu ();
  }

/*---------------------------------------------------------------------------*/

void ModeSet (e, is)
register char    *e;
register int     is;
  {
    unsigned short int n;

    switch (e[2])
      {
        case '?' :
            for  ( n = 1; ep[0]; n++, ep[0]-- )
              {
                switch (ep[n]) /* DEC Private modes */
                  {
                    case 1 :
                        tmodes.decckm=is;
                        SwitchKeyboard ();
                        break;
                    case 2 :
                        if ( is == SET )
                            break;
                        decanm    = nvmodes.decanm;
                        controls  = tmodes.controls;
                        ocontrols = tmodes.ocontrols;
                        nvmodes.decanm = VT52;
                        tmodes.deccolm = RESET;
                        g[0] = &NormalFont;
                        g[1] = &NormalFont;
                        glset = 0;
                        vcb.linelen = 80;
                        SwitchKeyboard ();
                        RefreshMenu ();
                        break;
                    case 3 :
                        tmodes.deccolm=is;
                        if ( is == RESET )
                            vcb.linelen = 80;
                        else
                            vcb.linelen = 132;
                        Vcls ( 1, vcb.screenlen );
                        Vlocate (1, 1);
                        SetFont ( RPort, *g[glset] );
                        RefreshMenu ();
                        break;
                    case 4 :
                        tmodes.decsclm=is;
                        RefreshMenu ();
                        break;
                    case 5 :
                        if (is != nvmodes.decscnm)
                          {
                            nvmodes.decscnm=is;
                            SwapFgBg ();
                            RefreshMenu ();
                          }
                        break;
                    case 6 :
                        tmodes.decom=is;
                        vcb.tm=((is==SET) ? vcb.stm : 1);
                        vcb.bm=((is==SET) ? vcb.sbm : vcb.screenlen);
                        Vlocate (vcb.tm,1);
                        break;
                    case 7 :
                        nvmodes.decawm=is;
                        RefreshMenu ();
                        break;
                    case 8 :
                        tmodes.decarm=is;
                        SwitchKeyboard ();
                        break;
/*
                    case 9 :
                        tmodes.decinlm=is;
                        break;
*/
                    case 18 :
                        nvmodes.decpff=is;
                        break;
                    case 19 :
                        nvmodes.decpex=is;
                        break;
                    case 25 :
                        tmodes.dectcem=is;
                        if ( tmodes.dectcem == SET )
                            RemoveCursor ();
                        else
                            PlaceCursor ();
                        break;
                    default :
                        break;
                  }
              }
            break;
        default  :
            for  ( n = 1; ep[0]; n++, ep[0]-- )
              {
                switch (ep[n]) /* ANSI Compatible set modes */
                  {
                    case 2 :
                        tmodes.kam = is;
                        if ( is == SET )
                            strcpy ( Title_line + 58, "Kbd Lcked" );
                        else
                            strcpy ( Title_line + 58, "         " );
                            
                        si2main_msg.opcode = SHOW_TITLE;
                        PutMsg ( main_task_port, (struct Message *) &si2main_msg );
                        WaitPort ( si2main_msg.vt_msg.mn_ReplyPort );
                        GetMsg ( si2main_msg.vt_msg.mn_ReplyPort );
                        break;
                    case 4  :
                        tmodes.irm=is;
                        break;
                    case 12 :
                        nvmodes.echo=is;
                        RefreshMenu ();
                        break;
                    case 20 :
                        nvmodes.lnm=is;
                        RefreshMenu ();
                        break;
                    default :
                        break;
                  }
              }
      }
  }

/*---------------------------------------------------------------------------*/

void Respond ( str )
register unsigned char   *str;
  {
    ser_rp_req.IOSer.io_Command = CMD_WRITE;
    ser_rp_req.IOSer.io_Data    = (APTR) str;
    ser_rp_req.IOSer.io_Length  = strlen ( str );
    ser_req.IOSer.io_Flags     &= ~IOF_QUICK;
    SendIO ( (struct IORequest *) &ser_rp_req );
    WaitIO ( (struct IORequest *) &ser_rp_req );
  }

/*---------------------------------------------------------------------------*/

void SwitchKeyboard ()
  {
    si2main_msg.opcode = SWITCH_KEYBOARD;
    PutMsg ( main_task_port, (struct Message *) &si2main_msg );
    WaitPort ( si2main_msg.vt_msg.mn_ReplyPort );
    GetMsg ( si2main_msg.vt_msg.mn_ReplyPort );
  }
