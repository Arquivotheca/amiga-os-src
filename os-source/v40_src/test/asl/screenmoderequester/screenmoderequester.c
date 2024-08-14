/*
    asl.library Screen Mode Requester Test
    Written by:
        John J. Szucs
        Amiga Systems Section
        Product Assurance Department
        Commodore International Services Company
    Copyright © 1992 Commodore International Services Company

Template:
    EARLY/S,W=WINDOW/S,S=SCREEN/S,PSN=PUBSCREENNAME/K,PI=PRIVATEIDCMP/S,IMF=INTUIMSGFUNC/S,SW=SLEEPWINDOW/S,UD=USERDATA/S,TF=TYPEFACE/K,TS=TYPESIZE/N/K,B=BOLD/S,U=UNDERLINED/S,I=ITALIC/S,L=LOCALE/K,TT=TITLETEXT/K,PT=POSITIVETEXT/K,NT=NEGATIVETEXT/K,IL=INITIALLEFT/N/K,IT=INITIALTOP/N/K,IW=INITIALWIDTH/N/K,IH=INITIALHEIGHT/N/K,IDID=INITIALDISPLAYID/N/K,IDW=INITIALDISPLAYWIDTH/N/K,IDH=INITIALDISPLAYHEIGHT/N/K,IDD=INITIALDISPLAYDEPTH/N/K,IOT=INITIALOVERSCANTYPE/N/K,IAS=INITIALAUTOSCROLL/N/K,IIO=INITIALINFOOPENED/S,IIL=INITIALINFOLEFT/N/K,IIT=INITIALINFOTOP/N/K,DW=DOWIDTH/S,DH=DOHEIGHT/S,DD=DODEPTH/S,DOT=DOOVERSCANTYPE/S,DAS=DOAUTOSCROLL/S,PF=PROPERTYFLAGS/N/K,PM=PROPERTYMASK/N/K,MINW=MINWIDTH/N/K,MAXW=MAXWIDTH/N/K,MINH=MINHEIGHT/N/K,MAXH=MAXHEIGHT/N/K,MIND=MINDEPTH/N/K,MAXD=MAXDEPTH/N/K,FF=FILTERFUNC/K,CSML=CUSTOMSMLIST/S

Usage:
    ScreenModeRequester [EARLY] [WINDOW] [SCREEN] [PUBSCREENNAME <name>]
        [PRIVATEIDCMP] [INTUIMSGFUNC] [SLEEPWINDOW] [USERDATA]
        [TYPEFACE <type face>] [TYPESIZE <type size>] [BOLD] [UNDERLINED] [ITALIC]
        [LOCALE <locale>] [TITLETEXT <title text>] [POSITIVETEXT <positive text>] [NEGATIVETEXT <negative text>]
        [INITIALLEFT <left>] [INITIALTOP <top>] [INITIALWIDTH <width>] [INITIALHEIGHT <height>]
        [INITIALDISPLAYID <display ID>]
        [INITIALDISPLAYWIDTH <width> [INITIALDISPLAYHEIGHT <heigh> [INITIALDISPLAYDEPTH <depth>]
        [INITIALOVERSCANTYPE <overscan>] [INITIALAUTOSCROLL <auto scroll>]
        [INITIALINFOOPENED] [INITIALINFOLEFT <left>] [INITIALINFOTOP <top>]
        [DOWIDTH] [DOHEIGHT] [DODEPTH] [DOOVERSCANTYPE] [DOAUTOSCROLL]
        [PROPERTYFLAGS <property flags>] [PROPERTYMASK <property mask>]
        [MINWIDTH <width>] [MAXWIDTH <width>] [MINHEIGHT <height>] [MAXHEIGHT <height>]
        [MINDEPTH <depth>] [MAXDEPTH <depth>]
        [FILTERFUNC <pattern>] [CUSTOMSMLIST]

    EARLY               -   Use tags in AllocAslRequest(); if not specified,
                        tags are used in AslRequest()
    WINDOW              -   Open window
    SCREEN              -   Open screen
    PUBSCREENNAME <name>  -   Open on public screen with name <name>
    PRIVATEIDCMP        -   Allocate private IDCMP
    INTUIMSGFUNC        -   Test IntuiMessage function hook
    SLEEPWINDOW         -   Sleep parent window
    USERDATA            -   Test user data (set to $C0EDBABE)
    TYPEFACE <type face> - Set type face to <type face>
    TYPESIZE <type size> -   Set type size to <type size>
    BOLD                -   Bold type
    UNDERLINED          -   Underlined type
    ITALIC              -   Italic type
    LOCALE <locale>     -   Use locale <locale>
    TITLETEXT <title text> - Use <title text> for title
    POSITIVETEXT <positive text> - Use <positive text> for positive text (default "OK")
    NEGATIVETEXT <negative text> - Use <negative text> for negative text (default "Cancel")
    INITIALLEFT <left>     - Initial left edge of <left>
    INITIALTOP <top>       -  Initial top edge of <top>
    INITIALWIDTH <width>   - Initial width of <width>
    INITIALHEIGHT <height> - Initial height of <height>
    INITIALDISPLAYID <display ID> - Initial display ID (decimal); from graphics/displayinfo.h
    INITIALDISPLAYWIDTH <width> - Initial display width
    INITIALDISPLAYHEIGHT <heigh> - Initial display height
    INITIALDISPLAYDEPTH <depth> - Initial display depth
    INITIALOVERSCANTYPE <overscan> - Initial overscan type (decimal); from graphics/displayinfo.h
    INITIALAUTOSCROLL <auto scroll> - Initial autoscroll
    INITIALINFOOPENED   - Open display mode information window at start-up
    INITIALINFOLEFT <left> - Left edge of display mode information window
    INITIALINFOTOP <top>- Top edge of display mode information window
    DOWIDTH             -   Do width
    DOHEIGHT            -   Do height
    DODEPTH             -   Do depth
    DOOVERSCANTYPE      -   Do overscan type selection
    DOAUTOSCROLL        -   Do auto scroll selection
    PROPERTYFLAGS <property flags> - Property flags (decimal); from graphics/displayinfo.h
    PROPERTYMASK <property mask> - Property mask (decimal); from graphics/displayinfo.h
    MINWIDTH <width>    -   Minimum width
    MAXWIDTH <width>    -   Maximum width
    MINHEIGHT <height>  -   Minimum height
    MAXHEIGHT <height>  -   Maximum height
    MINDEPTH <depth>    -   Minimum depth
    MAXDEPTH <depth>    -   Maximum depth
    FILTERFUNC <pattern>-   Test filter function; rejects any mode not matching <pattern>
    CUSTOMSMLIST        -   Test custom screen mode list

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
 * Globals
 */

