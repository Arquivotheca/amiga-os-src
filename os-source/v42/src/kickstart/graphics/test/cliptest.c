#include <string.h>
#include <intuition/intuition.h>
#include <utility/tagitem.h>

char buffer[500];
struct Window *win;
#define IDCMPFLAGS (IDCMP_CLOSEWINDOW)
int GfxBase,IntuitionBase;

main(int argc, char **argv)
{
        struct IntuiMessage *msg;
        int x, y, mx, my;
        long class;
        struct RastPort *rp;

        int quit = 0;
        IntuitionBase = OpenLibrary("intuition.library", 0);
        GfxBase = OpenLibrary("graphics.library", 0);

        win = OpenWindowTags(0, WA_Width, 600, WA_Height, 200, WA_Title, "CrashMe",
                                                        WA_DragBar, 1, WA_CloseGadget, 1, WA_RMBTrap, 1,
                                                        WA_AutoAdjust, 1, WA_IDCMP, IDCMPFLAGS,
                                                        WA_SmartRefresh, 1, TAG_DONE);

        if (win == NULL) goto done;
        x = y = 0;
        rp = win->RPort;
        mx = win->BorderLeft + rp->TxWidth;
        my = win->BorderTop + rp->TxBaseline + rp->TxHeight;
        SetAPen(rp, 1);
        SetDrMd(rp, JAM2);
        do {
                while (msg = (struct IntuiMessage *)GetMsg(win->UserPort)) {
                        class = msg->Class;
                        ReplyMsg(msg);
                        if (class == IDCMP_CLOSEWINDOW) quit = 1;
                }
                if ( (x & 63) == 0) {
                        sprintf(buffer, "X=%ld, y=%ld", x, y);
                        Move(rp, mx , my );
                        Text(rp, buffer, strlen(buffer));
                }
                Move(rp, -50,-100);
                Draw(rp, x++, y++);
                //if (!quit) WaitPort(win->UserPort);
        } while (!quit);

done:
        if (win) CloseWindow(win);
        if (IntuitionBase) CloseLibrary(IntuitionBase);
        if (GfxBase) CloseLibrary(GfxBase);
}
