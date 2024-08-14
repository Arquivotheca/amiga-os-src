/*
    asl.library File Requester Test
    Written by:
        John J. Szucs
        Amiga Systems Section
        Product Assurance Department
        Commodore International Services Company
    Copyright © 1992 Commodore International Services Company

Template:
    EARLY/S,W=WINDOW/S,S=SCREEN/S,PSN=PUBSCREENNAME/K,PI=PRIVATEIDCMP/S,IMF=INTUIMSGFUNC/S,SW=SLEEPWINDOW/S,UD=USERDATA/S,TF=TYPEFACE/K,TS=TYPESIZE/N/K,B=BOLD/S,U=UNDERLINED/S,I=ITALIC/S,L=LOCALE/K,TT=TITLETEXT/K,PT=POSITIVETEXT/K,NT=NEGATIVETEXT/K,IL=INITIALLEFT/N/K,IT=INITIALTOP/N/K,IW=INITIALWIDTH/N/K,IH=INITIALHEIGHT/N/K,IF=INITIALFILE/K,ID=INITIALDRAWER/K,IP=INITIALPATTERN/K,F1=FLAGS1/N,F2=FLAGS2/N,DSM=DOSAVEMODE/S,DMS=DOMULTISELECT/S,DP=DOPATTERNS/S,DO=DRAWERSONLY/S,FF=FILTERFUNC/S,RI=REJECTICONS/S,RP=REJECTPATTERN/K,AP=ACCEPTPATTERN/K,FD=FILTERDRAWERS/S

Usage:
    FileRequester [EARLY] [WINDOW] [SCREEN] [PUBSCREENNAME <name>]
        [PRIVATEIDCMP] [INTUIMSGFUNC] [SLEEPWINDOW] [USERDATA]
        [TYPEFACE <type face>] [TYPESIZE <type size>] [BOLD] [UNDERLINED] [ITALIC]
        [LOCALE <locale>] [TITLETEXT <title text>] [POSITIVETEXT <positive text>] [NEGATIVETEXT <negative text>]
        [INITIALLEFT <left>] [INITIALTOP <top>] [INITIALWIDTH <width>] [INITIALHEIGHT <height>]
        [INITIALFILE <file>] [INIITALDRAWER <drawer>] [INITIALPATTERN <pattern>]
        [FLAGS1 <flags 1>] [FLAGS2 <flags 2>]
        [DOSAVEMODE] [DOMULTISELECT] [DOPATTERNS] [DRAWERSONLY]
        [FILTERFUNC <filter pattern>] [REJECTICONS]
        [REJECTPATTERN <reject pattern>] [ACCEPTPATTERN <accept pattern>] [FILTERDRAWERS]

    EARLY           -   Use tags in AllocAslRequest(); if not specified,
                        tags are used in AslRequest()
    WINDOW          -   Open window
    SCREEN          -   Open screen
    PUBSCREENNAME   -   Create and open on public screen with name <name>
    PRIVATEIDCMP    -   Allocate private IDCMP
    INTUIMSGFUNC    -   Test IntuiMessage function hook;
                        outputs to debugging terminal on serial.device unit 0;
                        counts hits, bad hooks, bad objects, and bad messages
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
    INITIALFILE <file>     - Initial file selection of <file>
    INITIALDRAWER <drawer>  - Initial drawer selection of <drawer>
    INITIALPATTERN <pattern> - Initial pattern of <pattern>
    FLAGS1 <flags 1> -  Value for ASLFR_Flags1 tag;
                        see asl.library/AslRequest() AutoDocs for details
    FLAGS2 <flags 2> -  Value for ASLFR_Flags2 tag;
                        see asl.library/AslRequest() AutoDocs for details
    DOSAVEMODE      -   Save mode
    DOMULTISELECT   -   Multi-select support
    DOPATTERNS      -   Pattern gadget support
    DRAWERSONLY     -   Drawers only
    FILTERFUNC <pattern>   -   Test filter function hook; accept only files
                               matching <pattern>
    REJECTICONS     -   Reject icons
    REJECTPATTERN   -   Reject files matching <reject pattern>
    ACCEPTPATTERN   -   Accept files matching <accept pattern>
    FILTERDRAWERS   -   Process reject and accept patterns and user-specified
                        pattern for drawers

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

#define FILENAME_LENGTH 108 /* Maximum length of filename */