struct Library *IntuitionBase=NULL; /* intuition.library base */
struct Library *GfxBase=NULL; /* graphics.library base */
struct Library *DiskfontBase=NULL; /* diskfont.library base */

struct Library *LocaleBase=NULL; /* locale.library base */
struct Library *AslBase=NULL; /* asl.library base */

struct RDArgs *rdArgs=NULL; /* RDArgs control structure */

struct Screen *screen=NULL; /* Screen */
struct Window *window=NULL; /* Window */

struct TextFont *textFont=NULL; /* Text font */

struct ScreenModeRequester *screenModeRequester=NULL; /* Screen mode requester */

struct Locale *locale=NULL; /* Locale */

UBYTE filterPattern[DISPLAYNAMELEN*2]; /* Parsed pattern for filter function */

ULONG intuiMsgHit=0L; /* IntuiMessage hook hit count */
ULONG intuiMsgBadHook=0L; /* IntuiMessage hook bad hook count */
ULONG intuiMsgBadObject=0L; /* IntuiMessage hook bad object count */
ULONG intuiMsgBadMessage=0L; /* IntuiMessage hook bad message count */

ULONG filterHit=0L; /* Filter hook hit count */
ULONG filterBadHook=0L; /* Filter hook bad hook count */
ULONG filterBadObject=0L; /* Filter hook bad object count */

/*
 * Constants
 */

/* System */
#define KICKSTART_VERSION 37 /* Version of Amiga Operating System Kickstart */
#define WORKBENCH_VERSION 38 /* Version of Amiga Operating System Workbench */

