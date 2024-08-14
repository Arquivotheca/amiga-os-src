/*
    asl.library Font Requester Test
    Written by:
        John J. Szucs
        Amiga Systems Section
        Product Assurance Department
        Commodore International Services Company
    Copyright © 1992 Commodore International Services Company

Template:
    EARLY/S,W=WINDOW/S,S=SCREEN/S,PSN=PUBSCREENNAME/K,PI=PRIVATEIDCMP/S,IMF=INTUIMSGFUNC/S,SW=SLEEPWINDOW/S,UD=USERDATA/S,TF=TYPEFACE/K,TS=TYPESIZE/N/K,B=BOLD/S,U=UNDERLINED/S,I=ITALIC/S,L=LOCALE/K,TT=TITLETEXT/K,PT=POSITIVETEXT/K,NT=NEGATIVETEXT/K,IL=INITIALLEFT/N/K,IT=INITIALTOP/N/K,IW=INITIALWIDTH/N/K,IH=INITIALHEIGHT/N/K,IN=INITIALNAME/K,IS=INITIALSIZE/N/K,IB=INITIALBOLD/S,IU=INITIALUNDERLINED/S,II=INITIALITALIC/S,IF=INITIALFLAGS/N/K,IFP=INITIALFRONTPEN/N/K,IBP=INITIALBACKPEN/N/K,IJ1=INITIALJAM1/S,IJ2=INITIALJAM2/S,ICOMP=INITIALCOMPLEMENT/S,IIV=INITIALINVERSVID/S,F=FLAGS/N/K,DFP=DOFRONTPEN/S,DBP=DOBACKPEN/S,DS=DOSTYLE/S,DDM=DODRAWMODE/S,FWO=FIXEDWIDTHONLY/S,MINH=MINHEIGHT/N/K,MAXH=MAXHEIGHT/N/K,FF=FILTERFUNC/K,HF=HOOKFUNC/S,ML=MODELIST/S

Usage:
    FontRequester [EARLY] [WINDOW] [SCREEN] [PUBSCREENNAME <name>]
        [PRIVATEIDCMP] [INTUIMSGFUNC] [SLEEPWINDOW] [USERDATA]
        [TYPEFACE <type face>] [TYPESIZE <type size>] [BOLD] [UNDERLINED] [ITALIC]
        [LOCALE <locale>] [TITLETEXT <title text>] [POSITIVETEXT <positive text>] [NEGATIVETEXT <negative text>]
        [INITIALLEFT <left>] [INITIALTOP <top>] [INITIALWIDTH <width>] [INITIALHEIGHT <height>]
        [INITIALNAME <name>] [NITIALSIZE <size>] [INITIALBOLD] [INITIALUNDERLINED] [INITIALITALIC] [INITIALFLAGS <flags>]
        [INITIALFRONTPEN <pen>] [INITIALBACKPEN <pen>] [INITIALJAM1] [INITIALJAM2] [INITIALCOMPLEMENT] [INITIALINVERSVID]
        [FLAGS <flags>] [DOFRONTPEN] [DOBACKPEN] [DOSTYLE] [DODRAWMODE]
        [FIXEDWIDTHONLY], [MINHEIGHT] [MAXHEIGHT] [FILTERFUNC] [HOOKFUNC] [MODELIST]
    EARLY           -   Use tags in AllocAslRequest(); if not specified,
                        tags are used in AslRequest()
    WINDOW          -   Open window
    SCREEN          -   Open screen
    PUBSCREENNAME   -   Create and open on public screen with name <name>
    PRIVATEIDCMP    -   Allocate private IDCMP
    INTUIMSGFUNC    -   Test IntuiMessage function hook;
                        outputs hook, object, and message to debugging termal;
                        counts hook hits, bad hooks, bad objects, and bad messages
    SLEEPWINDOW     -   Sleep parent window
    USERDATA        -   Test user data (set to $C0EDBABE)
    TYPEFACE        -   Set type face to <type face>
    TYPESIZE        -   Set type size to <type size>
    BOLD            -   Bold type
    UNDERLINED      -   Underlined type
    ITALIC          -   Italic type
    LOCALE <locale> -   Use locale <locale>
    TITLETEXT <title text> - Use <title text> for title
    POSITIVETEXT <positive text> - Use <positive text> for positive text (default "OK")
    NEGATIVETEXT <negative text> - Use <negative text> for negative text (default "Cancel")
    INITIALLEFT <left>     - Initial left edge of <left>
    INITIALTOP <top>       -  Initial top edge of <top>
    INITIALWIDTH <width>   - Initial width of <width>
    INITIALHEIGHT <height> - Initial height of <height>
    INITIALNAME <name>  -   Initial typeface name
    INITIALSIZE <size>  -   Initial type size
    INITIALBOLD         -   Initial type bold
    INITIALUNDERLINED   -   Initial type underlined
    INITIALITALIC       -   Initial type italic
    INITIALFLAGS <flags>    -   Initial type flags; see graphics/text.h for definitions
    INITIALFRONTPEN <pen>   -   Initial front pen
    INITIALBACKPEN <pen>    -   Initial back pen
    INITIALJAM1         -   Initial JAM1 drawing mode
    INITIALJAM2         -   Initial JAM2 drawing mode
    INITIALCOMPLEMENT   -   Initial COMPLEMENT drawing mode
    INITIALINVERSVID    -   Initial INVERSVID drawing mode
    FLAGS <flags>           -   Decimal value for ASLFO_Flags; see libraries/asl.h for definitions
    DOFRONTPEN          -   Include front pen selection in font requester
    DOBACKPEN           -   Include back pen selection in font requester
    DOSTYLE             -   Include style selection in font requester
    DODRAWMODE          -   Include drawing mode selection in font requester
    FIXEDWIDTHONLY      -   Only select from fixed-width fonts
    MINHEIGHT           -   Minimum height of font to select
    MAXHEIGHT           -   Maximum height of font to select
    FILTERFUNC <pattern> -   Test filter function; only accepts fonts with name
                            matching <pattern>
    HOOKFUNC            -   Test hook function
    MODELIST            -   Test custom drawing mode list

Notes:
    The long series of:

        if (args[CLI_...]) {
            appendTagItem(tagArray,...,...);
        }

    statements may be replaced with a more elaborate table-driven structure.
    This is not possible under the time constraints.

*/

