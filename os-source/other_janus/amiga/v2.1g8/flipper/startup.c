#include <exec/types.h>
#include <exec/exec.h>
#include <intuition/intuition.h>
#include <workbench/icon.h>
#include <workbench/workbench.h>
#include <workbench/startup.h>
#include <proto/exec.h>
#include <proto/intuition.h>
/* #include <local/sccs.h> */

#define STARTUP 1

#include "string.h"
#include "share.h"

extern char amiga_df0[], amiga_df1[];
extern char pc_a[], pc_b[];
extern int window_mode;

struct Library *IconBase;

int window_position(string)
char *string;
{
    int x, y;
    int temp;
    int xdigits, ydigits;

    Debug1("Window_position(%s)\n", string);
    x = 0;
    xdigits = 0;
    while(isdigit(*string)) {
        /* multiply by 10..... */
        temp = x;
        x <<= 2;
        x += temp;
        x <<= 1;
        x += (int) (*string - '0') ;
        string++;
        xdigits++;
    }
    while( (*string != '\0') && !isdigit(*string))
        string++;
    y = 0;
    ydigits = 0;
    while(isdigit(*string)) {
        temp = y;
        y <<= 2;
        y += temp;
        y <<= 1;
        y += (int) (*string - '0');
        string++;
        ydigits++;
    }
    Debug2("x: %ld, y: %ld\n", x, y);
    xposition = x;
    yposition = y;
    if((xdigits > 0) && ( ydigits > 0)) {
        window_mode = 1;
        return 1;
    }
    else
        return 0;
}


#ifdef WB_SUPPORT
int flipper_mode(string)
char *string;
{
    Debug1("flipper_mode(%s)\n", string);
    if(stricmp(string, "auto") == 0) {
        currentmode = MODE_AUTO;
    }
    else if(stricmp(string, "amiga") == 0) {
        currentmode = MODE_MANUAL;
        currentselect = SELECT_AMIGA;
    }
    else if(stricmp(string, "pc") == 0) {
        currentmode = MODE_MANUAL;
        currentselect = SELECT_PC;
    }
    else if(stricmp(string, "quit") == 0) {
        currentmode = MODE_QUIT;
    }
    else {
        Debug1("flipper_mode: don't grok %s\n", string);
        return 0;
    }
    return 1;
}

void HandleWB(wbargs)
struct WBStartup *wbargs;
{
    struct WBStartup *wbpar;

    wbpar = (struct WBStartup *) wbargs;
    Debug1("WBStartup: sm_NumArgs: %ld\n", wbargs->sm_NumArgs);
    IconBase = OpenLibrary(ICONNAME, 0);
    {
        int i;
        struct WBArg *thisarg;
        for(i = 1; i < wbpar->sm_NumArgs; i++) {
            thisarg = wbpar->sm_ArgList + i;
            Debug2("Argument %ld Lock: %ld\n", i, thisarg->wa_Lock);
            Debug1("Name: %s\n", thisarg->wa_Name);
            if(strcmp(thisarg->wa_Name, "")) {
                struct DiskObject * diskobj;
                char **toolarray;
                LONG olddir;
                if(thisarg->wa_Lock) {
                    olddir = CurrentDir(thisarg->wa_Lock);
                    diskobj = GetDiskObject(thisarg->wa_Name);
                    toolarray = diskobj->do_ToolTypes;
                    {
                        while(*toolarray) {
                            int i;
                            char *thistool;
                            
                            Debug1("Tool: %s\n", *toolarray);
                            thistool = *toolarray;
                            for(i = 0; ; i++) {
                                if(options[i].option == NULL) {
                                    Debug1("Don't grok %s\n", thistool);
                                    break;
                                }
                                Debug2("Check %s <-> %s\n", \
                                    thistool, options[i].option);
                                if(strncmp(thistool, options[i].option, \
                                    strlen(options[i].option)) == 0) {
                                    thistool += strlen(options[i].option);
                                    Debug2("handle %s(%s)\n", \
                                    options[i].option, thistool);
                                    options[i].handleoption(thistool);
                                    break;
                                }
                            }
                            toolarray++;
                        }
                    }
                    CurrentDir(olddir);
                }
            }
        }
    }
    CloseLibrary(IconBase);
}

#endif
