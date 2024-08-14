/* alphacom 101 special printer functions */
#include	"exec/types.h"
#include "../printer/printer.h"
#include "../printer/prtbase.h"

extern struct PrinterData *PD;

/****** printer.device/printers/Alphacom_Pro101_special_functions ********
 *
 *   NAME
 *   Alphacom AlphaPro 101 special functions implemented:
 * 
 ************************************************************************/

DoSpecial(command,outputBuffer,vline,currentVMI,crlfFlag,Parms)
   char outputBuffer[];
   UWORD *command;
   BYTE *vline;
   BYTE *currentVMI;
   BYTE *crlfFlag;
   UBYTE Parms[];
{
   int x=0;
   int i=0;
   int y=0;
   static char initThisPrinter[]="\033\037\014\033\036\011\015\033&\033R";

if(*command==aRIN) {
    while(x<13){outputBuffer[x]=initThisPrinter[x];x++;}
    *currentVMI=36;
    if((PD->pd_Preferences.PrintSpacing)==EIGHT_LPI) /* wrong again */
        {outputBuffer[5]='\007'; *currentVMI=27;}

   	if((PD->pd_Preferences.PrintPitch)==ELITE)outputBuffer[2]='\012';
   	if((PD->pd_Preferences.PrintPitch)==FINE)outputBuffer[2]='\010';

    Parms[0]=(PD->pd_Preferences.PrintLeftMargin);
    Parms[1]=(PD->pd_Preferences.PrintRightMargin);
    *command=aSLRM;
}

if(*command==aSLRM) {
    outputBuffer[x++]=27;
    outputBuffer[x++]=54;

    outputBuffer[x++]=13;

    for(i=0; i<80; i++)outputBuffer[x++]=8;
    if(Parms[0]>0)while(y<(Parms[0]-1)){outputBuffer[x++]=' ';y++;}
    outputBuffer[x++]=27;
    outputBuffer[x++]='9';

    while(y<Parms[1]){outputBuffer[x++]=' '; y++;}
    outputBuffer[x++]=27;
    outputBuffer[x++]='0';
    outputBuffer[x++]=13;

    outputBuffer[x++]=27;
    outputBuffer[x++]=39;
    outputBuffer[x++]=13;
    return(x);
}


if((*command==aSUS2)&&(*vline==0)) {*command=aPLU; *vline=1; return(0);}
if((*command==aSUS2)&&(*vline<0)) {*command=aRI; *vline=1; return(0);}
if((*command==aSUS1)&&(*vline>0)) {*command=aPLD; *vline=0; return(0);}

if((*command==aSUS4)&&(*vline==0)) {*command=aPLD; *vline=(-1); return(0);}
if((*command==aSUS4)&&(*vline>0)) {*command=aIND; *vline=(-1); return(0);}
if((*command==aSUS3)&&(*vline<0)) {*command=aPLU; *vline=0; return(0);}

if(*command==aSUS0)
	{
	if(*vline>0) *command=aPLD;
	if(*vline<0) *command=aPLU;
	*vline=0;
	return(0);
	}

if(*command==aPLU){(*vline)++; return(0);}

if(*command==aPLD){(*vline)--; return(0);}


if(*command==aSFC) {
	outputBuffer[x++]='\033';
	if(Parms[0]==31)outputBuffer[x++]='A';
	else outputBuffer[x++]='B';
	return(x);
}

if(*command==aCAM) {
    outputBuffer[x++]='\015';

    for(i=0; i<80; i++)outputBuffer[x++]=8;
    outputBuffer[x++]=27;
    outputBuffer[x++]='9';
        
outputBuffer[x++]=27;
outputBuffer[x++]=54;
 
for(i=0; i<80; i++)outputBuffer[x++]=' ';
outputBuffer[x++]=27;
outputBuffer[x++]='0';

outputBuffer[x++]=27;
outputBuffer[x++]=39;
outputBuffer[x++]=13;

return(x);
}

if(*command==aRIS) PD->pd_PWaitEnabled=253;

return(0);
}