/*
 * System includes
 */

#include <exec/types.h>
#include <exec/memory.h>

#include <utility/tagitem.h>

#include <dos/dos.h>

#include <graphics/text.h>
#include <graphics/displayinfo.h>

#include <intuition/intuition.h>

#include <libraries/asl.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>

#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/diskfont_protos.h>

#include <clib/locale_protos.h>
#include <clib/asl_protos.h>

/*
 * ANSI includes
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

/*
 * Constants
 */

/* System */
#define KICKSTART_VERSION 37 /* Version of Amiga Operating System Kickstart */
#define WORKBENCH_VERSION 38 /* Version of Amiga Operating System Workbench */

/* Application */
#define APP_NAME "FontRequester" /* Name of application */
#define APP_USER_DATA 0xC0EDBABE /* User data */

/* Text attribute */
#define TEXTATTR_NAME "topaz.font" /* Typeface name */
#define TEXTATTR_SIZE 8 /* Type size */

#define TYPEFACE_LENGTH 64 /* Maximum length of typeface name */

/* Screen */
#define SCREEN_LEFT 0 /* Left edge */
#define SCREEN_TOP 0 /* Top edge */
#define SCREEN_WIDTH 640 /* Width */
#define SCREEN_HEIGHT 400 /* Height */
#define SCREEN_DEPTH 2 /* Depth */
#define SCREEN_DISPLAYID HIRESLACE_KEY /* Display ID */
#define SCREEN_TITLE "asl.library Font Requester Test Screen" /* Title */

/* Window */
#define WINDOW_LEFT 160 /* Left edge */
#define WINDOW_TOP 100 /* Top edge */
#define WINDOW_WIDTH 320 /* Width */
#define WINDOW_HEIGHT 200 /* Height */
#define WINDOW_TITLE "asl.library Font Requester Test Window" /* Title */

/* Command line interface */
#define CLI_TEMPLATE_BASE "EARLY/S,W=WINDOW/S,S=SCREEN/S,PSN=PUBSCREENNAME/K,PI=PRIVATEIDCMP/S,IMF=INTUIMSGFUNC/S,SW=SLEEPWINDOW/S,UD=USERDATA/S,TF=TYPEFACE/K,TS=TYPESIZE/N/K,B=BOLD/S,U=UNDERLINED/S,I=ITALIC/S,L=LOCALE/K,TT=TITLETEXT/K,PT=POSITIVETEXT/K,NT=NEGATIVETEXT/K,IL=INITIALLEFT/N/K,IT=INITIALTOP/N/K,IW=INITIALWIDTH/N/K,IH=INITIALHEIGHT/N/K"
#define CLI_TEMPLATE_EXTENDED "IN=INITIALNAME/K,IS=INITIALSIZE/N/K,IB=INITIALBOLD/S,IU=INITIALUNDERLINED/S,II=INITIALITALIC/S,IF=INITIALFLAGS/N/K,IFP=INITIALFRONTPEN/N/K,IBP=INITIALBACKPEN/N/K,IJ1=INITIALJAM1/S,IJ2=INITIALJAM2/S,ICOMP=INITIALCOMPLEMENT/S,IIV=INITIALINVERSVID/S,F=FLAGS/N/K,DFP=DOFRONTPEN/S,DBP=DOBACKPEN/S,DS=DOSTYLE/S,DDM=DODRAWMODE/S,FWO=FIXEDWIDTHONLY/S,MINH=MINHEIGHT/N/K,MAXH=MAXHEIGHT/N/K,FF=FILTERFUNC/K,HF=HOOKFUNC/S,ML=MODELIST/S"

#define CLI_TEMPLATE_LENGTH 1024 /* Maximum length of complete template */

#define CLI_EARLY 0
#define CLI_WINDOW 1
#define CLI_SCREEN 2
#define CLI_PUBSCREENNAME 3
#define CLI_PRIVATEIDCMP 4
#define CLI_INTUIMSGFUNC 5
#define CLI_SLEEPWINDOW 6
#define CLI_USERDATA 7
#define CLI_TYPEFACE 8
#define CLI_TYPESIZE 9
#define CLI_BOLD 10
#define CLI_UNDERLINED 11
#define CLI_ITALIC 12
#define CLI_LOCALE 13
#define CLI_TITLETEXT 14
#define CLI_POSITIVETEXT 15
#define CLI_NEGATIVETEXT 16
#define CLI_INITIALLEFT 17
#define CLI_INITIALTOP 18
#define CLI_INITIALWIDTH 19
#define CLI_INITIALHEIGHT 20

#define CLI_INITIALNAME 21
#define CLI_INITIALSIZE 22
#define CLI_INITIALBOLD 23
#define CLI_INITIALUNDERLINED 24
#define CLI_INITIALITALIC 25
#define CLI_INITIALFLAGS 26
#define CLI_INITIALFRONTPEN 27
#define CLI_INITIALBACKPEN 28
#define CLI_INITIALJAM1 29
#define CLI_INITIALJAM2 30
#define CLI_INITIALCOMPLEMENT 31
#define CLI_INITIALINVERSVID 32
#define CLI_FLAGS 33
#define CLI_DOFRONTPEN 34
#define CLI_DOBACKPEN 35
#define CLI_DOSTYLE 36
#define CLI_DODRAWMODE 37
#define CLI_FIXEDWIDTHONLY 38
#define CLI_MINHEIGHT 39
#define CLI_MAXHEIGHT 40
#define CLI_FILTERFUNC 41
#define CLI_HOOKFUNC 42
#define CLI_MODELIST 43

