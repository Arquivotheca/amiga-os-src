/* -----------------------------------------------------------------------
 * mainloop.c   rlogin v38.x   rewrite
 *
 *  This is the main loop of the appplication. This file also contains
 *  the support routines for jump scrolling.
 *
 * $Locker:  $
 *
 * $Id: mainloop.c,v 38.0 93/04/08 15:27:49 bj Exp $
 *
 * $Revision: 38.0 $
 *
 * $Log:	mainloop.c,v $
 * Revision 38.0  93/04/08  15:27:49  bj
 * This is the complete rewrite of rlogin (v 38.xx.)
 * This is the main loop as well as the code that handles
 * the jump scrolling.
 * 
 *
 * $Header: AS225:src/c/rlogin/RCS/mainloop.c,v 38.0 93/04/08 15:27:49 bj Exp $
 *
 *------------------------------------------------------------------------
 */


#include "rl.h"

VOID
mainloop(struct glob *g)
{
	SYSBASE ;
	DOSBASE(g) ;
	SOCKBASE(g) ;

	REGISTER ULONG setbits ;
	struct MsgPort *crp ;
	
	ULONG  consolesig,
	       netevent, 
	       urgevent,
	       clipevent,
	       waitmask,
	       fkeynum,
	       sfkeynum,
	       len,
	       chars_in_scroll_buffer ;
	       
	WORD   oldwin_y,
	       scroll_block_count ;

	UBYTE  c, 
	       tmpc,
	       clipchar,
	       ctrlbuf[512] ;
	       
	UBYTE  *bp, *bp2, *sbp, *tmpp, *ppp ;
	
	int    bcnt = 0, 
	       incontrol = 0, 
	       dobreak = 0, 
	       cnt,
	       cc ;

	UBYTE closegad_on[]  = { '\x9b', '1','1','{','\0' } ;
	UBYTE closegad_off[] = { '\x9b', '1','1','}','\0' } ;
	UBYTE linewrap_on[]  = { '\x9b', '\x3f', '\x37', '\x68' } ;
	UBYTE gimme_resize[] = { '\x9b', '1','2','{','\0' } ;
	UBYTE menu_on[]      = { '\x9b', '1','0','{','\0' } ;

	g->g_error = 0L ;

	clipevent  = g->g_clipevent ;
	netevent   = g->g_netevent ;
	urgevent   = g->g_urgevent ;
	crp        = g->g_ConReadPort ;
	consolesig = 1L << crp->mp_SigBit ;
	waitmask   = SIGBREAKF_CTRL_C | consolesig | netevent | urgevent | clipevent ;

	MENU_ON ;
	CLOSEGAD_ON ;
	LINEWRAP_ON ;
	
	if( g->g_user_resize)
	{
		RESIZE_ON ;
	}
	
	g->g_conwrite_was_used = 1 ;
	oldwin_y = g->g_Window->Height ;
	
	bp = g->g_netbuffer ;
	g->g_getout = FALSE ;

	do
	{
		setbits = Wait(waitmask) ;

		if(setbits & SIGBREAKF_CTRL_C)
		{
			g->g_getout = TRUE ;
		}

		if(setbits & consolesig)
		{
			c = (console_getchar(g) & 0xFF) ;

			if(incontrol)
			{
				ctrlbuf[bcnt] = c ;
				bcnt++ ;

				if( ESC_SEQ_TERMINATOR(c) )
				{
					if( c == CON_IDCMP_REPORT ) 
					{
						if( CON_GOTMENU( ctrlbuf ))
						{
							HandleMenu( g, ctrlbuf ) ;
						}
						else if( CON_CLOSEGAD( ctrlbuf ))
						{
							CLOSEGAD_OFF ;
							if((g->g_getout = AskExit(g)) == 0L)
							{
								CLOSEGAD_ON ;
							}
						}
						else if( CON_RESIZE( ctrlbuf ))
						{
							send_window_size( g ) ;
							if( g->g_user_conunit == 0L )
							{
								if( g->g_Window->Height < oldwin_y )
								{
									send(g->g_socket, "\r", 1, 0) ;
								}
							}
							oldwin_y = g->g_Window->Height ;
						}
					}
					else if( c == CONCLIP_TERM && CONCLIP_PENDING(ctrlbuf)) 
					{
						if(MakeClipMine(g))
						{
							Signal(g->g_myself, clipevent) ;
						}
					}
					else if (c == CON_SPECIAL_KEY)
					{
						if( ctrlbuf[3] == TILDE ) // fkey or help key
						{
							if( ((tmpc = ctrlbuf[2]) >= '0') && tmpc <= '9' )
							{
								if(g->g_localfkeys)
								{
									fkeynum = (LONG)(tmpc - '0') ;
//									mysprintf(g->g_buffer, "fkey %ld pressed\n", fkeynum+1L) ;
//									PutStr(g->g_buffer) ;
									if(g->g_fkeys[fkeynum][0])
									{
										len = (long)strlen(g->g_fkeys[fkeynum]) ;
										send(g->g_socket, g->g_fkeys[fkeynum], len, 0) ;
//										mysprintf(g->g_buffer, "len string = %ld\n", len) ;
//										PutStr(g->g_buffer) ;
										if(g->g_fklf[fkeynum])
										{
											send(g->g_socket, "\n", 1L, 0 ) ;
//											PutStr("LF to be sent here\n") ;
										}
//										for( cc = 0, tmpp = g->g_fkeys[fkeynum] ; cc < len ; cc++, tmpp++) 
//										{
//											mysprintf(g->g_buffer, "char #%ld = %03ld/%lc\n", cc, (long)*tmpp, (long)*tmpp) ;
//											PutStr(g->g_buffer) ;
//										}
									}
								}
								else
								{
									send(g->g_socket, ctrlbuf, bcnt, 0) ;
								}
							}
						}
					}
						
					else 
					{
						send( g->g_socket, ctrlbuf, bcnt, 0 ) ;
					}
					bcnt = incontrol = 0 ;
				}

			}
			else if( c == 0X9B ) // start ctrl seq
			{
				incontrol = 1 ;
				ctrlbuf[bcnt] = 0X1B ;
				bcnt++ ;
				ctrlbuf[bcnt] = '[' ;
				bcnt++ ;
			}
			else
			{
				send( g->g_socket, &c, 1, 0 ) ;
				bcnt = incontrol = 0 ;
			}
		}

		if(setbits & urgevent )
		{
			handle_oob(g) ;
		}

		if(setbits & netevent )
		{
			chars_in_scroll_buffer = 0 ;
			scroll_block_count     = 0 ;
			sbp = g->g_scroll_buffer ;
			
			while( (cc = recv(g->g_socket, (char *)bp, (int)SCROLLBLOCK_SIZE, 0)) > 0)
			{
				if((SetSignal(0L,0L)) & urgevent )
				{
					dobreak = 1 ;
				}

				if(console_char_ready(g))
				{
					dobreak = 1 ;
				}

				for( cnt = 0, bp2 = (UBYTE *)bp ; cnt < cc ; cnt++, bp2++)
				{
					if( (*bp2 > 127) && (*bp2 < 160) && (*bp2 != 155 /* escape */))
					{
						*bp2 &= 0x7f ;
					}
				}

				if( g->g_user_jump == FALSE )
				{
					if(cc != -1)
					{
						con_write( bp, (LONG)cc, g ) ;
					}

					if( dobreak )
					{
						Signal( g->g_myself, netevent ) ; 
						dobreak = 0 ;
						break ;
					}
				}
				else
				{
					CopyMem(bp,sbp,(ULONG)cc) ;
					scroll_block_count++ ;
					sbp += cc ;
					chars_in_scroll_buffer += cc ;
					
					if( (cc < SCROLLBLOCK_SIZE) || (scroll_block_count >= SCROLLBLOCK_COUNT))
					{
						flush_scroll_buffer(g->g_scroll_buffer, chars_in_scroll_buffer, g) ;
						Signal(g->g_myself, netevent) ;
						chars_in_scroll_buffer = 0 ;
						scroll_block_count     = 0 ;
						sbp = g->g_scroll_buffer ;
					}
				}
				if(dobreak)
				{
					Signal(g->g_myself, netevent) ;
					dobreak = 0 ;
					break ;
				}
			}
			if( g->g_user_jump == TRUE)
			{
				flush_scroll_buffer( g->g_scroll_buffer, chars_in_scroll_buffer,g) ;
			}


			if(g->g_errno != EWOULDBLOCK)
			{
				if( g->g_errno == 0 && cc <= 0 )
				{
					g->g_getout = TRUE ;
				}
			}
		} 
		
		if( setbits & clipevent) 
		{
			if( GetClipChar(&clipchar, g) == 0L )
			{
				if( clipchar )
				{
					send(g->g_socket, &clipchar, 1, 0) ;
					if( clipchar <= 13 && g->g_paste_delay >= 1L )
					{
						Delay(g->g_paste_delay) ;
					}
				}
				Signal(g->g_myself, clipevent) ; /* set the signal again ! */
			}
			else
			{
				g->g_clip_in_progress = FALSE ;
				KillPaste(g) ;
			}
		}
	}
	while(g->g_getout == FALSE) ;
}

