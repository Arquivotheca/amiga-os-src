/* misc routines for cursor moves, reverse chars, etc */

#include "exec/types.h"
#include "con-handler.h"

BYTE div[] = { 100,10,1 };

/*  Convert number n to string in v, length 3 */
VOID __regargs numtostring (gv, n, start)
struct Global *gv;
int n,start;
{
   int i,j;
   UBYTE *v = &(gv->printbuffer[start]);

   if(n<0)n = -n;

   for (i = 0; i < 3; i++) {
      j=SDivMod32(n,div[i]);
      n=getreg(1);		// remainder in D1
      v[i] = j + '0';
   }
}

/*  Add Cursor OFF/ON codes to string in v, at 0 and at start */
int __regargs addcursor (v, start)
UBYTE *v;
int start;
{
   v[0] = 0x9b;   /*  cursor off */
   v[1] = '0';
   v[2] = ' ';
   v[3] = 'p';
   v[start++] = 0x9b;   /*  cursor on */
   v[start++] = ' ';
   v[start++] = 'p';
   return(start);
}

VOID __regargs fixbuf (iobo, str)
struct IOStdReq *iobo;
UBYTE *str;
{
      iobo->io_Data = (APTR)str;
      iobo->io_Offset = 0;
      iobo->io_Command = CMD_WRITE;
      iobo->io_Length = -1;		// save the strlen
      DoIO((struct IORequest *)iobo);
}

int __regargs ReverseChar(gv,ch,count)
struct Global *gv;
UBYTE ch;
int count;
{
    /* if its a normal char, put it into the print array */
    UBYTE *pbuf= gv->printbuffer;

    if ((ch > 31) || (ch == 10))pbuf[count++] = ch;
    else { /* we have to reverse the character */
	/* check if we are we already in reverse mode */	
	if((count>3) && (!Strnicmp(&pbuf[count-4], "\x9B" "27m", 4))) {
	    pbuf[count-4] = ch+64; /* so keep us in reverse mode */
	    strcpy(&pbuf[count-3],"\x9B" "27m");
	    count++;
	}
	else {
	    strcpy(&pbuf[count],"\x9B" "7mX" "\x9B" "27m");
	    count += 8;
            pbuf[count - 5] = ch + 64;
	}
    }
return(count);
}