#define CLI_OPTIONS 44

/* asl.library */
#define ASL_TAG_COUNT 256       /* Maximum number of tags */

/*
 * Globals
 */

struct Library *GfxBase=NULL; /* graphics.library base */
struct Library *IntuitionBase=NULL; /* intuition.library base */
struct Library *DiskfontBase=NULL; /* diskfont.library base */

struct Library *AslBase=NULL; /* asl.library base */
struct Library *LocaleBase=NULL; /* locale.library base */

struct RDArgs *rdArgs=NULL; /* RDArgs control structure */

struct Screen *screen=NULL; /* Screen */
struct Window *window=NULL; /* Window */

struct TextFont *textFont=NULL; /* Text font */

struct FontRequester *fontRequester=NULL; /* Font requester */

struct Locale *locale=NULL; /* Locale */

UBYTE filterPattern[TYPEFACE_LENGTH]; /* Filter pattern */

ULONG intuiMsgHit=0L; /* IntuiMessage hook hit count */
ULONG intuiMsgBadHook=0L; /* IntuiMessage hook bad hook count */
ULONG intuiMsgBadObject=0L; /* IntuiMessage hook bad object count */
ULONG intuiMsgBadMessage=0L; /* IntuiMessage hook bad message count */

ULONG filterHit=0L; /* Filter hook hit count */
ULONG filterBadHook=0L; /* Filter hook bad hook count */
ULONG filterBadObject=0L; /* Filter hook bad object count */

/*
 * Prototypes
 */

void shutdown(int returnCode);
void main(int argc,char *argv[]);

struct TagItem *createTagArray(ULONG count);
void destroyTagArray(struct TagItem *tagArray);
void appendTagArray(struct TagItem *tagArray,Tag tag,ULONG data);

ULONG __saveds __asm intuiMsgFunc(register __a0 struct Hook *hook,
    register __a2 struct FontRequester *hookFontRequester,
    register __a1 struct IntuiMessage *intuiMessage);
ULONG  __saveds __asm filterFunc(register __a0 struct Hook *hook,
    register __a2 struct FontRequester *hookFontRequester,
    register __a1 struct TextAttr *textAttr);

/****** tag/createTagArray ******************************************
*
*   NAME
*       createTagArray -- create tag array
*
*   SYNOPSIS
*       tagArray=createTagArray(count);
*
*       struct TagItem *createTagArray(ULONG count);
*
*   FUNCTION
*       Create tag array.
*
*   INPUTS
*       count -- number of TagItems for array
*
*   RESULT
*       tagItem -- tag array
*
*   EXAMPLE
*
*       #define TAG_COUNT 256
*
*       struct TagItem *tagArray;
*
*       tagArray=createTagArray(TAG_COUNT);
*       if (!tagArray) {
*           printf("Error allocating tag array\n");
*           ...
*       }
*
*       to create tag array of 256 elements.
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*       destroyTagArray(), appendTagItem()
*
******************************************************************************
*
*/

struct TagItem *createTagArray(ULONG count)
{

    struct TagItem *tagArray;

    /* Allocate tag array */
    tagArray=AllocVec(sizeof(struct TagItem)*count,NULL);
    if (!tagArray) {
        return(NULL);
    }

    /* Initialize end of tag array marker */
    tagArray[0].ti_Tag=TAG_DONE;
    tagArray[0].ti_Data=NULL; /* Not really necessary -- I'm just picky */

    /* Return tag array */
    return(tagArray);

}

/****** tag/destroyTagArray ******************************************
*
*   NAME
*       destroyTagArray -- destroy tag array
*
*   SYNOPSIS
*       destroyTagArray(tagArray);
*
*       void destroyTagArray(struct TagItem *tagArray);
*
*   FUNCTION
*       Destroy tag array created by createTagArray().
*
*   INPUTS
*       tagArray -- tag array
*
*   RESULT
*       None
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*       createTagArray(), appendTagItem()
*
******************************************************************************
*
*/

void destroyTagArray(struct TagItem *tagArray)
{

    /* Free tag array */
    FreeVec(tagArray);

}

/****** tag/appendTagItem ******************************************
*
*   NAME
*       appendTagItem -- append tag item to tag array
*
*   SYNOPSIS
*       appendTagItem(tagArray,tag,data);
*
*       void appendTagItem(struct TagItem *tagArray,Tag tag,ULONG data);
*
*   FUNCTION
*       Append tag item to tag array.
*
*   INPUTS
*       tagArray -- tag  valuearray
*       tag -- tag
*       data -- tag data
*
*   RESULT
*       None
*
*   EXAMPLE
*
*       struct TagItem *tagArray;
*
*       struct Window *window;
*
*       ...
*
*       appendTagItem(tagArray,ASLFO_Window,(ULONG) window);
*
*       ...
*
*   NOTES
*
*   BUGS
*
*       No array bounds checking is implemented.
*
*       Do not append more tag items than specified in the createTagArray()
*           call. The end of tag array marker (with ti_Tag equal to TAG_DONE)
*           is included in this count.
*
*       Time complexity is linear to number of tag items in array. Improvable
*           by maintaining pointer to last tag. Not implemented for sake
*           of simplicity of calling conventions.
*
*   SEE ALSO
*       createTagArray(), destroyTagArray()
*
******************************************************************************
*
*/