////////////////////////////////////////////////////////
// The SCROLLBUFFER value is made up of X _SCROLLBLOCK_s
// 
// AS this is written it is 10 scrollblocks of size 256 bytes
// to make up a 2560 buffer. This 256 byte value coincides with 
// the maximum number of chars read from the net per call to
// recv(). These values can be changed via editing the file 
// _rl.h_ and recompiling.
/////////////////////////////////////////////////////////

/* ======================================================================
 * FlushScrollBuffer() 
 *
 * This function takes a raw buffer of text from the network, counts
 * the number of "lines" in the buffer, calculates the number of lines
 * the console needs to scroll upwards, scroll that amount and outputs
 * the text.  
 *
 * This function has known limitations. In particular, it is possible
 * for a network application to overwhelm this by sending a large amount
 * of control sequences (the U*ix mail reader 'elm' causes this.) See
 * the note for 'countlfs()'.
 * ======================================================================
 */

VOID
flush_scroll_buffer( BYTE *buf, ULONG numchars, struct glob *g)
{
	REGISTER WORD linefeeds ;
	REGISTER ULONG x ;
	REGISTER UBYTE *bp ;
	REGISTER LONG xmax = (LONG)g->g_ConUnit->cu_XMax + 1 ;
	REGISTER LONG ymax = (LONG)g->g_ConUnit->cu_YMax + 1 ;

	linefeeds  = countlfs(buf,numchars, xmax, g) ;

	if( linefeeds > ymax )
	{
		bp = (UBYTE *)buf ;
		x = 0 ;
		while((numchars >= SCROLLBLOCK_SIZE) && (x < SCROLLBLOCK_COUNT))
		{
			doscroll( countlfs( bp,(LONG)SCROLLBLOCK_SIZE,(ULONG)xmax,g),g) ;
			con_write(bp, SCROLLBLOCK_SIZE, g) ;
			bp += SCROLLBLOCK_SIZE ;
			numchars -= SCROLLBLOCK_SIZE ;
			x++ ;
		}
		doscroll(countlfs(bp,numchars,xmax,g),g) ;
		con_write(bp,numchars,g) ;
	}
	else
	{
		doscroll(linefeeds,g) ;
		con_write(buf,numchars,g) ;
	}
}