/* Application */
#define APP_NAME "FileRequester" /* Name of application */
#define APP_USER_DATA 0xC0EDBABE /* Value for user data field of ASL file requester */

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
#define SCREEN_TITLE "asl.library File Requester Test Screen" /* Title */

/* Window */
#define WINDOW_LEFT 160 /* Left edge */
#define WINDOW_TOP 100 /* Top edge */
#define WINDOW_WIDTH 320 /* Width */
#define WINDOW_HEIGHT 200 /* Height */
#define WINDOW_TITLE "asl.library File Requester Test Window" /* Title */

/* Command line interface */
#define CLI_TEMPLATE_BASE "EARLY/S,W=WINDOW/S,S=SCREEN/S,PSN=PUBSCREENNAME/K,PI=PRIVATEIDCMP/S,IMF=INTUIMSGFUNC/S,SW=SLEEPWINDOW/S,UD=USERDATA/S,TF=TYPEFACE/K,TS=TYPESIZE/N/K,B=BOLD/S,U=UNDERLINED/S,I=ITALIC/S,L=LOCALE/K,TT=TITLETEXT/K,PT=POSITIVETEXT/K,NT=NEGATIVETEXT/K,IL=INITIALLEFT/N/K,IT=INITIALTOP/N/K,IW=INITIALWIDTH/N/K,IH=INITIALHEIGHT/N/K"
#define CLI_TEMPLATE_EXTENDED "IF=INITIALFILE/K,ID=INITIALDRAWER/K,IP=INITIALPATTERN/K,F1=FLAGS1/N,F2=FLAGS2/N,DSM=DOSAVEMODE/S,DMS=DOMULTISELECT/S,DP=DOPATTERNS/S,DO=DRAWERSONLY/S,FF=FILTERFUNC/K,RI=REJECTICONS/S,RP=REJECTPATTERN/K,AP=ACCEPTPATTERN/K,FD=FILTERDRAWERS/S"
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

#define CLI_INITIALFILE 21
#define CLI_INITIALDRAWER 22
#define CLI_INITIALPATTERN 23

#define CLI_FLAGS1 24
#define CLI_FLAGS2 25

#define CLI_DOSAVEMODE 26
#define CLI_DOMULTISELECT 27
#define CLI_DOPATTERNS 28
#define CLI_DRAWERSONLY 29

#define CLI_FILTERFUNC 30

#define CLI_REJECTICONS 31

#define CLI_REJECTPATTERN 32
#define CLI_ACCEPTPATTERN 33

#define CLI_FILTERDRAWERS 34

#define CLI_OPTIONS 35

/* asl.library */
#define ASL_TAG_COUNT 256       /* Maximum number of tags */

/*
 * Globals
 */

struct Library *GfxBase=NULL; /* graphics.library base */
struct Library *IntuitionBase=NULL; /* intuition.library base */
struct Library *DiskfontBase=NULL; /* diskfont.library base */

struct Library *LocaleBase=NULL; /* locale.library base */
struct Library *AslBase=NULL; /* asl.library base */

struct RDArgs *rdArgs=NULL; /* RDArgs control structure */

struct Screen *screen=NULL; /* Screen */
struct Window *window=NULL; /* Window */

struct TextFont *textFont=NULL; /* Text font */

struct FileRequester *fileRequester=NULL; /* File requester */

struct Locale *locale=NULL; /* Locale */

UBYTE filterPattern[FILENAME_LENGTH]; /* Parsed pattern for filter function */

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
    register __a2 struct FileRequester *hookFileRequester,
    register __a1 struct IntuiMessage *intuiMessage);
ULONG  __saveds __asm filterFunc(register __a0 struct Hook *hook,
    register __a2 struct FileRequester *hookFileRequester,
    register __a1 struct AnchorPath *anchorPath);

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
*       appendTagItem(tagArray,ASLFR_Window,(ULONG) window);
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

