/* TEST.C   -- Open a window in the workbench
 *  then try to attach, or use the console device to do io.
 */

#include <intuition/intuition.h>

ULONG	IntuitionBase	= 0;
ULONG	CDerror		= 0;
int	TimerError	= 0;

int	HdError	= !NULL;
struct	Window *Window;
extern	struct	MsgPort	consoleMsgPort;
extern	struct	MsgPort	timerMsgPort;
extern	struct	MsgPort	HdMsgPort;
extern	VOID cleanup();
ULONG    WakeUpBit, WaitMask;
char ostr[80];

VOID main()
{
   struct Window *MakeWindow();

   if ( (IntuitionBase = OpenLibrary("intuition.library",0)) < 1)
      cleanup();

/* Create the actual window using the following routine. */
   if ((Window=MakeWindow()) == NULL) {
      printf("\nCould not open window.\n");
      cleanup();
      }
   if ((CDerror = CDOpen(Window)) != NULL)
      cleanup();

   if ((TimerError = TimerOpen()) != NULL)
   {
      printf("\nTimer open failed\n");
      cleanup();
   }

   test();	/* run test program */
   cleanup();	/* Cleanly leave this AMIGA environment */
}

/*************************************
 *    cleanup: close up open things  *
 *************************************/

VOID cleanup()
{
      if (HdError == NULL) HdClose();
      if (TimerError == NULL) TimerClose();
      if (CDerror == NULL) CDClose();
      if (Window) CloseWindow(Window);
      if (IntuitionBase) CloseLibrary(IntuitionBase);
#ifdef DEBUG
      Debug(0);
#endif
      exit(0);
}
