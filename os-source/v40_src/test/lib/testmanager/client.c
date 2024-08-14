; /* example client code - execute to compile.
lc -cs -t -j73 client.c
blink from lib:c.o client.o to client lib lib:lc.lib ErrReport.lib lib:amiga.lib nd sc sd define __main=__tinymain
quit
*/

#include <exec/types.h>
#include <dos/dos.h>
#include <clib/dos_protos.h>

#include "tmgr_common.h"
#include "ErrRpt_protos.h"

struct errReport *er;
SHORT rc;
void GoodBye(void);

#define PAUSETIME 5 * TICKS_PER_SECOND

void main(int argc, char *argv[]) {

    rc = RETURN_FAIL;
    er = NULL;
    if (er = initTstData("TestClient", NULL)) {
        PutStr("LVL_PROG test message\n");
        if (RPT_OK != errRpt(er, LVL_PROG, PROB_NONE, RETURN_OK, "Got going\n", NULL))
            GoodBye();
        Delay(PAUSETIME);
        PutStr("LVL_VB1 test message\n");
        if (RPT_OK != errRpt(er, LVL_VB1, PROB_NONE, RETURN_OK, "I'm verbose\n", NULL))
            GoodBye();
        Delay(PAUSETIME);
        PutStr("LVL_VB2 test message\n");
        if (RPT_OK != errRpt(er, LVL_VB2, PROB_NONE, RETURN_OK, "I'm verbose-2\n", NULL))
            GoodBye();
        Delay(PAUSETIME);
        PutStr("LVL_VB3 test message\n");
        if (RPT_OK != errRpt(er, LVL_VB3, PROB_NONE, RETURN_OK, "I'm verbose-3\n", NULL))
            GoodBye();
        Delay(PAUSETIME);
        PutStr("LVL_DB1 message\n");
        if (RPT_OK != errRpt(er, LVL_DB1, PROB_NONE, RETURN_OK, "I'm debugged\n", NULL))
            GoodBye();
        Delay(PAUSETIME);
        PutStr("LVL_DB2 message\n");
        if (RPT_OK != errRpt(er, LVL_DB2, PROB_NONE, RETURN_OK, "I'm debugged-2\n", NULL))
            GoodBye();
        Delay(PAUSETIME);
        PutStr("LVL_DB3 message\n");
        if (RPT_OK != errRpt(er, LVL_DB3, PROB_NONE, RETURN_OK, "I'm debugged-3\n", NULL))
            GoodBye();
        Delay(PAUSETIME);
        PutStr("LVL_ERR message\n");
        if (RPT_OK != errRpt(er, LVL_ERR, PROB_IOERR, 103, "I'm error 103 %s %s\n", (LONG)"and no", "more info"))
            GoodBye();
        Delay(PAUSETIME);
        PutStr("LVL_PROG test message\n");
        if (RPT_OK != errRpt(er, LVL_PROG, PROB_NONE, RETURN_OK, "In between Errors (L2 approacheth)\n", NULL))
            GoodBye();
        Delay(PAUSETIME);
        PutStr("LVL_ERR2 message\n");
        if (RPT_OK != errRpt(er, LVL_ERR2, PROB_CUSTOM, 20, "I'm a custom error code (%ld)\n", 20))
            GoodBye();
        Delay(PAUSETIME);
        PutStr("LVL_PROG test message\n");
        if (RPT_OK != errRpt(er, LVL_PROG, PROB_NONE, RETURN_OK, "Farewell, made it to Test's End\n", NULL))
            GoodBye();

        PutStr("That was the last test, have a nice day!\n");

        rc = RETURN_OK;
    }
    else {
        PutStr("initTstData() FAILED!!\n");
    }

    GoodBye();
}

void GoodBye(void) {
    if (er)
        freeTstData(er);
    exit(rc);
}