void appendTagItem(struct TagItem *tagArray,Tag tag,ULONG data)
{

    struct TagItem *tagItem;

    /* Begin at head of tag array */
    tagItem=tagArray;

    /* Loop until end of tag array marker */
    while (tagItem->ti_Tag!=TAG_DONE) {
        tagItem++;
    }

    /* Fill tag item */
    tagItem->ti_Tag=tag;
    tagItem->ti_Data=data;

    /* Advance to next tag item */
    tagItem++;

    /* Write end of tag array marker */
    tagItem->ti_Tag=TAG_DONE;
    tagItem->ti_Data=NULL; /* Not really necessary -- I'm just picky */

}

/****** fontrequester/shutdown ******************************************
*
*   NAME
*       shutdown -- shutdown
*
*   SYNOPSIS
*       shutdown(returnCode);
*
*       void shutdown(int returnCode);
*
*   FUNCTION
*       Shutdown.
*
*   INPUTS
*       returnCode -- return code
*
*   RESULT
*       None
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
******************************************************************************
*
*/

void shutdown(int returnCode)
{

    /*
     * Text font close
     */

    /* Close text font */
    if (textFont) {
        CloseFont(textFont);
    }

    /*
     * Window close
     */

    /* Close window */
    if (window) {
        CloseWindow(window);
    }

    /*
     * Screen close
     */

    /* Close screen */
    if (screen) {
        CloseScreen(screen);
    }

    /*
     * Locale close
     */

    /* Close locale */
    if (locale) {
        CloseLocale(locale);
    }

    /*
     * Font requester close
     */

    /* Free font requester */
    if (fontRequester) {
        FreeAslRequest(fontRequester);
    }

    /*
     * Command line parsing close
     */

    /* Free command line parsing control structure */
    FreeArgs(rdArgs);

    /*
     * Library close
     */

    /* Close locale.library */
    if (LocaleBase) {
        CloseLibrary(LocaleBase);
    }

    /* Close asl.library */
    if (AslBase) {
        CloseLibrary(AslBase);
    }

    /* Close diskfont.library */
    if (DiskfontBase) {
        CloseLibrary(DiskfontBase);
    }

    /* Close intuition.library */
    if (IntuitionBase) {
        CloseLibrary(IntuitionBase);
    }

    /* Close graphics.library */
    if (GfxBase) {
        CloseLibrary(GfxBase);
    }

    /*
     * Exit
     */

    /* Exit with return code */
    exit(returnCode);

}

/****** main/main ******************************************
*
*   NAME
*       main -- entry point
*
*   SYNOPSIS
*       main(argc,argv);
*
*       void main(int argc,char *argv[]);
*
*   FUNCTION
*       Entry point.
*
*   INPUTS
*       argc -- argument count
*       argv -- argument value array
*
*   RESULT
*       None
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*       If there are any, this is how to get to them.
*
*   SEE ALSO
*
******************************************************************************
*
*/