/* ========================================================================
 * countlfs() - this function takes a buffer of text, the number of chars
 * in the buffer and the  current width of the display and atempts to give
 * you an idea of how many actual line feeds would be required to display
 * the text with only one scroll operation.
 *
 * The current method can get quite confused by things such as the Unix
 * mail reader "elm" which can send globs of control characters at once,
 * fooling the count. This needs to be improved.
 * ========================================================================
 */

ULONG
countlfs( UBYTE *b, ULONG cnt, ULONG wide, struct glob *g )
{
	REGISTER UBYTE *bp1 ;
	REGISTER ULONG x,lfs, llen ;

	for( bp1 = b, lfs = llen = x = 0L; x < cnt ; x++, bp1++)
	{
		if( *bp1 == 10 )
		{
			lfs++ ;
			llen = 0L ;
		}
		if( g->g_user_linewrap && (llen > wide) )
		{
			lfs++ ;
			llen =  0L ;
		}
	}
	return(lfs) ;
}

/* =======================================================================
 * doscroll()    this function does the actual jump scrolling
 * ======================================================================
 */

VOID
doscroll(ULONG lines, struct glob *g)
{
	REGISTER LONG xpos = (LONG)g->g_ConUnit->cu_XCP  + 1 ;
	REGISTER LONG ypos = (LONG)g->g_ConUnit->cu_YCP  + 1 ;
	REGISTER LONG ymax = (LONG)g->g_ConUnit->cu_YMax + 1 ;

	REGISTER LONG scrollup ;
	REGISTER LONG jump ;
	REGISTER UBYTE *b = g->g_buffer ;

	scrollup = (ypos + lines) - ymax ;
	jump = (long)(ypos-scrollup) ; 

	if( (ypos + lines) > ymax + 1 )
	{
		mysprintf( b, "\x1b[%ld;%ldH",jump,(LONG)xpos) ;
		con_write( b, (LONG)strlen(b),g) ;
       
		        /* scroll */
		mysprintf(b,"\x1b[%ldS\0", (LONG)scrollup) ; 
		con_write( b, (LONG)strlen(b),g) ;
	}
}

/* end */
