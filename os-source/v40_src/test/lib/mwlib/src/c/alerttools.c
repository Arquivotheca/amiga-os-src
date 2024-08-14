/*
    Copyright (c) 1989 Mitchell/Ware Systems, Inc.
    
    May be used by Commodore-Amiga, Inc. for in-house purposes only.
    No part of this program, or any parts or modifications thereof, either
    in source-code or object-code, or library form, may be used by 
    Commodore, or any of its employees, either indepently, or in conjunction
    with Commordore, as, or as a part of , any public-domain or commercial
    product, program, or any type of software, teaching aid, etc. without
    the prior written permission of Mitchell/Ware Systems.

    This copyright notice must not be removed from the document of which
    it is attached.    
*/
/*****************************************************************************
    AlertTools.c 
    
    Mitchell/Ware Systems           Version 1.00            9/15/88
    
    AlertTools consist of the following:
    
        void AlertInit()                - Ready system for new alert
        void AlertBuild(x, y, mess)     - Add this message
        BOOL AlertActivate(alertnum)    - Activate alert, return result

    "x", "y" - are the location that the next message will go. If they are
            zero, then AlertBuild will figure out where the next one goes.
    
    "alertnum" is the class of alert, DEADEND_ALERT or RECOVERY_ALERT.

*****************************************************************************/

#include <exec/types.h>
#include <exec/alerts.h>
#include <intuition/intuition.h>
#include <Tools.h>

static ALMESS *al_mess = NULL;
static int ax, ay, nl, al_len;

void AlertInit()    /* ready the system for an alert */
{
    if (al_mess)
        free(al_mess);
    al_mess = NULL;
    al_len = 0;
    ax = 10;
    ay = 10;
    nl = 10;
}
        
void AlertBuild(x, y, mess) /* Build the ALERT string */
int x, y;
char *mess;
{
    ALMESS *al;
    int len, wlen;
    
    /* defaults */
    if (x)
        ax = x;
    else
        x = ax;
    
    if (y)
    {
        if (y > ay - nl)
            ay = y + nl;
    }
    else 
        y = ay, ay += nl;
    
    if (mess)
    {
        al = calloc(1, wlen = sizeof(ALMESS) + (len = strlen(mess)) + 1);
        al->x = x;
        al->y = y;
        strcpy(al->mess, mess);
        if (al_mess)
        {
            al_mess = realloc(al_mess, al_len + wlen);
            memcpy((char *) al_mess + al_len, al, wlen);
            *((char *) al_mess + al_len - 1) = 1;    /* NOT-THE-END flag */            
            al_len += wlen;
        }
        else
        {
            al_len = wlen;
            al_mess = al;
        }
    }
}

BOOL AlertActivate(alertnum)
ULONG alertnum;
{
    BOOL ret;
    
    if (al_mess)
    {
        ret = DisplayAlert(alertnum, al_mess, ay);
    }
    else /* error! */
    {
        AlertInit();
        AlertBuild(NULL, NULL, "DEADEND_(MW/GURU)_ALERT");
        AlertBuild(NULL, NULL, NULL);
        AlertBuild(NULL, NULL, "AlertActivate() called out of sequence");
        AlertBuild(NULL, NULL, NULL);
        AlertBuild(NULL, NULL, "Hit EITHER MOUSE BUTTON to DIE!");
        AlertActivate(DEADEND_ALERT);
        AlertInit(); /* should never get here! */
    }
}
