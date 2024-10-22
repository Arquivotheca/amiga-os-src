;/*
lc -O -M -d -j73 -oresident.o -iV:include -iV:inc.lattice -v -csf resident
blink resident.o to resident.ld LIB LIB:amiga.lib lib:debug.lib sc sd nd
protect resident.ld +p
quit
*/

#define NO_EXEC_PRAGMAS
#include "internal/commands.h"
#include "global.h"
#include "libhdr.h"
#include "clib/macros.h"

#define SMALL 1

#define TEMPLATE    "NAME,FILE,REMOVE/S,ADD/S,REPLACE/S,PURE=FORCE/S,SYSTEM/S"
#define OPT_NAME    	0
#define OPT_FILE  	1
#define OPT_REMOVE	2
#define OPT_ADD		3
#define OPT_REPLACE	4
#define OPT_PURE	5
#define OPT_SYSTEM	6
#define OPT_COUNT	7

int cmd_resident(void)
{
   struct DosLibrary *DOSBase;
   struct Library *UtilityBase;
   int rc=RETURN_ERROR;
   LONG res = 0;
   long opts[OPT_COUNT];
   struct RDArgs *rdargs;
   UBYTE namespace[256];
   UBYTE *name=namespace;	/* initialize just in case */
   UBYTE *file,*fi;
   struct Segment *oldseg;
   struct Segment *list;
   int count=1;
   BPTR code = 0;
   int pc;
   char buffer[48],buf2[8];
   unsigned int len;

#ifndef SMALL
  DOSBase = (struct DosLibrary *)OpenLibrary(DOSLIB, DOSVER);
  memset((char *)opts, 0, sizeof(opts));
  rdargs = ReadArgs(TEMPLATE, opts, NULL);
  if (rdargs == NULL) {
	PrintFault(IoErr(),NULL);
  }
  else {
#else
  if(rdargs=getargs(&DOSBase,&UtilityBase,TEMPLATE,(LONG **)&opts,sizeof(opts))) {
#endif
      memset((char *)namespace, 0, 255);
      rc = RETURN_OK;
	   if ((!opts[OPT_FILE]) && (opts[OPT_NAME])) { /* no file name ? */
	      file = (UBYTE *)opts[OPT_NAME]; /* make up one */
	      fi=FilePart(file);
	      len = strlen(fi);
	      memcpy(namespace,fi,MIN(255,len));
	   }
	   else if ((opts[OPT_FILE]) && (!opts[OPT_NAME])) { /* no name ? */
	      file = (UBYTE *)opts[OPT_FILE]; /* make up one */
	      fi=FilePart(file);
	      len = strlen(fi);
	      memcpy(namespace,fi,MIN(255,len));
	   }
	   else {
	      name = (UBYTE *)opts[OPT_NAME];
	      file = (UBYTE *)opts[OPT_FILE];
	   }

	   if (name) {
	    /* we have the name, now do the action */

	    oldseg = searchsegs(DOSBase,name,0);
	    if ( opts[OPT_REMOVE]) {
		if(oldseg) {
		    if(oldseg->seg_UC <= CMD_BUILTIN) { /* if internal, disable the command */
			oldseg->seg_UC = CMD_DISABLED;
		    }
		    else if(oldseg->seg_UC < 0) { /* internal segment */
			rc = RETURN_WARN; /* it might not have been internal before */
			res=202;
		    }
		    else {
			if(!RemSegment(oldseg)) {
			    rc = RETURN_WARN; /* should be error */
			    res=IoErr();
			}
		    }
		}
	        else {
		    res=205;
		    rc = RETURN_WARN;
		 }
	    }
   /* we add if we get an add or an attempt on a builtin or system */
   else if (file && (opts[OPT_ADD] || 
		(oldseg->seg_UC < 0) && (oldseg->seg_UC > CMD_DISABLED)) ||
		((!opts[OPT_REPLACE]) && oldseg->seg_UC > 1)) {
           if((pc=testpure(DOSBase,file,opts[OPT_PURE]))>0)code=LoadSeg(file);
   	   if(!code) {
		res=IoErr();
		if(!pc)res=212;  /* object not pure */
		else if(!res)res=205; /* object not found */
		rc =RETURN_WARN;
	   }
	   else {
	        if(!Stricmp(name,"resident") || !Stricmp(name,"CLI")) {
		    rc=1;
    	 	    res = 212;
		}
		else {
		    if(opts[OPT_SYSTEM])count = -1;
	            if(!AddSegment(name, code, count)) {
           		UnLoadSeg(code);
			res=IoErr();
			rc = RETURN_ERROR;
		   }
		}
	   }
   }
   /* we are trying a replace */

    else if (file) {
	    if(opts[OPT_REPLACE]) {
      	        if(!oldseg) {	/* they only want a replace */
		    res=205;  /* object not found */
	 	    rc=RETURN_WARN;
		}
                else if (oldseg->seg_UC > 1) {
		    res=202;  /* object in use */
	            rc= RETURN_ERROR;
	        }
                else if (oldseg->seg_UC <= CMD_BUILTIN) {
		    oldseg->seg_UC = CMD_BUILTIN;
		    goto errorExit;
		}
                else if (oldseg->seg_UC < 0) { /* is it an internal ? */
		    res=222; /* object protected from deletion */
		    rc = RETURN_ERROR;
		}
	    }
	    if (rc==RETURN_OK) {
                if((pc=testpure(DOSBase,file,opts[OPT_PURE]))>0)code=LoadSeg(file);
            	    if(!code) {
			if(pc==0)res=212; /* object not pure */
			else res=205; /* object not found */
	                rc= RETURN_WARN;
		    }
		    else {
		        UnLoadSeg(oldseg->seg_Seg);	/* do the replace */
            	        oldseg->seg_UC = opts[OPT_SYSTEM] ? -1 : 1;
	    	        oldseg->seg_Seg = code;
		    }
		}
    }
	   }
   else {	/* might as well do a list */
    if(Fault(STR_TH_USE_COUNT,NULL,buffer,44)&&
    	Fault(STR_NAME,NULL,buf2,8))writef(DOSBase,buffer,buf2);
    Forbid();
    list=(struct Segment *)BADDR(((struct DosInfo *)BADDR(((struct RootNode *)
			(DOSBase->dl_Root))->rn_Info))->di_NetHand);
    while (list) {
     if(CheckSignal(SIGBREAKF_CTRL_C)) {
	    PrintFault(304,0);
	    break;
     }
     memcpy(namespace,list->seg_Name+1,(UBYTE)list->seg_Name[0]);
     namespace[(UBYTE)list->seg_Name[0]]=0;

     if(list->seg_UC==CMD_DISABLED)Fault(STR_TH_DISABLED,NULL,buffer,44);
     else if(list->seg_UC<=CMD_BUILTIN)Fault(STR_TH_INTERNAL,NULL,buffer,44);
     else if(list->seg_UC >= 0)strcpy(buffer,"%TH %I4\n");
     else Fault(STR_TH_SYSTEM,NULL,buffer,44); /* must be system */
     if((list->seg_UC != -1) || opts[OPT_SYSTEM])
	     writef(DOSBase,buffer,namespace,list->seg_UC-1);
     list = (struct Segment *)BADDR(list->seg_Next);
    }
    Permit();
   }
errorExit:
  FreeArgs(rdargs);
    if(rc) {
        if(rc > 1)PrintFault(res,NULL);
        SetIoErr(res);
    }
  }
  CloseLibrary(UtilityBase);
  CloseLibrary((struct Library *)DOSBase);
#ifdef ram
}
else {OPENFAIL;}
#endif
return(rc);
}

int testpure(DOSBase,file,force)
struct DosLibrary *DOSBase;
UBYTE *file;
LONG force;
{
   struct FileInfoBlock __aligned exinfo;
   LONG res = -1;
   BPTR lock;

       if(lock=Lock(file,ACCESS_READ)) {
           if(res = Examine(lock,&exinfo)) {
	       if (exinfo.fib_DirEntryType > 0)res = 0; /* no directories */
	       else if (force || (exinfo.fib_Protection & FIBF_PURE))res=TRUE;
               else res =  0;	/* plain file */
               UnLock(lock);
	   }
       }
       return(res); /* no file found */
}
