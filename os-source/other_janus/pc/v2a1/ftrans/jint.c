/***************************************************************************
 *																								  	
 *	JINT.C	-	Support routines for INT 0B Janus calls
 *
 * Tim King March 1987                                          
 *
 * MODIFIED for MICROSOFT-C 01-25-88 by Bill Koester (CATS) West Chester
 *
 ***************************************************************************/
																						  				
#define LINT_ARGS	  								/* Enable type checking  	  		*/
#include "janus.h"
#include "jint.h"
#include <dos.h>
 
struct AmigaDiskReq *paramptr; 				/* Pointer to Amiga Disk Request	*/
														/* structure in shared memory		*/
 
char *buffer;         							/* Global buffer pointer			*/
 
 
/***************************************************************************
 *
 *	j_amiga(function)	-	Utility routine to call an Amiga function	
 *
 ***************************************************************************/
unsigned char j_amiga(function)
unsigned char function;
{
  union REGS r;
  r.h.ah = function;
  r.h.al = AMIGAFUN;
  r.x.bx = -1;
  int86(JANUS_INT, &r, &r );
  return(r.h.al);
}
 
/***************************************************************************
 *
 *	amigainit()			-	Initialize the Amiga interface. Returns nonzero
 *								on error.
 *
 ***************************************************************************/ 
unsigned char amigainit()																 		
{
  int rc, i;
  unsigned int j_param_seg, j_param_offset, j_buffer_seg;
  union REGS r;
  struct SREGS sr;
  unsigned long temp;
 
  /* This is the first item to be called. Therefore call it twice
     because the handler may not be installed first time (!) */

  for (i=0;i<2;i++)
  {
    r.h.ah = J_GET_BASE; 						/* Function code in ah 				*/
	 r.h.al = AMIGAFUN;							/* Janus service number in al 	*/
    int86(JANUS_INT, &r, &r);
    segread(&sr);
  }
 
  if (r.h.al != J_OK)
  {
     return(J_NO_SERVICE); 					/* Error no Amiga service			*/
  }

  j_param_seg = sr.es;
  j_param_offset = r.x.di;
  j_buffer_seg = r.x.dx;
 
  if (j_param_offset == -1)
  {
     return(J_NO_SERVICE);   					/* Error no Amiga service 			*/
  }

  /* Convert offset & segment into far pointer so we can use it 				*/
  /* WARNING - CAUTION - DANGER !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
  /* Braindead MICROSOFT-C implements far pointers as a segment/offset 		*/
  /* pair contained in a 32 bit long [segment ptr : offset ] so to get at	*/
  /* the real address F00A8 your far pointer must contain F00000A8 like the*/
  /* segment offset pair F000:00A8. It seems that MICROSOFT-C would rather	*/
  /* be efficient than compatible or correct!!! Arrgh.							*/

  paramptr = (struct AmigaDiskReq *)((unsigned long)
                                     (((unsigned long)j_param_seg) << 16) +
                                     (unsigned long)j_param_offset );

  buffer = (char *) ((unsigned long)
							((unsigned long)j_buffer_seg << 16) +
                     (unsigned long)(paramptr->adr_BufferOffset));
 
  /* Perform some initialisation */
  paramptr -> adr_Offset_h = 0;
  paramptr -> adr_Offset_l = 0;
  paramptr -> adr_Count_h  = 0;
 
  return (J_OK);
}
 
/***************************************************************************
 *
 * amiga(code,count)	-	Perform Amiga service return:
 *																	-1			error
 *																	0			EOF
 *																	actual 	otherwise
 *
 ***************************************************************************/ 
short amiga( code, count )													 	
unsigned short code, count;
{
  unsigned char rc;

  paramptr->adr_Fnctn = code;
  paramptr->adr_Count_h = 0;
  paramptr->adr_Count_l = count;

  rc = j_amiga(J_CALL_AMIGA);
  if (rc == J_NO_SERVICE) return(J_NO_SERVICE);/* Error - no service 		*/
														  		
  rc = j_amiga(J_WAIT_AMIGA);
/*
	printf("Error from J_WAIT = %d\n",rc);
*/
  if (rc != J_FINISHED) return( -1 );  	/* Error - not terminated 			*/

  rc = (unsigned char)(paramptr->adr_Err & 0xff);
  if (rc)
  {
/*
	printf("adr_Err = %d\n",rc);
*/
    return( -1 );        						/* Error on Amiga 					*/
  }

  if (code==ADR_FNCTN_READ || code==ADR_FNCTN_WRITE)
  {
    return( paramptr->adr_Count_l );
  }

  return(J_OK);
}
