/* qume letterpro 20 special printer functions */
 
/****** printer.device/printers/Qume_LetterPro_20_special_functions *****
 *
 *   NAME
 *   Qume LetterPro 20 special functions implemented:
 * 
 ************************************************************************/

#include	"exec/types.h"
#include "../printer/printer.h"
#include "../printer/prtbase.h"

extern struct PrinterData *PD;

DoSpecial(command,outputBuffer,vline,currentVMI,crlfFlag,Parms)
   char outputBuffer[];
   UWORD *command;
   BYTE *vline;
   BYTE *currentVMI;
   BYTE *crlfFlag;
   UBYTE Parms[];
{
    int x=0;
    int y=0;
   BYTE temp;
   static char initMarg[]="\033\011L\0339\033\011q\0330\015";
   static char initThisPrinter[]="\033\037\015\033\036\011\033J\033M\033R\015";

if(*command==aRIN) {
   while(x<13){outputBuffer[x]=initThisPrinter[x];x++;}

   *currentVMI=36;
   if((PD->pd_Preferences.PrintSpacing)==EIGHT_LPI) { /* wrong again */
        outputBuffer[5]='\007';
        *currentVMI=27;
   }

   if((PD->pd_Preferences.PrintPitch)==ELITE)outputBuffer[2]='\013';
   if((PD->pd_Preferences.PrintPitch)==FINE)outputBuffer[2]='\011';

    Parms[0]=(PD->pd_Preferences.PrintLeftMargin);
    Parms[1]=(PD->pd_Preferences.PrintRightMargin);
    *command=aSLRM;
}

if(*command==aSLRM) {
    initMarg[2]=Parms[0]; 
	if (Parms[1] > 126) {
		Parms[1] = 126; /* limit right margin to 127 */
	}
    initMarg[7]=Parms[1]+1; 
    while(y<11)outputBuffer[x++]=initMarg[y++];
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

if(*command==aSLPP)
	{
	outputBuffer[x++]='\033';
	outputBuffer[x++]='F';
	outputBuffer[x++]=(Parms[0]/16)+'0';
	temp=Parms[0]%16;
	if(temp>9)outputBuffer[x++]=temp+'A'-10;
	   else outputBuffer[x++]=temp+'0';
	return(x);
	}

if(*command==aRIS) PD->pd_PWaitEnabled=253;

return(0);
}
