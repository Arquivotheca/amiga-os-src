#include "ed.h"

#if AMIGA

extern LONG pcount;
extern struct MinList paste;
extern struct MinNode *pastenode;
extern int version;
extern struct Library *IFFParseBase;
extern UBYTE *copybuffer;

struct cmdMsg cm={0};

void StartPaste()
{
    struct MsgPort *pasteport, *cutport;
    struct MinNode *node;

  if(version >= 36) {
    pcount=0;
    pasteport=CreateMsgPort();
    cm.cm_Msg.mn_Node.ln_Type=0;
    cm.cm_Msg.mn_Length=(long)sizeof(struct cmdMsg);
    cm.cm_Msg.mn_ReplyPort=pasteport;
    cm.cm_type=TYPE_READ;

    Forbid();
    if(cutport=FindPort("ConClip.rendezvous")) {
	PutMsg(cutport,(struct Message *)&cm);
	Permit();
	WaitPort(pasteport);
	if(!cm.cm_error && cm.cm_type==TYPE_REPLY) {
	    /* pull paste node off the conclip list and add them to ours */
	    node=RemHead((struct List *)&cm.cm_Chunks);
	    while (node) {
		AddTail((struct List *)&paste,(struct Node *)node);
		node=RemHead((struct List *)&cm.cm_Chunks);	    
	    }
	    /* start the paste by connecting the first item to the port */
	    pastenode=(struct MinNode *)RemHead((struct List *)&paste);
	}
    }
    else Permit();
    DeleteMsgPort(pasteport);
  }
}

void KillPaste()
{
  if(version >= 36) {
    while(pastenode) {
	if( ((struct CHRSChunk *)pastenode)->freeme)
		FreeVec(((struct CHRSChunk *)pastenode)->freeme);
	FreeVec(pastenode);
	pastenode=(struct MinNode *)RemHead((struct List *)&paste);
    }
    pcount=0;
  }
}

UBYTE GetPaste()
{
    UBYTE ch=0;

  if(version >= 36) {
    ch=((struct CHRSChunk *)pastenode)->text[pcount++];
    if(pcount>((struct CHRSChunk *)pastenode)->size) {
	if( ((struct CHRSChunk *)pastenode)->freeme)
		FreeVec(((struct CHRSChunk *)pastenode)->freeme);
	FreeVec(pastenode);
	pastenode=(struct MinNode *)RemHead((struct List *)&paste);
	pcount=0;
    }
  }
    return(ch);
}