void main(int argc,char *argv[])
{

    static WORD screenPens[] ={
        -1
    }; /* Screen pen array */
    static LONG args[CLI_OPTIONS]; /* Argument array */
    static struct TagItem *tagArray; /* Tag array */
    static struct TextAttr textAttr;  /* Text attribute */

    static UBYTE template[CLI_TEMPLATE_LENGTH]; /* Full template */

    static STRPTR modeList[] ={
        "DrawMode",
        "JAM1",
        "JAM2",
        "COMPLEMENT",
        NULL
    };

    UBYTE initialStyle;
    UBYTE initialDrawMode;

    BOOL result; /* Result of font requester */
    LONG error; /* Error */

    /* IntuiMessage function hook */
    static struct Hook intuiMsgHook ={
        NULL, /* mln_Succ */
        NULL, /* mln_Pred */
        intuiMsgFunc, /* h_Entry */
        NULL, /* h_SubEntry */
        NULL /* h_Data */
    };

    /* Filter function hook */
    static struct Hook filterHook ={
        NULL, /* mln_Succ */
        NULL, /* mln_Pred */
        filterFunc, /* h_Entry */
        NULL, /* h_SubEntry */
        NULL /* h_Data */
    };

    /*
     * Library open
     */

    /* Open graphics.library */
    GfxBase=OpenLibrary("graphics.library",KICKSTART_VERSION);
    if (!GfxBase) {
        printf("Error opening graphics.library V%d\n",KICKSTART_VERSION);
        shutdown(RETURN_FAIL);
    }

    /* Open intuition.library */
    IntuitionBase=OpenLibrary("intuition.library",KICKSTART_VERSION);
    if (!IntuitionBase) {
        printf("Error opening intuition.library V%d\n",KICKSTART_VERSION);
        shutdown(RETURN_FAIL);
    }

    /* Open diskfont.library */
    DiskfontBase=OpenLibrary("diskfont.library",KICKSTART_VERSION);
    if (!DiskfontBase) {
        printf("Error opening diskfont.library V%d\n",KICKSTART_VERSION);
        shutdown(RETURN_FAIL);
    }

    /* Open asl.library */
    AslBase=OpenLibrary("asl.library",WORKBENCH_VERSION);
    if (!AslBase) {
        printf("Error opening asl.library V%d\n",WORKBENCH_VERSION);
        shutdown(RETURN_FAIL);
    }

    /* Open locale.library */
    LocaleBase=OpenLibrary("locale.library",WORKBENCH_VERSION);
    if (!LocaleBase) {
        printf("Error opening locale.library V%d\n",WORKBENCH_VERSION);
        shutdown(RETURN_FAIL);
    }

    /*
     * Command line parsing
     */

    /* Clear argument array */
    memset(args,0,sizeof(args));

    /* Construct template -- kludge to work-around limit on length of
       constant strings in SAS C Version 5.10 and earler */
    strcpy(template,CLI_TEMPLATE_BASE);
    strcat(template,",");
    strcat(template,CLI_TEMPLATE_EXTENDED);

    /* Parse command line */
    rdArgs=ReadArgs(template,args,NULL);
    if (!rdArgs) {
        PrintFault(IoErr(),APP_NAME);
        shutdown(RETURN_ERROR);
    }

    /*
     * Screen open
     */

    /* If SCREEN argument is TRUE and PUBSCREENNAME is TRUE ... */
    if (args[CLI_SCREEN] && args[CLI_PUBSCREENNAME]) {
        printf("SCREEN and PUBSCREENNAME arguments are mutually exclusive\n");
        shutdown(RETURN_ERROR);
    }

    /* If SCREEN argument is TRUE ... */
    if (args[CLI_SCREEN]) {

        /* Open screen */
        screen=OpenScreenTags(NULL,
            SA_Left, SCREEN_LEFT,
            SA_Top, SCREEN_TOP,
            SA_Width, SCREEN_WIDTH,
            SA_Height, SCREEN_HEIGHT,
            SA_Depth, SCREEN_DEPTH,
            SA_Title, SCREEN_TITLE,
            SA_Type, CUSTOMSCREEN,
            SA_DisplayID, SCREEN_DISPLAYID,
            SA_Pens, screenPens,
            TAG_DONE);
        if (!screen) {
            printf("Error opening screen\n");
            shutdown(RETURN_FAIL);
        }

    }

    /* If PUBSCREENNAME argument is TRUE ... */
    if (args[CLI_PUBSCREENNAME]) {

        /* Open public screen */
        screen=OpenScreenTags(NULL,
            SA_Left, SCREEN_LEFT,
            SA_Top, SCREEN_TOP,
            SA_Width, SCREEN_WIDTH,
            SA_Height, SCREEN_HEIGHT,
            SA_Depth, SCREEN_DEPTH,
            SA_Title, args[CLI_PUBSCREENNAME],
            SA_Type, CUSTOMSCREEN,
            SA_DisplayID, SCREEN_DISPLAYID,
            SA_Pens, screenPens,
            SA_PubName, args[CLI_PUBSCREENNAME],
            TAG_DONE);
        if (!screen) {
            printf("Error opening screen\n");
            shutdown(RETURN_FAIL);
        }

    }

    /*
     * Window open
     */

    /* If WINDOW argument is TRUE ... */
    if (args[CLI_WINDOW]) {

        /* If screen open ... */
        if (screen) {

            /* Open window on screen */
            window=OpenWindowTags(NULL,
                WA_Left, WINDOW_LEFT,
                WA_Top, WINDOW_TOP,
                WA_Width, WINDOW_WIDTH,
                WA_Height, WINDOW_HEIGHT,
                WA_Title, WINDOW_TITLE,
                WA_DragBar, TRUE,
                WA_DepthGadget, TRUE,
                WA_SizeGadget, TRUE,
                WA_CloseGadget, TRUE,
                WA_NoCareRefresh, TRUE,
                WA_CustomScreen, screen,
                WA_IDCMP, IDCMP_CLOSEWINDOW|IDCMP_NEWSIZE|IDCMP_MENUPICK,
                TAG_DONE);

        } else {

            /* Open window on default public screen */
            window=OpenWindowTags(NULL,
                WA_Left, WINDOW_LEFT,
                WA_Top, WINDOW_TOP,
                WA_Width, WINDOW_WIDTH,
                WA_Height, WINDOW_HEIGHT,
                WA_Title, WINDOW_TITLE,
                WA_DragBar, TRUE,
                WA_DepthGadget, TRUE,
                WA_SizeGadget, TRUE,
                WA_CloseGadget, TRUE,
                WA_NoCareRefresh, TRUE,
                WA_IDCMP, IDCMP_CLOSEWINDOW|IDCMP_NEWSIZE|IDCMP_MENUPICK,
                TAG_DONE);

        }

        if (!window) {
            printf("Error opening window\n");
            shutdown(RETURN_FAIL);
        }

    }

    /*
     * Initialize tag array
     */

    /* Create tag array */
    tagArray=createTagArray(ASL_TAG_COUNT);
    if (!tagArray) {
        printf("Error creating tag array\n");
    }

    /* ASLFO_Window */
    if (window) {
        appendTagItem(tagArray,ASLFO_Window,(ULONG) window);
    }

    /* ASLFO_PubScreenName */
    if (args[CLI_PUBSCREENNAME]) {
        appendTagItem(tagArray,ASLFO_PubScreenName,args[CLI_PUBSCREENNAME]);
    }

    /* ASLFO_Screen */
    if (screen) {
        appendTagItem(tagArray,ASLFO_Screen,(ULONG) screen);
    }

    /* ASLFO_PrivateIDCMP */
    if (args[CLI_PRIVATEIDCMP]) {
        appendTagItem(tagArray,ASLFO_PrivateIDCMP,TRUE);
    }

    /* ASLFO_IntuiMsgFunc */
    if (args[CLI_INTUIMSGFUNC]) {
        appendTagItem(tagArray,ASLFO_IntuiMsgFunc,(ULONG) &intuiMsgHook);
    }

    /* ASLFO_SleepWindow */
    if (args[CLI_SLEEPWINDOW]) {
        appendTagItem(tagArray,ASLFO_SleepWindow,TRUE);
    }

    /* ASLFO_UserData */
    if (args[CLI_USERDATA]) {
        appendTagItem(tagArray,ASLFO_UserData,APP_USER_DATA);
    }

    /* ASLFO_TextAttr */
    if (args[CLI_TYPEFACE] || args[CLI_TYPESIZE] ||
        args[CLI_BOLD] || args[CLI_UNDERLINED] || args[CLI_ITALIC]) {

        /* If typeface specified ... */
        if (args[CLI_TYPEFACE]) {
            /* Use specified typeface */
            textAttr.ta_Name=(STRPTR) args[CLI_TYPEFACE];
        /* ... else ... */
        } else {
            /* Use default typeface */
            textAttr.ta_Name=TEXTATTR_NAME;
        }

        /* If type size specified ... */
        if (args[CLI_TYPESIZE]) {
            /* Use specified type size */
            textAttr.ta_YSize=*((LONG *) args[CLI_TYPESIZE]);
        } else {
            /* Use default type size */
            textAttr.ta_YSize=TEXTATTR_SIZE;
        }

        /* Clear style */
        textAttr.ta_Style=NULL;

        /* If bold specified ... */
        if (args[CLI_BOLD]) {
            /* Set bold flag */
            textAttr.ta_Style|=FSF_BOLD;
        }

        /* If underlined specified ... */
        if (args[CLI_UNDERLINED]) {
            /* Set underlined flag */
            textAttr.ta_Style|=FSF_UNDERLINED;
        }

        /* If italic specified ... */
        if (args[CLI_ITALIC]) {
            /* Set italic flag */
            textAttr.ta_Style|=FSF_ITALIC;
        }

        /* Clear flags */
        textAttr.ta_Flags=NULL;

        /* Open font to force loading from disk */
        textFont=OpenDiskFont(&textAttr);
        if (!textFont) {
            printf("Error opening font\n");
            shutdown(RETURN_FAIL);
        }

        /* Append ASLFO_TextAttr tag */
        appendTagItem(tagArray,ASLFO_TextAttr,(ULONG) &textAttr);

    }

    /* ASLFO_Locale */
    if (args[CLI_LOCALE]) {

        /* Open locale */
        locale=OpenLocale((STRPTR) args[CLI_LOCALE]);
        if (!locale) {
            printf("Error opening locale %s\n",args[CLI_LOCALE]);
        }

        /* Append locale tag */
        appendTagItem(tagArray,ASLFO_Locale,(ULONG) locale);

    }

    /* ASLFO_TitleText */
    if (args[CLI_TITLETEXT]) {
        appendTagItem(tagArray,ASLFO_TitleText,args[CLI_TITLETEXT]);
    }

    /* ASLFO_PositiveText */
    if (args[CLI_POSITIVETEXT]) {
        appendTagItem(tagArray,ASLFO_PositiveText,args[CLI_POSITIVETEXT]);
    }

    /* ASLFO_NegativeText */
    if (args[CLI_NEGATIVETEXT]) {
        appendTagItem(tagArray,ASLFO_NegativeText,args[CLI_NEGATIVETEXT]);
    }

    /* ASLFO_InitialLeftEdge */
    if (args[CLI_INITIALLEFT]) {
        appendTagItem(tagArray,ASLFO_InitialLeftEdge,*((LONG *) args[CLI_INITIALLEFT]));
    }

    /* ASLFO_InitialTopEdge */
    if (args[CLI_INITIALTOP]) {
        appendTagItem(tagArray,ASLFO_InitialTopEdge,*((LONG *) args[CLI_INITIALTOP]));
    }

    /* ASLFO_InitialWidth */
    if (args[CLI_INITIALWIDTH]) {
        appendTagItem(tagArray,ASLFO_InitialWidth,*((LONG *) args[CLI_INITIALWIDTH]));
    }

    /* ASLFO_InitialHeight */
    if (args[CLI_INITIALHEIGHT]) {
        appendTagItem(tagArray,ASLFO_InitialHeight,*((LONG *) args[CLI_INITIALHEIGHT]));
    }

    /* ASLFO_InitialName */
    if (args[CLI_INITIALNAME]) {
        appendTagItem(tagArray,ASLFO_InitialName,(ULONG) args[CLI_INITIALNAME]);
    }

    /* ASLFO_InitialSize */
    if (args[CLI_INITIALSIZE]) {
        appendTagItem(tagArray,ASLFO_InitialSize,*((LONG *) args[CLI_INITIALSIZE]));
    }

    /* ASLFO_InitialStyle */
    if (args[CLI_INITIALBOLD] || args[CLI_INITIALUNDERLINED] || args[CLI_INITIALITALIC]) {

        initialStyle=NULL;

        if (args[CLI_INITIALBOLD]) {
            initialStyle|=FSF_BOLD;
        }

        if (args[CLI_INITIALUNDERLINED]) {
            initialStyle|=FSF_UNDERLINED;
        }

        if (args[CLI_INITIALITALIC]) {
            initialStyle|=FSF_ITALIC;
        }

        appendTagItem(tagArray,ASLFO_InitialStyle,initialStyle);

    }

    /* ASLFO_InitialFlags */
    if (args[CLI_INITIALFLAGS]) {
        appendTagItem(tagArray,ASLFO_InitialFlags,*((LONG *) args[CLI_INITIALFLAGS]));
    }

    /* ASLFO_InitialFrontPen */
    if (args[CLI_INITIALFRONTPEN]) {
        appendTagItem(tagArray,ASLFO_InitialFrontPen,*((LONG *) args[CLI_INITIALFRONTPEN]));
    }

    /* ASLFO_InitialBackPen */
    if (args[CLI_INITIALBACKPEN]) {
        appendTagItem(tagArray,ASLFO_InitialBackPen,*((LONG *) args[CLI_INITIALBACKPEN]));
    }

    /* ASLFO_InitialDrawMode */
    if (args[CLI_INITIALJAM1] || args[CLI_INITIALJAM2] || args[CLI_INITIALCOMPLEMENT] || args[CLI_INITIALINVERSVID]) {

        initialDrawMode=NULL;

        if (args[CLI_INITIALJAM1]) {
            initialDrawMode|=JAM1;
        }

        if (args[CLI_INITIALJAM2]) {
            initialDrawMode|=JAM2;
        }

        if (args[CLI_INITIALCOMPLEMENT]) {
            initialDrawMode|=COMPLEMENT;
        }

        if (args[CLI_INITIALINVERSVID]) {
            initialDrawMode|=INVERSVID;
        }

        appendTagItem(tagArray,ASLFO_InitialDrawMode,initialDrawMode);
    }

    /* ASLFO_Flags */
    if (args[CLI_FLAGS]) {
        appendTagItem(tagArray,ASLFO_Flags,*((LONG *) args[CLI_FLAGS]));
    }

    /* ASLFO_DoFrontPen */
    if (args[CLI_DOFRONTPEN]) {
        appendTagItem(tagArray,ASLFO_DoFrontPen,TRUE);
    }

    /* ASLFO_DoBackPen */
    if (args[CLI_DOBACKPEN]) {
        appendTagItem(tagArray,ASLFO_DoBackPen,TRUE);
    }

    /* ASLFO_DoStyle */
    if (args[CLI_DOSTYLE]) {
        appendTagItem(tagArray,ASLFO_DoStyle,TRUE);
    }

    /* ASLFO_DoDrawMode */
    if (args[CLI_DODRAWMODE]) {
        appendTagItem(tagArray,ASLFO_DoDrawMode,TRUE);
    }

    /* ASLFO_FixedWidthOnly */
    if (args[CLI_FIXEDWIDTHONLY]) {
        appendTagItem(tagArray,ASLFO_FixedWidthOnly,TRUE);
    }

    /* ASLFO_MinHeight */
    if (args[CLI_MINHEIGHT]) {
        appendTagItem(tagArray,ASLFO_MinHeight,*((LONG *) args[CLI_MINHEIGHT]));
    }

    /* ASLFO_MaxHeight */
    if (args[CLI_MAXHEIGHT]) {
        appendTagItem(tagArray,ASLFO_MaxHeight,*((LONG *) args[CLI_MAXHEIGHT]));
    }

    /* ASLFO_FilterFunc */
    if (args[CLI_FILTERFUNC]) {

        /* Parse filter pattern */
        if (ParsePatternNoCase((STRPTR) args[CLI_FILTERFUNC],
            filterPattern,TYPEFACE_LENGTH)==-1) {
            printf("Error parsing filter pattern\n");
            shutdown(RETURN_FAIL);
        }

        /* Append ASLFO_FilterFunc tag */
        appendTagItem(tagArray,ASLFO_FilterFunc,(ULONG) &filterHook);

    }

    /* ASLFO_HookFunc */
    if (args[CLI_HOOKFUNC]) {
        /* !!! Not yet implemented !!! */
        printf("ASLFO_HookFunc not yet implemented\n");
    }

    /* ASLFO_ModeList */
    if (args[CLI_MODELIST]) {
        appendTagItem(tagArray,ASLFO_ModeList,(LONG) modeList);
    }

    /*
     * Open font requester
     */

    /* Allocate font requester; use tag array if EARLY specified */
    fontRequester=AllocAslRequest(ASL_FontRequest,args[CLI_EARLY]?tagArray:NULL);
    if (!fontRequester) {
        printf("Error allocating font requester\n");
        destroyTagArray(tagArray);
        shutdown(RETURN_FAIL);
    }

    /* Open font requester */
    result=AslRequest(fontRequester,args[CLI_EARLY]?NULL:tagArray);

    /* Fetch secondary error return */
    error=IoErr();

    /* Output success/failure */
    printf("AslRequest() return code %s\n",result?"TRUE":"FALSE");

    /* If failure ... */
    if (!result) {

        /* Output diagnostics */
        switch (error) {

            case 0:
                printf("Font requester cancelled by user\n");
                break;

            default:
                PrintFault(error,APP_NAME);
                break;
        }

    }

    /*
     * Destroy tag array
     */

    /* Destroy tag array */
    destroyTagArray(tagArray);

    /*
     * Dump FontRequester structure
     */

    /* If success ... */
    if (result) {

        /* Dump FontRequester structure */
        printf("fo_Attr.ta_Name=%s\n",fontRequester->fo_Attr.ta_Name);
        printf("fo_Attr.ta_YSize=%d\n",fontRequester->fo_Attr.ta_YSize);
        printf("fo_Attr.ta_Style=$%02x\n",fontRequester->fo_Attr.ta_Style);
        printf("fo_Attr.ta_Flags=$%02x\n",fontRequester->fo_Attr.ta_Flags);

        printf("fo_FrontPen=%d\n",fontRequester->fo_FrontPen);
        printf("fo_BackPen=%d\n",fontRequester->fo_BackPen);
        printf("fo_DrawMode=%d\n",fontRequester->fo_DrawMode);
        printf("fo_UserData=$%08lx\n",fontRequester->fo_UserData);

        printf("fo_LeftEdge=%d\n",fontRequester->fo_LeftEdge);
        printf("fo_TopEdge=%d\n",fontRequester->fo_TopEdge);
        printf("fo_Width=%d\n",fontRequester->fo_Width);
        printf("fo_Height=%d\n",fontRequester->fo_Height);

    }

    /*
     * Dump hook data
     */

    /* IntuiMsg hook hits */
    if (intuiMsgHit) {
        printf("IntuiMsg hook hits: %lu\n",intuiMsgHit);
    }

    /* IntuiMsg bad hooks */
    if (intuiMsgBadHook) {
        printf("IntuiMsg bad hooks: %lu\n",intuiMsgBadHook);
    }

    /* IntuiMsg bad objects */
    if (intuiMsgBadObject) {
        printf("IntuiMsg bad objects: %lu\n",intuiMsgBadObject);
    }

    /* IntuiMsg bad messages */
    if (intuiMsgBadMessage) {
        printf("IntuiMsg bad messages: %lu\n",intuiMsgBadMessage);
    }

    /* Filter hook hits */
    if (filterHit) {
        printf("Filter hook hits: %lu\n",filterHit);
    }

    /* Filter bad hooks */
    if (filterBadHook) {
        printf("Filter bad hooks: %lu\n",filterBadHook);
    }

    /* Filter bad objects */
    if (filterBadObject) {
        printf("Filter bad objects: %lu\n",filterBadObject);
    }

    /*
     * Fall-through
     */

    shutdown(RETURN_OK);

}