/* Application */
#define APP_NAME "ScreenModeRequester" /* Name of application */
#define APP_USER_DATA 0xC0EDBABE /* User data */

/* Text attribute */
#define TEXTATTR_NAME "topaz.font" /* Typeface name */
#define TEXTATTR_SIZE 8 /* Type size */

/* Screen */
#define SCREEN_LEFT 0 /* Left edge */
#define SCREEN_TOP 0 /* Top edge */
#define SCREEN_WIDTH 640 /* Width */
#define SCREEN_HEIGHT 400 /* Height */
#define SCREEN_DEPTH 2 /* Depth */
#define SCREEN_DISPLAYID HIRESLACE_KEY /* Display ID */
#define SCREEN_TITLE "asl.library Screen Mode Requester Test Screen" /* Title */
#define SCREEN_PUBLIC_NAME "ScreenModeRequester" /* Public screen name */

/* Window */
#define WINDOW_LEFT 160 /* Left edge */
#define WINDOW_TOP 100 /* Top edge */
#define WINDOW_WIDTH 320 /* Width */
#define WINDOW_HEIGHT 200 /* Height */
#define WINDOW_TITLE "asl.library Screen Mode Requester Test Window" /* Title */

/* Command line interface */
#define CLI_TEMPLATE_BASE "EARLY/S,W=WINDOW/S,S=SCREEN/S,PSN=PUBSCREENNAME/K,PI=PRIVATEIDCMP/S,IMF=INTUIMSGFUNC/S,SW=SLEEPWINDOW/S,UD=USERDATA/S,TF=TYPEFACE/K,TS=TYPESIZE/N/K,B=BOLD/S,U=UNDERLINED/S,I=ITALIC/S,L=LOCALE/K,TT=TITLETEXT/K,PT=POSITIVETEXT/K,NT=NEGATIVETEXT/K,IL=INITIALLEFT/N/K,IT=INITIALTOP/N/K,IW=INITIALWIDTH/N/K,IH=INITIALHEIGHT/N/K"
#define CLI_TEMPLATE_EXTENDED "IDID=INITIALDISPLAYID/N/K,IDW=INITIALDISPLAYWIDTH/N/K,IDH=INITIALDISPLAYHEIGHT/N/K,IDD=INITIALDISPLAYDEPTH/N/K,IOT=INITIALOVERSCANTYPE/N/K,IAS=INITIALAUTOSCROLL/N/K,IIO=INITIALINFOOPENED/S,IIL=INITIALINFOLEFT/N/K,IIT=INITIALINFOTOP/N/K,DW=DOWIDTH/S,DH=DOHEIGHT/S,DD=DODEPTH/S,DOT=DOOVERSCANTYPE/S,DAS=DOAUTOSCROLL/S,PF=PROPERTYFLAGS/N/K,PM=PROPERTYMASK/N/K,MINW=MINWIDTH/N/K,MAXW=MAXWIDTH/N/K,MINH=MINHEIGHT/N/K,MAXH=MAXHEIGHT/N/K,MIND=MINDEPTH/N/K,MAXD=MAXDEPTH/N/K,FF=FILTERFUNC/K,CSML=CUSTOMSMLIST/S"

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

#define CLI_INITIALDISPLAYID 21
#define CLI_INITIALDISPLAYWIDTH 22
#define CLI_INITIALDISPLAYHEIGHT 23
#define CLI_INITIALDISPLAYDEPTH 24
#define CLI_INITIALOVERSCANTYPE 25
#define CLI_INITIALAUTOSCROLL 26
#define CLI_INITIALINFOOPENED 27
#define CLI_INITIALINFOLEFT 28
#define CLI_INITIALINFOTOP 29
#define CLI_DOWIDTH 30
#define CLI_DOHEIGHT 31
#define CLI_DODEPTH 32
#define CLI_DOOVERSCANTYPE 33
#define CLI_DOAUTOSCROLL 34
#define CLI_PROPERTYFLAGS 35
#define CLI_PROPERTYMASK 36
#define CLI_MINWIDTH 37
#define CLI_MAXWIDTH 38
#define CLI_MINHEIGHT 39
#define CLI_MAXHEIGHT 40
#define CLI_MINDEPTH 41
#define CLI_MAXDEPTH 42
#define CLI_FILTERFUNC 43
#define CLI_CUSTOMSMLIST 44