int CopyClip(f,n)
	int f,n;
{
    struct IFFHandle	*iff = NULL;

    long error=TRUE;
    LINE	*linep;
    int	loffs;
    int	i,s;
    REGION	region;

	/* if nothing to copy, just return */
	if ((s=getregion(&region)) != TRUE)return (s);

	/* only copy on V36 or above systems ? */
/*	if(version < 36)return(error); */
	if (!(IFFParseBase = OpenLibrary ("iffparse.library", 0L)))goto bye;
	 /* Allocate IFF_File structure. */
	if (!(iff = AllocIFF ()))goto bye;

	 /* Set up IFF_File for Clipboard I/O. */
	if (!(iff->iff_Stream = (ULONG) OpenClipboard(0)))goto bye;
	InitIFFasClip(iff);

	 /* Start the IFF transaction. */
	if (error = OpenIFF (iff, IFFF_WRITE))goto bye;

	/*
	 * Write our text to the clipboard as CHRS chunk in FORM FTXT
	 *
	 * First, write the FORM ID (FTXT)
	 */
    	if(!(error=PushChunk(iff, ID_FTXT, ID_FORM, IFFSIZE_UNKNOWN))) {
		/* Now the CHRS chunk ID followed by the chunk data
		 * We'll just write one CHRS chunk.
		 * You could write more chunks.
		 */
    		if(!(error=PushChunk(iff, 0, ID_CHRS, IFFSIZE_UNKNOWN))) {
		    linep = region.r_linep;	 /* Current line. */
		    loffs = region.r_offset; /* Current offset.	*/
		    i=0;
		    /* clear the copy buffer */
		    memset(copybuffer,0,2047);
		    while ((i < 2047) && region.r_size--) {
			if (loffs == llength(linep)) {/* End of line.*/
			    copybuffer[i++]='\n';
			    linep = lforw(linep);
			    loffs = 0;
			}
			else {	/* Middle of line.	*/
			    copybuffer[i++]=lgetc(linep,loffs);
			    ++loffs;
			}
		    }
		    copybuffer[i]=0;
		    /* Now the actual data (the text) */
		    if(WriteChunkBytes(iff, copybuffer, i) != i) {
		 	error = IFFERR_WRITE;
		    }
		    if(!error) error = PopChunk(iff);
		}
	}
	if(!error) error = PopChunk(iff);
	if(!error)mlwrite("copied to clipboard");
#if 0
#define RBUFSZ 512
    struct ContextNode  *cn;
    UBYTE readbuf[RBUFSZ];
    LONG rlen;

	if(error)goto bye;
	/*
	 * Close the write handle, then close the clipboard
	 */

	CloseIFF(iff);
	if (iff->iff_Stream) CloseClipboard ((struct ClipboardHandle *)
						iff->iff_Stream);
	/* now lets read it back */
	if (!(iff->iff_Stream = (ULONG) OpenClipboard(0))) {
		puts ("Reopen of Clipboard failed.");
		goto bye;
	}
	else printf("Reopened clipboard unit %ld\n",0);

	if (error = OpenIFF (iff, IFFF_READ)) {
		puts ("OpenIFF for read failed.");
		goto bye;
	}

	/* Tell iffparse we want to stop on FTXT CHRS chunks */
	if (error = StopChunk(iff, ID_FTXT, ID_CHRS)) {
		puts ("StopChunk failed.");
		goto bye;
	}

	/* Find all of the FTXT CHRS chunks */
	while(1) {
		error = ParseIFF(iff,IFFPARSE_SCAN);
		if(error == IFFERR_EOC) continue;	/* enter next context */
		else if(error) break;

		/* We only asked to stop at FTXT CHRS chunks
		 * If no error we've hit a stop chunk
		 * Read the CHRS chunk data
		 */
		cn = CurrentChunk(iff);

		if((cn)&&(cn->cn_Type == ID_FTXT)&&(cn->cn_ID == ID_CHRS)) {
			printf("CHRS chunk contains:\n");
			while((rlen = ReadChunkBytes(iff,readbuf,RBUFSZ)) > 0){
				Write(Output(),readbuf,rlen);
			}
			if(rlen < 0)	error = rlen;	
		}
	}

	if((error)&&(error != IFFERR_EOF)) {
		printf ("IFF read failed, error %ld: %s\n",
			error, errormsgs[-error - 1]);
		}
#endif

bye:
	/*
	 * Terminate the IFF transaction with the stream.  Free
	 * all associated structures.
	 */
	if(iff) {
	    CloseIFF(iff);

	    /*
	    * Close the clipboard stream
	    */
	    if (iff->iff_Stream)CloseClipboard ((struct ClipboardHandle *)
		iff->iff_Stream);
	    /* Free the IFF_File structure itself. */
	    FreeIFF(iff);
	}
	if(IFFParseBase)CloseLibrary (IFFParseBase);

	return(error);
}

#if 0

int PutPaste(action,string,len)
int action;
UBYTE *string;
LONG len;
{
    struct CHRSChunk *chrs;

    if(!len)return(-1);	/* nothing to do */
    chrs=(struct CHRSChunk *)AllocVec(len+1+sizeof(struct CHRSChunk),
		MEMF_PUBLIC|MEMF_CLEAR);
    if(chrs) {
	memcpy(chrs->data,string,len);
	chrs->size = len;
	chrs->text=&chrs->data[0];
	chrs->freeme=0;
	if(action == ACTION_QUEUE) {
	    AddTail((struct List *)&paste,(struct Node *)&chrs->mn);
	}
	else {
	    AddHead((struct List *)&paste,(struct Node *)&chrs->mn);

	}
	/* start things off unless there's a paste in progress */
	if((action == ACTION_FORCE) && (!pastenode)) {
	    pastenode=(struct MinNode *)RemHead((struct List *)&paste);
	    pcount=0;
	}
	return(len);
    }
    return(-1);
}
#endif
#endif
