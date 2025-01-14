;/*
lc -d -j73 -O -oset.o -iV:include -iV:inc.lattice -v -csf set
blink set.o to set.ld LIB lib:amiga.lib lib:debug.lib lib:lc.lib sc sd nd
protect set.ld +p
quit
*/

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* |_o_o|\\ Copyright (c) 1989 The Software Distillery.  All Rights Reserved */
/* |. o.| || This program may not be distributed without the permission of   */
/* | .  | || the authors:                             BBS: (919) 382-8265    */
/* | o  | ||   Dave Baker      Alan Beale         Jim Cooper                 */
/* |  . |//    Jay Denebeim    Bruce Drake        Gordon Keener              */
/* ======      John Mainwaring Andy Mercier       Jack Rouse                 */
/*             John Toebes     Mary Ellen Toebes  Doug Walker                */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


/*-----------------------------------------------------------------------*/
/* Command: SetEnv                                                       */
/* Author:  John A. Toebes, VIII                                         */
/* Change History:                                                       */
/*  Date    Person        Action                                         */
/* -------  ------------- -----------------                              */
/* 19MAR89  John Toebes   Initial Creation                               */
/* 25OCT89  John Toebes   Revamp for DOS36.21                            */
/* 06NOV89  John Toebes   Change to use SetEnvVar                        */
/* 02DEC89  John Toebes   Final changes from review, more comments       */
/* Notes:                                                                */
/*-----------------------------------------------------------------------*/

#include <exec/types.h>
#include <exec/nodes.h>
#include <dos/dosextens.h>
#include <dos/dos.h>
#include <dos/var.h>


#include <string.h>
#include <clib/macros.h>

#define NO_EXEC_PRAGMAS
#include "internal/commands.h"
#include "global.h"

#ifdef ram
void __stdargs writef( struct DosLibrary *DOSBase, UBYTE *fmt, UBYTE *args );
#endif

#define TEMPLATE    "NAME,STRING/F"
#define OPT_NAME    0
#define OPT_STRING  1
#define OPT_COUNT   2

#define SMALL 1