/****** fontrequester/intuiMsgFunc ******************************************
*
*   NAME
*       intuiMsgFunc -- IntuiMessage hook function
*
*   SYNOPSIS
*       success=intuiMsgFunc(hook,hookFontRequester,intuiMessage);
*
*       ULONG intuiMsgFunc(struct Hook *hook,
*           struct FontRequester *hookFontRequester,
*           struct IntuiMessage *intuiMessage);
*
*   FUNCTION
*       IntuiMessage hook function. Outputs arguments to debugging terminal
*       on serial port and counts hits, bad hooks, bad objects, and bad messages.
*
*   INPUTS
*       hook -- hook structure originating call
*       hookFontRequester -- object originating call
*       intuiMessage -- message originating call
*
*   RESULT
*       success -- always TRUE
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
******************************************************************************
*
*/

ULONG __saveds __asm intuiMsgFunc(register __a0 struct Hook *hook,
    register __a2 struct FontRequester *hookFontRequester,
    register __a1 struct IntuiMessage *intuiMessage)
{

    /* Output debugging message */
#ifdef DEBUG
    kprintf("intuiMsgFunc\n");
    kprintf("   hook=$%08lx\n",hook);
    kprintf("   hookFontRequester=$%08lx\n",hookFontRequester);
    kprintf("   intuiMessage=$%08lx\n",intuiMessage);
#endif /* DEBUG */

    /* Increment IntuiMessage hit counter */
    intuiMsgHit++;

    /* If hook bad ... */
    if (hook->h_Entry!=intuiMsgFunc) {

#ifdef DEBUG
        /* Output to debugging terminal */
        kprintf("   Bad hook\n");
#endif /* DEBUG */

        /* Increment bad hook count */
        intuiMsgBadHook++;
    }

    /* If font requester bad ... */
    if (hookFontRequester!=fontRequester) {

#ifdef DEBUG
        /* Output to debugging terminal */
        kprintf("   Bad object\n");
#endif /* DEBUG */

        /* Increment bad object count */
        intuiMsgBadObject++;

    }

    /* If message bad ... */
    if (intuiMessage->IDCMPWindow!=window) {

#ifdef DEBUG
        /* Output to debugging terminal */
        kprintf("   Bad message\n");
#endif /* DEBUG */


        /* Increment bad message count */
        intuiMsgBadMessage++;

    }

    /* Success */
    return(TRUE);

}

