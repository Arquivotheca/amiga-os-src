#include <intuition/intuition.h>
#include <exec/alerts.h>
#include <exec/devices.h>
#include <exec/exec.h>
#include <exec/execbase.h>
#include <exec/io.h>
#include <exec/libraries.h>
#include <exec/lists.h>
#include <exec/memory.h>
#include <exec/nodes.h>
#include <exec/ports.h>
#include <exec/types.h>
#include <workbench/workbench.h>
#include <libraries/dosextens.h>
#include <intuition/intuitionbase.h>

#include <proto/all.h>

extern struct IntuitionBase *IntuitionBase;

static char Buf1[32];
static struct IntuiText WhoText = {1,0,JAM2, 20,20,NULL,NULL, NULL};
static struct IntuiText MsgText = {0,1,JAM2, 20,30,NULL,NULL, &WhoText};
static struct IntuiText OKText  = {0,1,JAM2, 5,3,NULL,"OK", NULL};

char *WhoAmI(void);
int CheckIntui(void);
void TellUser(char *Text);
void moveBSTR(BSTR bptr,char *buffer,int maxlen);

/*
 * routine to find the name of the process telling the user something
 */
char *WhoAmI(void)
{
   struct Process              *Process;
   struct CommandLineInterface *CLI;

   Process = FindTask(0);
   CLI = (struct CommandLineInterface *) BADDR(Process->pr_CLI);
   if( CLI )
   {
      moveBSTR(CLI->cli_CommandName, Buf1, 32);
      return(Buf1);
   } else {
      return(Process->pr_Task.tc_Node.ln_Name);
   }
}

/*
 * check that intuition is available
 */
int CheckIntui(void)
{
   if( !IntuitionBase ) 
   {  
      if( IntuitionBase = OpenLibrary("intuition.library",0) )
         return(FALSE);
      else
         Alert(AT_Recovery+AG_OpenLib+AO_Intuition,NULL);
      return(TRUE);
   }
   return(FALSE);
}

/*
 * issue a requester to tell the user something
 */
void TellUser(char *Text)
{
   int   Width,L1;

   if( CheckIntui() )  
      return;
   MsgText.IText = Text;
   WhoText.IText = WhoAmI(); 
   if((Width=IntuiTextLength(&MsgText))<(L1=IntuiTextLength(&WhoText)))
      Width = L1;
   AutoRequest(NULL,&MsgText,&OKText,&OKText,0L,0L,Width+70,80);
}

/*
 *  moveBSTR copies a BSTR to a C char string.
 */
void moveBSTR(BSTR bptr,char *buffer,int maxlen)
{
   register char *ptr;
   register unsigned int len, i;
   unsigned char l;

   ptr = (char *) BADDR(bptr);

   l = (unsigned int) (*ptr++); 

   if( !(len = l) )
   {
      *buffer = '\0'; 
      return;
   }
   if( len > maxlen )
      len = maxlen;
   for(i = 0;i < len;i++)
      *buffer++ = *ptr++;

   if(i < maxlen)
      *buffer = '\0'; 
   return;
}