int cmd_setenv(void)
{
   struct DosLibrary *DOSBase;
   struct Library *UtilityBase;
   int rc = RETURN_ERROR;
   char *str;
   long opts[OPT_COUNT];
   struct RDArgs *rdargs;
   struct LocalVar *lv;
   int i,len;
   UBYTE *buf;
   UBYTE buffer[256];
   char *template = TEMPLATE; /* template for set, setenv */
   long mode=0, remove=0;

#ifndef SMALL
#ifdef ram
   if ((DOSBase = (struct DosLibrary *)OpenLibrary(DOSLIB, DOSVER))) {
#else
      DOSBase = (struct DosLibrary *)OpenLibrary(DOSLIB, DOSVER);
#endif
      UtilityBase=OpenLibrary("utility.library",0);
#else
	/* just open the libraries, we can't readargs yet */
      getargs(&DOSBase,&UtilityBase,NULL,NULL,0);
#endif

      /* note: if set moves to disk, the 31 is inappropriate for a path */

      if(GetProgramName(buffer,31)) { /* set the template and pick the defaults */
	buf=FilePart(buffer);
	if(len=strlen(buf)) {
	    if(ToUpper(buf[0]) == 'U') {
	        remove = TRUE;
	    	template="NAME";
	    	if(len == 8)mode = GVF_GLOBAL_ONLY|LV_VAR; /* unsetenv */
	    	else if (len == 7) mode = GVF_LOCAL_ONLY|LV_ALIAS; /* unalias */
	    	else mode = GVF_LOCAL_ONLY|LV_VAR; /* unset */
	    }
	    else {
	    	if(len == 6)mode = GVF_GLOBAL_ONLY|LV_VAR; /* setenv */
	    	else if (len == 5) mode = GVF_LOCAL_ONLY|LV_ALIAS; /* alias */
	    	else mode = GVF_LOCAL_ONLY|LV_VAR; /* set */
	    }
	}
      }
      memset((char *)opts, 0, sizeof(opts));
      rdargs = ReadArgs(template, opts, NULL);
      if (rdargs == NULL) {
         /*-------------------------------------------------------------*/
         /* Something wrong with their arguments, tell them and quit    */
         /*-------------------------------------------------------------*/
         PrintFault(IoErr(), NULL);
      }
      else if ( opts[OPT_NAME] && strlen((char *)opts[OPT_NAME]) > 32) {
	PrintFault(120,NULL);
      }
      else {
         /*-------------------------------------------------------------*/
         /* Assume things will work out ok and try to set the variable  */
         /*-------------------------------------------------------------*/
         rc = RETURN_OK;
	 if(opts[OPT_NAME]) {

             if(( (str = (char *)opts[OPT_STRING]) != NULL)&& !remove){
		if(!(*str))str="";
		/* if alias is > 255, just use first 255 characters */
//		if((i=strlen((char *)opts[OPT_STRING]))>255)i=255;
		if(!SetVar((char *)opts[OPT_NAME], str,
			   strlen((char *)opts[OPT_STRING]), mode)) {
                /*----------------------------------------------------------*/
                /* It didn't work so tell them and give the proper RC       */
                /*----------------------------------------------------------*/
		Fault(STR_CANT_SET,NULL,buffer,36);
		writef(DOSBase,buffer,opts[OPT_NAME]);
                rc = RETURN_ERROR;
		}
	     }
	     else if (remove) {
		/* should we use the result of DeleteVar ? */
		DeleteVar((char *)opts[OPT_NAME],mode);
		}
	     else if(!Stricmp(buf,"alias")) { /* give alias value */
		if( (GetVar( (char *)opts[OPT_NAME],buffer,255,mode) ) >= 0) {
		    writef(DOSBase,"%s\n",buffer);
		}
		else {
		    rc=RETURN_WARN;
		    SetIoErr(ERROR_OBJECT_NOT_FOUND);
		}
	     }
	 }
	 else {	/* print out the alias/variable list */
	   if(mode&GVF_GLOBAL_ONLY) {
		System("LIST env: QUICK NOHEAD FILES",NULL);	
	   }
	   else {
	      for(lv=(struct LocalVar *) (THISPROC)->pr_LocalVars.mlh_Head;
		 	lv->lv_Node.ln_Succ;
			lv=(struct LocalVar *)lv->lv_Node.ln_Succ) {
			     if(CheckSignal(SIGBREAKF_CTRL_C)) {
	    			PrintFault(304,0);
	    			break;
			     }
			    if((lv->lv_Flags & GVF_BINARY_VAR)) {
			        strcpy(buffer,"[BINARY]");
			        len=8;
			    }
			    else {
			        len=MIN(lv->lv_Len,255);
			        memcpy(buffer,lv->lv_Value,len);
			        buffer[len]=0;
			    }
			    for(i=0; i < len; i++) {
			        if((buffer[i] == 0x1b) || (buffer[i] == 0x9b))
				  buffer[i]=127; /* no escape display in alias */
			    }
			    if((mode&255) == (lv->lv_Node.ln_Type)&255) 
			        writef(DOSBase,"%TH %s\n",
				    lv->lv_Node.ln_Name,buffer);
			}
	   }
	 }
         /*-------------------------------------------------------------*/
         /* We are done so cleanup and exit                             */
         /*-------------------------------------------------------------*/
         FreeArgs(rdargs);
      }
   CloseLibrary(UtilityBase);
   CloseLibrary((struct Library *)DOSBase);
#ifdef ram
   }
   else { OPENFAIL }
#endif
   return(rc);
 }

#ifdef ram
void __stdargs writef( DOSBase, fmt, args )
struct DosLibrary DOSBase; 
UBYTE *fmt, *args;
{
   VFWritef( Output(), fmt, (LONG *)&args );
}
#endif