/****** fontrequester/filterFunc ******************************************
*
*   NAME
*       filterFunc -- filter hook
*
*   SYNOPSIS
*       accept=filterFunc(hook,hookFontRequester,anchorPath);
*
*       ULONG filterFunc(struct Hook *hook,
*           struct FonteRequester *hookFontRequester,
*           struct TextAttr *textAttr);
*
*   FUNCTION
*       Filter hook function. Outputs arguments to debugging terminal
*       on serial port and counts hits, bad hooks, bad objects, and bad messages.
*
*       Filters out any font not matching the pattern in the global
*       variable filterPattern.
*
*   INPUTS
*       hook -- hook originating call
*       textAttr -- text attribute describing font to filter
*       hookFontRequester -- fille requester originating call
*
*   RESULT
*       accept -- TRUE to accept; FALSE to reject
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
******************************************************************************
*
*/

ULONG __saveds __asm filterFunc(register __a0 struct Hook *hook,
    register __a2 struct FontRequester *hookFontRequester,
    register __a1 struct TextAttr *textAttr)
{

    BOOL accept;

#ifdef DEBUG
    /* Output debugging message */
    kprintf("filterFunc\n");
    kprintf("   hook=$%08lx\n",hook);
    kprintf("   hookFontRequester=$%08lx\n",hookFontRequester);
    kprintf("   textAttr=$%08lx\n",textAttr);
    kprintf("            %s %ld\n",textAttr->ta_Name,textAttr->ta_YSize);
#endif /* DEBUG */

    /* Increment filter hook hit count */
    filterHit++;

    /* If hook bad ... */
    if (hook->h_Entry!=filterFunc) {

#ifdef DEBUG
        /* Output to debugging terminal */
        kprintf("   Bad hook\n");
#endif /* DEBUG */

        /* Increment bad hook count */
        filterBadHook++;
    }

    /* If font requester bad ... */
    if (hookFontRequester!=fontRequester) {

#ifdef DEBUG
        /* Output to debugging terminal */
        kprintf("   Bad object\n");
#endif /* DEBUG */

        /* Increment bad object count */
        filterBadObject++;

    }

    /* Accept if object matches pattern */
    accept=MatchPatternNoCase(filterPattern,textAttr->ta_Name);
#ifdef DEBUG
    kprintf("    %s %ld %s\n",textAttr->ta_Name,textAttr->ta_YSize,accept?"accepted":"rejected");
#endif /* DEBUG */

    /* Return accept/reject */
    return((ULONG) accept);

}