#define CLI_OPTIONS 45

/* asl.library */
#define ASL_TAG_COUNT 256       /* Maximum number of tags */

/* Custom screen modes */
#define CUSTOM_DISPLAY_ID 0xFFFFBABE /* Base custom display ID */

/*
 * Prototypes
 */

void shutdown(int returnCode);
void main(int argc,char *argv[]);

struct TagItem *createTagArray(ULONG count);
void destroyTagArray(struct TagItem *tagArray);
void appendTagArray(struct TagItem *tagArray,Tag tag,ULONG data);

ULONG __saveds __asm intuiMsgFunc(register __a0 struct Hook *hook,
    register __a2 struct ScreenModeRequester *hookScreenModeRequester,
    register __a1 struct IntuiMessage *intuiMessage);
ULONG  __saveds __asm filterFunc(register __a0 struct Hook *hook,
    register __a2 struct ScreenModeRequester *hookScreenModeRequester,
    register __a1 ULONG modeID);

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
*       appendTagItem(tagArray,ASLSM_Window,(ULONG) window);
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

/****** screenrequester/shutdown ******************************************
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
     * Screen mode requester close
     */

    /* Free screen mode requester */
    if (screenModeRequester) {
        FreeAslRequest(screenModeRequester);
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

    BOOL result; /* Result of screen mode requester */
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

    /* Custom display mode list */
    static struct List displayModeList;
    static struct DisplayMode displayMode[] ={

        /* DisplayMode */
        {

            {

                /* Node */
                NULL, /* ln_Succ */
                NULL, /* ln_Pred */
                0, /* ln_Type */
                0, /* ln_Pri */
                "Super-Duper Res", /* ln_Name */

            },

            {

                /* DimensionInfo */
                {

                    /* Header */
                    DTAG_DIMS, /* StructID */
                    CUSTOM_DISPLAY_ID, /* DisplayID */
                    TAG_SKIP, /* SkipID */
                    0, /* Length */

                },

                32, /* MaxDepth */
                640, /* MinRasterWidth */
                480, /* MinRasterHeight */
                1024, /* MaxRasterWidth */
                1024, /* MaxRasterHeight */
                0, 0, 1023, 1023, /* Nomimal */
                0, 0, 1023, 1023, /* MaxOScan */
                0, 0, 1023, 1023, /* VideoOScan */
                0, 0, 1023, 1023, /* TxtOScan */
                0, 0, 1023, 1023, /* StdOScan */

            },

            DIPF_IS_FOREIGN|DIPF_IS_DRAGGABLE|DIPF_IS_WB|DIPF_IS_GENLOCK, /* dm_PropertyFlags */

        },

        {

            {

                /* Node */
                NULL, /* ln_Succ */
                NULL, /* ln_Pred */
                0, /* ln_Type */
                0, /* ln_Pri */
                "Super-Duper Extra-High-Res", /* ln_Name */

            },

            {

                /* DimensionInfo */
                {

                    /* Header */
                    DTAG_DISP, /* StructID */
                    CUSTOM_DISPLAY_ID+1, /* DisplayID */
                    TAG_SKIP, /* SkipID */
                    0, /* Length */

                },

                24, /* MaxDepth */
                1024, /* MinRasterWidth */
                1024, /* MinRasterHeight */
                2048, /* MaxRasterWidth */
                2048, /* MaxRasterHeight */
                0, 0, 2047, 2047, /* Nomimal */
                0, 0, 2047, 2047, /* MaxOScan */
                0, 0, 2047, 2047, /* VideoOScan */
                0, 0, 2047, 2047, /* TxtOScan */
                0, 0, 2047, 2047, /* StdOScan */

            },

            DIPF_IS_FOREIGN|DIPF_IS_DRAGGABLE|DIPF_IS_WB|DIPF_IS_GENLOCK, /* dm_PropertyFlags */

        },

        {

            /* Node */
            NULL, /* ln_Succ */
            NULL, /* ln_Pred */
            0, /* ln_Type */
            0, /* ln_Pri */
            NULL, /* ln_Name (terminator) */

        },

    };

    int index; /* General-purpose index variable */

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
                WA_IDCMP, IDCMP_CLOSEWINDOW|IDCMP_NEWSIZE|IDCMP_MENUPICK,
                WA_CustomScreen, screen,
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

    /* ASLSM_Window */
    if (window) {
        appendTagItem(tagArray,ASLSM_Window,(ULONG) window);
    }

    /* ASLSM_PubScreenName */
    if (args[CLI_PUBSCREENNAME]) {
        appendTagItem(tagArray,ASLSM_PubScreenName,args[CLI_PUBSCREENNAME]);
    }

    /* ASLSM_Screen */
    if (screen) {
        appendTagItem(tagArray,ASLSM_Screen,(ULONG) screen);
    }

    /* ASLSM_PrivateIDCMP */
    if (args[CLI_PRIVATEIDCMP]) {
        appendTagItem(tagArray,ASLSM_PrivateIDCMP,TRUE);
    }

    /* ASLFR_IntuiMsgFunc */
    if (args[CLI_INTUIMSGFUNC]) {
        appendTagItem(tagArray,ASLFR_IntuiMsgFunc,(ULONG) &intuiMsgHook);
    }

    /* ASLSM_SleepWindow */
    if (args[CLI_SLEEPWINDOW]) {
        appendTagItem(tagArray,ASLSM_SleepWindow,TRUE);
    }

    /* ASLSM_UserData */
    if (args[CLI_USERDATA]) {
        appendTagItem(tagArray,ASLSM_UserData,APP_USER_DATA);
    }

    /* ASLSM_TextAttr */
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

        /* Append ASLSM_TextAttr tag */
        appendTagItem(tagArray,ASLSM_TextAttr,(ULONG) &textAttr);

    }

    /* ASLSM_Locale */
    if (args[CLI_LOCALE]) {

        /* Open locale */
        locale=OpenLocale((STRPTR) args[CLI_LOCALE]);
        if (!locale) {
            printf("Error opening locale %s\n",args[CLI_LOCALE]);
        }

        /* Append locale tag */
        appendTagItem(tagArray,ASLSM_Locale,(ULONG) locale);

    }

    /* ASLSM_TitleText */
    if (args[CLI_TITLETEXT]) {
        appendTagItem(tagArray,ASLSM_TitleText,args[CLI_TITLETEXT]);
    }

    /* ASLSM_PositiveText */
    if (args[CLI_POSITIVETEXT]) {
        appendTagItem(tagArray,ASLSM_PositiveText,args[CLI_POSITIVETEXT]);
    }

    /* ASLSM_NegativeText */
    if (args[CLI_NEGATIVETEXT]) {
        appendTagItem(tagArray,ASLSM_NegativeText,args[CLI_NEGATIVETEXT]);
    }

    /* ASLSM_InitialLeftEdge */
    if (args[CLI_INITIALLEFT]) {
        appendTagItem(tagArray,ASLSM_InitialLeftEdge,*((LONG *) args[CLI_INITIALLEFT]));
    }

    /* ASLSM_InitialTopEdge */
    if (args[CLI_INITIALTOP]) {
        appendTagItem(tagArray,ASLSM_InitialTopEdge,*((LONG *) args[CLI_INITIALTOP]));
    }

    /* ASLSM_InitialWidth */
    if (args[CLI_INITIALWIDTH]) {
        appendTagItem(tagArray,ASLSM_InitialWidth,*((LONG *) args[CLI_INITIALWIDTH]));
    }

    /* ASLSM_InitialHeight */
    if (args[CLI_INITIALHEIGHT]) {
        appendTagItem(tagArray,ASLSM_InitialHeight,*((LONG *) args[CLI_INITIALHEIGHT]));
    }

    /* ASLSM_InitialDisplayID */
    if (args[CLI_INITIALDISPLAYID]) {
        appendTagItem(tagArray,ASLSM_InitialDisplayID,*((LONG *) args[CLI_INITIALDISPLAYID]));
    }

    /* ASLSM_InitialDisplayWidth */
    if (args[CLI_INITIALDISPLAYWIDTH]) {
        appendTagItem(tagArray,ASLSM_InitialDisplayWidth,*((LONG *) args[CLI_INITIALDISPLAYWIDTH]));
    }

    /* ASLSM_InitialDisplayHeight */
    if (args[CLI_INITIALDISPLAYHEIGHT]) {
        appendTagItem(tagArray,ASLSM_InitialDisplayHeight,*((LONG *) args[CLI_INITIALDISPLAYHEIGHT]));
    }

    /* ASLSM_InitialDisplayDepth */
    if (args[CLI_INITIALDISPLAYDEPTH]) {
        appendTagItem(tagArray,ASLSM_InitialDisplayDepth,*((LONG *) args[CLI_INITIALDISPLAYDEPTH]));
    }

    /* ASLSM_InitialOverscanType */
    if (args[CLI_INITIALOVERSCANTYPE]) {
        appendTagItem(tagArray,ASLSM_InitialOverscanType,*((LONG *) args[CLI_INITIALOVERSCANTYPE]));
    }

    /* ASLSM_InitialAutoScroll */
    if (args[CLI_INITIALAUTOSCROLL]) {
        appendTagItem(tagArray,ASLSM_InitialAutoScroll,*((LONG *) args[CLI_INITIALAUTOSCROLL]));
    }

    /* ASLSM_InitialInfoOpened */
    if (args[CLI_INITIALINFOOPENED]) {
        appendTagItem(tagArray,ASLSM_InitialInfoOpened,TRUE);
    }

    /* ASLSM_InitialInfoLeftEdge */
    if (args[CLI_INITIALINFOLEFT]) {
        appendTagItem(tagArray,ASLSM_InitialInfoLeftEdge,*((LONG *) args[CLI_INITIALINFOLEFT]));
    }

    /* ASLSM_InitialInfoTopEdge */
    if (args[CLI_INITIALINFOTOP]) {
        appendTagItem(tagArray,ASLSM_InitialInfoTopEdge,*((LONG *) args[CLI_INITIALINFOTOP]));
    }

    /* ASLSM_DoWidth */
    if (args[CLI_DOWIDTH]) {
        appendTagItem(tagArray,ASLSM_DoWidth,TRUE);
    }

    /* ASLSM_DoHeight */
    if (args[CLI_DOHEIGHT]) {
        appendTagItem(tagArray,ASLSM_DoHeight,TRUE);
    }

    /* ASLSM_DoDepth */
    if (args[CLI_DODEPTH]) {
        appendTagItem(tagArray,ASLSM_DoDepth,TRUE);
    }

    /* ASLSM_DoOverscanType */
    if (args[CLI_DOOVERSCANTYPE]) {
        appendTagItem(tagArray,ASLSM_DoOverscanType,TRUE);
    }

    /* ASLSM_DoAutoScroll */
    if (args[CLI_DOAUTOSCROLL]) {
        appendTagItem(tagArray,ASLSM_DoAutoScroll,TRUE);
    }

    /* ASLSM_PropertyFlags */
    if (args[CLI_PROPERTYFLAGS]) {
        appendTagItem(tagArray,ASLSM_PropertyFlags,*((LONG *) args[CLI_PROPERTYFLAGS]));
    }

    /* ASLSM_PropertyMask */
    if (args[CLI_PROPERTYMASK]) {
        appendTagItem(tagArray,ASLSM_PropertyMask,*((LONG *) args[CLI_PROPERTYMASK]));
    }

    /* ASLSM_MinWidth */
    if (args[CLI_MINWIDTH]) {
        appendTagItem(tagArray,ASLSM_MinWidth,*((LONG *) args[CLI_MINWIDTH]));
    }

    /* ASLSM_MaxWidth */
    if (args[CLI_MAXWIDTH]) {
        appendTagItem(tagArray,ASLSM_MaxWidth,*((LONG *) args[CLI_MAXWIDTH]));
    }

    /* ASLSM_MinHeight */
    if (args[CLI_MINHEIGHT]) {
        appendTagItem(tagArray,ASLSM_MinHeight,*((LONG *) args[CLI_MINHEIGHT]));
    }

    /* ASLSM_MaxHeight */
    if (args[CLI_MAXHEIGHT]) {
        appendTagItem(tagArray,ASLSM_MaxHeight,*((LONG *) args[CLI_MAXHEIGHT]));
    }

    /* ASLSM_MinDepth */
    if (args[CLI_MINDEPTH]) {
        appendTagItem(tagArray,ASLSM_MinDepth,*((LONG *) args[CLI_MINDEPTH]));
    }

    /* ASLSM_MaxDepth */
    if (args[CLI_MAXDEPTH]) {
        appendTagItem(tagArray,ASLSM_MaxDepth,*((LONG *) args[CLI_MAXDEPTH]));
    }

    /* ASLSM_FilterFunc */
    if (args[CLI_FILTERFUNC]) {

        /* Parse filter pattern */
        if (ParsePatternNoCase((STRPTR) args[CLI_FILTERFUNC],
            filterPattern,DISPLAYNAMELEN*2)==-1) {
            printf("Error parsing filter pattern\n");
            shutdown(RETURN_FAIL);
        }

        /* Append ASLSM_FilterFunc tag */
        appendTagItem(tagArray,ASLSM_FilterFunc,(ULONG) &filterHook);

    }

    /* ASLSM_CustomSMList */
    if (args[CLI_CUSTOMSMLIST]) {

        /* Initial custom mode list */
        NewList(&displayModeList);

        /* Append display modes */
        for (index=0;displayMode[index].dm_Node.ln_Name;index++) {
            AddTail(&displayModeList,(struct Node *) &displayMode[index]);
        }

        /* Append ASLSM_CustomSMList tag */
        appendTagItem(tagArray,ASLSM_CustomSMList,(ULONG) &displayModeList);

    }

    /*
     * Open screen mode requester
     */

    /* Allocate screen mode requester; use tag array if EARLY specified */
    screenModeRequester=AllocAslRequest(ASL_ScreenModeRequest,args[CLI_EARLY]?tagArray:NULL);
    if (!screenModeRequester) {
        printf("Error allocating screen mode requester\n");
        destroyTagArray(tagArray);
        shutdown(RETURN_FAIL);
    }

    /* Open screen mode requester */
    result=AslRequest(screenModeRequester,args[CLI_EARLY]?NULL:tagArray);

    /* Fetch secondary error return */
    error=IoErr();

    /* Output success/failure */
    printf("AslRequest() return code %s\n",result?"TRUE":"FALSE");

    /* If failure ... */
    if (!result) {

        /* Output diagnostics */
        switch (error) {

            case 0:
                printf("Screen mode requester cancelled by user\n");
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
     * Dump ScreenModeRequester structure
     */

    /* If success ... */
    if (result) {

        /* Dump ScreenModeRequester structure */
        printf("sm_DisplayID = $%08lx\n",screenModeRequester->sm_DisplayID);
        printf("sm_DisplayWidth = %lu\n",screenModeRequester->sm_DisplayWidth);
        printf("sm_DisplayHeight = %lu\n",screenModeRequester->sm_DisplayHeight);
        printf("sm_DisplayDepth = %u\n",screenModeRequester->sm_DisplayDepth);
        printf("sm_OverscanType = $%04x\n",screenModeRequester->sm_OverscanType);
        printf("sm_AutoScroll = %s\n",screenModeRequester->sm_AutoScroll?"TRUE":"FALSE");
        printf("sm_BitMapWidth = %lu\n",screenModeRequester->sm_BitMapWidth);
        printf("sm_BitMapHeight = %lu\n",screenModeRequester->sm_BitMapHeight);
        printf("sm_LeftEdge = %d\n",screenModeRequester->sm_LeftEdge);
        printf("sm_TopEdge = %d\n",screenModeRequester->sm_TopEdge);
        printf("sm_Width = %d\n",screenModeRequester->sm_Width);
        printf("sm_Height = %d\n",screenModeRequester->sm_Height);
        printf("sm_InfoOpened = %s\n",screenModeRequester->sm_InfoOpened?"TRUE":"FALSE");
        printf("sm_InfoLeftEdge = %d\n",screenModeRequester->sm_InfoLeftEdge);
        printf("sm_InfoTopEdge = %d\n",screenModeRequester->sm_InfoTopEdge);
        printf("sm_InfoWidth = %d\n",screenModeRequester->sm_InfoWidth);
        printf("sm_InfoHeight = %d\n",screenModeRequester->sm_InfoHeight);
        printf("sm_UserData = $%08lx\n",screenModeRequester->sm_UserData);

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

/****** filerequester/intuiMsgFunc ******************************************
*
*   NAME
*       intuiMsgFunc -- IntuiMessage hook function
*
*   SYNOPSIS
*       success=intuiMsgFunc(hook,hookScreenModeRequester,intuiMessage);
*
*       ULONG intuiMsgFunc(struct Hook *hook,
*           struct ScreenModeRequester *hookScreenModeRequester,
*           struct IntuiMessage *intuiMessage);
*
*   FUNCTION
*       IntuiMessage hook function. Outputs arguments to debugging terminal
*       on serial port and counts hits, bad hooks, bad objects, and bad messages.
*
*   INPUTS
*       hook -- hook structure originating call
*       hookScreenModeRequester -- object originating call
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
    register __a2 struct ScreenModeRequester *hookScreenModeRequester,
    register __a1 struct IntuiMessage *intuiMessage)
{

    /* Output debugging message */
#ifdef DEBUG
    kprintf("intuiMsgFunc\n");
    kprintf("   hook=$%08lx\n",hook);
    kprintf("   hookScreenModeRequester=$%08lx\n",hookScreenModeRequester);
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

    /* If screen mode requester bad ... */
    if (hookScreenModeRequester!=screenModeRequester) {

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

/****** screenmoderequester/filterFunc ******************************************
*
*   NAME
*       filterFunc -- filter hook
*
*   SYNOPSIS
*       accept=filterFunc(hook,hookScreenModeRequester,modeID);
*
*       ULONG filterFunc(struct Hook *hook,
*           struct ScreenModeRequester *hookScreenModeRequester,
*           ULONG modeID);
*
*   FUNCTION
*       Filter hook function. Outputs arguments to debugging terminal
*       on serial port and counts hits, bad hooks, bad objects, and bad messages.
*
*       Filters out any screen mode with name not matching pattern in
*       global variable filterPattern.
*
*   INPUTS
*       hook -- hook originating call
*       modeID -- object originating call
*       hookFileRequester -- fille requester originating call
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
    register __a2 struct ScreenModeRequester *hookScreenModeRequester,
    register __a1 ULONG modeID)
{

    DisplayInfoHandle displayInfoHandle;
    struct NameInfo nameInfo;

    BOOL accept;

#ifdef DEBUG
    /* Output debugging message */
    kprintf("filterFunc\n");
    kprintf("   hook=$%08lx\n",hook);
    kprintf("   hookScreenModeRequester=$%08lx\n",hookScreenModeRequester);
    kprintf("   modeID=$%08lx\n",modeID);
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

    /* If screen mode requester bad ... */
    if (hookScreenModeRequester!=screenModeRequester) {

#ifdef DEBUG
        /* Output to debugging terminal */
        kprintf("   Bad object\n");
#endif /* DEBUG */

        /* Increment bad object count */
        filterBadObject++;

    }

    /* Get display info; if not found ... */
    displayInfoHandle=FindDisplayInfo(modeID);
    if (!displayInfoHandle) {

#ifdef DEBUG
        kprintf("   DisplayInfoRecord for mode $%08lx not found\n",modeID);
#endif /* DEBUG */

        /* Accept (do not filter) */
        return(TRUE);

    }

    /* Get name info; if error ... */
    if (GetDisplayInfoData(displayInfoHandle,
        (UBYTE *) &nameInfo,sizeof(nameInfo),
        DTAG_NAME,NULL)==0) {

#ifdef DEBUG
        kprintf("   NameInfo for mode $%08lx not found\n",modeID);
#endif /* DEBUG */

        /* Accept (do not filter) */
        return(TRUE);

    }

    /* Accept if object matches pattern */
    accept=MatchPatternNoCase(filterPattern,nameInfo.Name);
#ifdef DEBUG
    kprintf("   %s %s\n",nameInfo.Name,accept?"accepted":"rejected");
#endif /* DEBUG */

    /* Return accept/reject */
    return((ULONG) accept);

}
