/* * * * * * * * * * * * * * * * * * * *
 * (struct Window *) MakeWindow()
 *
 * init a new window structure using fixed parameter values.
 *
 * Inputs: none
 * Results: pointer to newly created window.
 * Errors: cannot open new window because 1) no memory to allocate
 *          required structures, 2) cannot open window
 *
 */

/* #include <standard.h>*/
#include <intuition/intuition.h>

struct Window *MakeWindow()
{
   struct NewWindow *nwp;
   struct Window
      *wp, *OpenWindow();

   if ((nwp = (struct NewWindow *)AllocMem(sizeof(struct NewWindow),0))==0){
      printf("\nError--Cannot allocate memory for new window structure!\n");
      return(NULL);
      }

   nwp->LeftEdge    = 0;
   nwp->TopEdge     = 0;
   nwp->Width       = 640;
   nwp->Height      = 200;
   nwp->DetailPen   = -1;
   nwp->BlockPen    = -1;
   nwp->Title       = "AMIGA Hard Disk Diagnostics";
   nwp->Flags       = WINDOWCLOSE|SMART_REFRESH|ACTIVATE|WINDOWSIZING|
		      WINDOWDEPTH|WINDOWDRAG|NOCAREREFRESH;
   nwp->IDCMPFlags  = CLOSEWINDOW;
   nwp->Type        = WBENCHSCREEN;
   nwp->FirstGadget = NULL;
   nwp->CheckMark   = NULL;
   nwp->Screen      = NULL;
   nwp->BitMap      = NULL;
   nwp->MinWidth    = 100;
   nwp->MinHeight   = 50;
   nwp->MaxWidth    = 32767;
   nwp->MaxHeight   = 32767;

   wp = OpenWindow(nwp);
   FreeMem(nwp, sizeof(struct NewWindow));
   return(wp);
}