/****** filerequester/shutdown ******************************************
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
     * File requester close
     */

    /* Free file requester */
    if (fileRequester) {
        FreeAslRequest(fileRequester);
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

    BOOL result; /* Result of file requester */
    LONG error; /* Error */

    int argLoop; /* Argument loop */

    static UBYTE lockName[FILENAME_LENGTH]; /* Filename buffer */

    static UBYTE acceptPatternParsed[FILENAME_LENGTH]; /* Parsed accept pattern */
    static UBYTE rejectPatternParsed[FILENAME_LENGTH]; /* Parsed reject pattern */

    static UBYTE template[CLI_TEMPLATE_LENGTH]; /* Full template */

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

    /* ASLFR_Window */
    if (window) {
        appendTagItem(tagArray,ASLFR_Window,(ULONG) window);
    }

    /* ASLFR_PubScreenName */
    if (args[CLI_PUBSCREENNAME]) {
        appendTagItem(tagArray,ASLFR_PubScreenName,args[CLI_PUBSCREENNAME]);
    }

    /* ASLFR_Screen */
    if (screen) {
        appendTagItem(tagArray,ASLFR_Screen,(ULONG) screen);
    }

    /* ASLFR_PrivateIDCMP */
    if (args[CLI_PRIVATEIDCMP]) {
        appendTagItem(tagArray,ASLFR_PrivateIDCMP,TRUE);
    }

    /* ASLFR_IntuiMsgFunc */
    if (args[CLI_INTUIMSGFUNC]) {
        appendTagItem(tagArray,ASLFR_IntuiMsgFunc,(ULONG) &intuiMsgHook);
    }

    /* ASLFR_SleepWindow */
    if (args[CLI_SLEEPWINDOW]) {
        appendTagItem(tagArray,ASLFR_SleepWindow,TRUE);
    }

    /* ASLFR_UserData */
    if (args[CLI_USERDATA]) {
        appendTagItem(tagArray,ASLFR_UserData,APP_USER_DATA);
    }

    /* ASLFR_TextAttr */
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

        /* Open font to force load from disk */
        textFont=OpenDiskFont(&textAttr);
        if (!textFont) {
            printf("Error opening font\n");
            shutdown(RETURN_FAIL);
        }

        /* Append ASLFR_TextAttr tag */
        appendTagItem(tagArray,ASLFR_TextAttr,(ULONG) &textAttr);

    }

    /* ASLFR_Locale */
    if (args[CLI_LOCALE]) {

        /* Open locale */
        locale=OpenLocale((STRPTR) args[CLI_LOCALE]);
        if (!locale) {
            printf("Error opening locale %s\n",args[CLI_LOCALE]);
        }

        /* Append locale tag */
        appendTagItem(tagArray,ASLFR_Locale,(ULONG) locale);

    }

    /* ASLFR_TitleText */
    if (args[CLI_TITLETEXT]) {
        appendTagItem(tagArray,ASLFR_TitleText,args[CLI_TITLETEXT]);
    }

    /* ASLFR_PositiveText */
    if (args[CLI_POSITIVETEXT]) {
        appendTagItem(tagArray,ASLFR_PositiveText,args[CLI_POSITIVETEXT]);
    }

    /* ASLFR_NegativeText */
    if (args[CLI_NEGATIVETEXT]) {
        appendTagItem(tagArray,ASLFR_NegativeText,args[CLI_NEGATIVETEXT]);
    }

    /* ASLFR_InitialLeftEdge */
    if (args[CLI_INITIALLEFT]) {
        appendTagItem(tagArray,ASLFR_InitialLeftEdge,*((LONG *) args[CLI_INITIALLEFT]));
    }

    /* ASLFR_InitialTopEdge */
    if (args[CLI_INITIALTOP]) {
        appendTagItem(tagArray,ASLFR_InitialTopEdge,*((LONG *) args[CLI_INITIALTOP]));
    }

    /* ASLFR_InitialWidth */
    if (args[CLI_INITIALWIDTH]) {
        appendTagItem(tagArray,ASLFR_InitialWidth,*((LONG *) args[CLI_INITIALWIDTH]));
    }

    /* ASLFR_InitialHeight */
    if (args[CLI_INITIALHEIGHT]) {
        appendTagItem(tagArray,ASLFR_InitialHeight,*((LONG *) args[CLI_INITIALHEIGHT]));
    }

    /* ASLFR_InitialFile */
    if (args[CLI_INITIALFILE]) {
        appendTagItem(tagArray,ASLFR_InitialFile,args[CLI_INITIALFILE]);
    }

    /* ASLFR_InitialDrawer */
    if (args[CLI_INITIALDRAWER]) {
        appendTagItem(tagArray,ASLFR_InitialDrawer,args[CLI_INITIALDRAWER]);
    }

    /* ASLFR_InitialPattern */
    if (args[CLI_INITIALPATTERN]) {
        appendTagItem(tagArray,ASLFR_InitialPattern,args[CLI_INITIALPATTERN]);
    }

    /* ASLFR_Flags1 */
    if (args[CLI_FLAGS1]) {
        appendTagItem(tagArray,ASLFR_Flags1,*((LONG *) args[CLI_FLAGS1]));
    }

    /* ASLFR_Flags2 */
    if (args[CLI_FLAGS2]) {
        appendTagItem(tagArray,ASLFR_Flags2,*((LONG *) args[CLI_FLAGS2]));
    }

    /* ASLFR_DoSaveMode */
    if (args[CLI_DOSAVEMODE]) {
        appendTagItem(tagArray,ASLFR_DoSaveMode,TRUE);
    }

    /* ASLFR_DoMultiSelect */
    if (args[CLI_DOMULTISELECT]) {
        appendTagItem(tagArray,ASLFR_DoMultiSelect,TRUE);
    }

    /* ASLFR_DoPatterns */
    if (args[CLI_DOPATTERNS]) {
        appendTagItem(tagArray,ASLFR_DoPatterns,TRUE);
    }

    /* ASLFR_DrawersOnly */
    if (args[CLI_DRAWERSONLY]) {
        appendTagItem(tagArray,ASLFR_DrawersOnly,TRUE);
    }

    /* ASFLR_FilterFunc */
    if (args[CLI_FILTERFUNC]) {

        /* Parse filter pattern */
        if (ParsePatternNoCase((STRPTR) args[CLI_FILTERFUNC],
            filterPattern,FILENAME_LENGTH)==-1) {
            printf("Error parsing filter pattern\n");
            shutdown(RETURN_FAIL);
        }

        /* Append ASLFR_FilterFunc tag */
        appendTagItem(tagArray,ASLFR_FilterFunc,(ULONG) &filterHook);

    }

    /* ASLFR_RejectIcons */
    if (args[CLI_REJECTICONS]) {
        appendTagItem(tagArray,ASLFR_RejectIcons,TRUE);
    }

    /* ASLFR_RejectPattern */
    if (args[CLI_REJECTPATTERN]) {

        /* Parse reject pattern */
        if (ParsePatternNoCase((STRPTR) args[CLI_REJECTPATTERN],rejectPatternParsed,FILENAME_LENGTH)==-1L) {
            printf("Error in ParsePatternNoCase()\n");
            shutdown(RETURN_FAIL);
        }

        /* Append ASLFR_RejectPattern tag */
        appendTagItem(tagArray,ASLFR_RejectPattern,(ULONG) rejectPatternParsed);
    }

    /* ASLFR_AcceptPattern */
    if (args[CLI_ACCEPTPATTERN]) {

        /* Parse accept pattern */
        if (ParsePatternNoCase((STRPTR) args[CLI_ACCEPTPATTERN],acceptPatternParsed,FILENAME_LENGTH)==-1L) {
            printf("Error in ParsePatternNoCase()\n");
            shutdown(RETURN_FAIL);
        }

        /* Append ASLFR_AcceptPattern tag */
        appendTagItem(tagArray,ASLFR_AcceptPattern,(ULONG) acceptPatternParsed);

    }

    /* ASLFR_FilterDrawers */
    if (args[CLI_FILTERDRAWERS]) {
        appendTagItem(tagArray,ASLFR_FilterDrawers,TRUE);
    }

    /*
     * Open file requester
     */

    /* Allocate file requester; use tag array if EARLY specified */
    fileRequester=AllocAslRequest(ASL_FileRequest,args[CLI_EARLY]?tagArray:NULL);
    if (!fileRequester) {
        printf("Error allocating file requester\n");
        destroyTagArray(tagArray);
        shutdown(RETURN_FAIL);
    }

    /* Open file requester */
    result=AslRequest(fileRequester,args[CLI_EARLY]?NULL:tagArray);

    /* Read secondary error code */
    error=IoErr();

    /* Output success/failure */
    printf("AslRequest() return code %s\n",result?"TRUE":"FALSE");

    /* If failure ... */
    if (!result) {

        /* Output diagnostics */
        switch (error) {

            case 0:
                printf("File requester cancelled by user\n");
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
     * Dump FileRequester structure
     */

    /* If success ... */
    if (result) {

        /* Dump FileRequester structure */
        printf("fr_File=%s\n",fileRequester->fr_File);
        printf("fr_Drawer=%s\n",fileRequester->fr_Drawer);

        printf("fr_LeftEdge=%d\n",fileRequester->fr_LeftEdge);
        printf("fr_TopEdge=%d\n",fileRequester->fr_TopEdge);
        printf("fr_Width=%d\n",fileRequester->fr_Width);
        printf("fr_Height=%d\n",fileRequester->fr_Height);

        printf("fr_NumArgs=%d\n",fileRequester->fr_NumArgs);

        for (argLoop=0;argLoop<fileRequester->fr_NumArgs;argLoop++) {

            if (!NameFromLock(fileRequester->fr_ArgList[argLoop].wa_Lock,
                lockName,FILENAME_LENGTH)) {
                printf("Error in NameFromLock()\n");
                shutdown(RETURN_FAIL);
            }

            printf("wa_Lock = %s ($%08lx)\n",lockName,
                fileRequester->fr_ArgList[argLoop]);

            printf("wa_Name = %s\n",fileRequester->fr_ArgList[argLoop].wa_Name);

        }

        printf("fr_UserData=$%08lx\n",fileRequester->fr_UserData);

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
*       success=intuiMsgFunc(hook,hookFileRequester,intuiMessage);
*
*       ULONG intuiMsgFunc(struct Hook *hook,
*           struct FileRequester *hookFileRequester,
*           struct IntuiMessage *intuiMessage);
*
*   FUNCTION
*       IntuiMessage hook function. Outputs arguments to debugging terminal
*       on serial port and counts hits, bad hooks, bad objects, and bad messages.
*
*   INPUTS
*       hook -- hook structure originating call
*       hookFileRequester -- object originating call
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
    register __a2 struct FileRequester *hookFileRequester,
    register __a1 struct IntuiMessage *intuiMessage)
{

    /* Output debugging message */
#ifdef DEBUG
    kprintf("intuiMsgFunc\n");
    kprintf("   hook=$%08lx\n",hook);
    kprintf("   hookFileRequester=$%08lx\n",hookFileRequester);
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

    /* If file requester bad ... */
    if (hookFileRequester!=fileRequester) {

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

/****** filerequester/filterFunc ******************************************
*
*   NAME
*       filterFunc -- filter hook
*
*   SYNOPSIS
*       accept=filterFunc(hook,hookFileRequester,anchorPath);
*
*       ULONG filterFunc(struct Hook *hook,
*           struct FileRequester *hookFileRequester,
*           struct AnchorPath *anchorPath);
*
*   FUNCTION
*       Filter hook function. Outputs arguments to debugging terminal
*       on serial port and counts hits, bad hooks, bad objects, and bad messages.
*
*       Filters out any file not matching the pattern in the global
*       variable filterPattern.
*
*   INPUTS
*       hook -- hook originating call
*       anchorPath -- anchor path of file to filter
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
    register __a2 struct FileRequester *hookFileRequester,
    register __a1 struct AnchorPath *anchorPath)
{

    BOOL accept;

#ifdef DEBUG
    /* Output debugging message */
    kprintf("filterFunc\n");
    kprintf("   hook=$%08lx\n",hook);
    kprintf("   hookFileRequester=$%08lx\n",hookFileRequester);
    kprintf("   anchorPath=$%08lx\n",anchorPath);
    kprintf("   anchorPath->ap_Info.fib_Filename=%s\n",anchorPath->ap_Info.fib_FileName);
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

    /* If file requester bad ... */
    if (hookFileRequester!=fileRequester) {

#ifdef DEBUG
        /* Output to debugging terminal */
        kprintf("   Bad object\n");
#endif /* DEBUG */

        /* Increment bad object count */
        filterBadObject++;

    }

    /* Accept if object matches pattern */
    accept=MatchPatternNoCase(filterPattern,anchorPath->ap_Info.fib_FileName);
#ifdef DEBUG
    kprintf("    %s %s\n",anchorPath->ap_Info.fib_FileName,accept?"accepted":"rejected");
#endif /* DEBUG */

    /* Return accept/reject */
    return((ULONG) accept);

}
