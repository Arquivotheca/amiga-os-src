#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/ports.h>
#include <exec/libraries.h>
#include <exec/devices.h>
#include <exec/io.h>
#include <exec/memory.h>
#include <libraries/dos.h>
#include <libraries/dosextens.h>
#include <libraries/filehandler.h>
#include "devices/narrator.h"
#include "handler.h"

#define TOUPPER(x) ((x) & 0x5f)

SayIt(sentence,vars)
UBYTE *sentence;
struct   VARS *vars;
{
int phonemeLength,senLen,i,error=0;

   if (!(senLen=strlen(sentence))) return(0);
   do {
    	if((vars->aMode)==FALSE) {
	    error=Translate(&sentence[-error],senLen,(vars->phoneme),MAXOSIZE);
 	}
	else {
	    strcpy(vars->phoneme,sentence);
            strcat(vars->phoneme,"\0");
	    error=0;
	}
        phonemeLength = strlen(vars->phoneme);
        vars->iow->message.io_Data = vars->phoneme;
        vars->iow->message.io_Length = phonemeLength;
	vars->iow->message.io_Message.mn_Node.ln_Type = 0;

        BeginIO(vars->iow);
        if (!(WaitIO (vars->iow)))GetMsg(vars->devport);
   } while (error != 0);
}

Say(str,length,vars)
UBYTE *str;
int length;
struct   VARS *vars;
{
UBYTE *cp,*sb;
int i=0,j;

cp= str;
  while (i < length) {
      j=0;
      sb = vars->buffer;
      while ((i < length) && (j<MAXSIZE)) {

	if((vars->delimit == TRUE) && ((*cp == 13) || (*cp == 10)))break;
	if(((*cp == '.') || (*cp == '?') || (*cp == '!'))&&(*(cp+1)<33))break;
	else if ((*cp < 32) || (*cp > 127)) {*sb++=' '; *cp++;}
	else *sb++ = *cp++;
	
	i++;
        j++;
      }

    if(*cp == '?')*sb++ = *cp;
    else *sb = '.';
    *sb = '\0';
    cp++;
    i++;

    if(!SetOpt(vars->buffer,vars->optionMode,vars))SayIt(vars->buffer,vars);
    if((((struct Task *)
	GetProcess(vars->clientport))->tc_SigRecvd)&&SIGBREAKF_CTRL_C)
	i=length+1;
  }
}


SetOpt(string,flag,vars)
char *string;
int flag;
struct   VARS *vars;
{
int v,i;
char *cp;

if((flag==FALSE) || (stricmp("opt/",string)))return(FALSE);
cp= &string[4];
while( *cp != '\0') {
    switch TOUPPER(*cp) {
         case 'P' :                    /* pitch 65-320 */
            v = atoi(++cp);
            if ( (v >= MINPITCH) && (v <= MAXPITCH)) vars->iow->pitch = v;
            break;

         case 'S' :                    /* rate 40-400  */
            v = atoi(++cp);
            if ( (v >= MINRATE) &&(v <= MAXRATE)) vars->iow->rate = v;
            break;

         case 'M' :
      	    vars->iow->sex = MALE;
            break;

         case 'F' :
      	    vars->iow->sex = FEMALE;
            break;

         case 'N' :
            vars->iow->mode = NATURALF0;
            break;

         case 'R' :
            vars->iow->mode = ROBOTICF0;
            break;

	 case 'O' :
	    if(*(cp+1) =='0')vars->optionMode=FALSE;
	    else vars->optionMode=TRUE;
	    break;

	 case 'A':
	    if(*(cp+1) == '0') vars->aMode=FALSE;
	    else vars->aMode=TRUE;
	    break;

	 case 'D' :
	    if(*(cp+1) == '0')vars->delimit=FALSE;
	    else vars->delimit=TRUE;
	    break;

         default:
   	   break;
	}
    cp++;
}
return(TRUE);
}
