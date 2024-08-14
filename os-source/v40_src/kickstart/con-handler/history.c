/* history functions for the con-handler */

#include "con-handler.h"

#ifdef HISTORY_ON

int __regargs InitHistory(gv,size)
struct Global *gv;
int size;
{
   UBYTE *point;
   int rc = 0;
   int tsize = size << 8; /* * 256 */

     if(size == 0)rc = ((gv->hsize+1) >> 8); /* / 256 */
     else {
	point=(UBYTE *)AllocMem(tsize,MEMF_PUBLIC);
	if(point) {
	    memset(point,'\n',tsize); /* clear the history buffer */
	    Forbid();	// Not needed!
	    rc=size;
	    if(gv->hbuffer) { /* free the old buffer */
		FreeMem(gv->hbuffer,gv->hsize+1);
	    }
	    gv->hbuffer=point;
	    gv->hsize= tsize-1;
  	    Permit();   // Not needed!
    	}
    }
    history=0;
    hscan=0;
    return(rc);
}


VOID __regargs SetHistory(gv,buffer,size,flag)
struct Global *gv;
UBYTE *buffer;
LONG size;
LONG flag;
{

    int i,last=0;
    UBYTE ch=1;
    if(flag) {
	history=0;
	memset(gv->hbuffer,'\n',gv->hsize+1); /* clear the history buffer */
    }
    for(i=0; (i<size && ch); i++) {
	ch=buffer[i];
	if((ch == '\n')||(!ch)) {
	    AddHistoryLine(gv,&buffer[last],i-last);
	    last=i+1;
	}
    }
    if(!history)hscan=0;
    else hscan=(history-1)&hmask;
}

int __regargs GetHistory(gv,buffer,len)
struct Global *gv;
UBYTE *buffer;
LONG len;
{
    LONG i;
    int scan=history;
    BOOL flag=0;

    /* find the earliest history */
    for(i=0; i<=hmask; i++) {
	if(gv->hbuffer[scan] != '\n') {
	    flag=TRUE;
	    break;
	}        
	scan  = (++scan)&hmask;
    }
    if(!flag)return(0);	/* history buffer is empty */
    
    for(i=0; ((i < len) && (scan != history)); i++) {
	if(buffer)buffer[i]=gv->hbuffer[scan];
	scan  = (++scan)&hmask;
    }
    return(i);    
}
#endif

VOID __regargs AddHistoryLine(gv,string,len)
struct Global *gv;
UBYTE *string;
int len;
{
    int pos = history;
    UBYTE c;
    int i;

    for(i=0; i<len; i++) {
	if((c=string[i]) == 10)break;
	if((c > 127)&&(c < 160))c=32;
	gv->hbuffer[pos]=c;
        pos = (++pos) & hmask;
    }

    if(i) {
        gv->hbuffer[pos] = 10;	/* this better be a linefeed */
        history = (pos+1) & hmask;
	gv->history_count++;
    	/* make sure its a clean line */
    	i=history;
    	while(i != pos) {
	    if(gv->hbuffer[i] == '\n')break;
	    gv->hbuffer[i]='\n';
	    i= (++i)&hmask;
	}
    }
}
