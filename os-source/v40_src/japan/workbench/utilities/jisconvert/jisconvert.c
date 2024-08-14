
#include <ctype.h>
#include <exec/types.h>
#include <exec/memory.h>
#include <intuition/intuition.h>
#include <dos/dos.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>	/* temp */

#include "JISConvert.h"
//#include "JISConvert_rev.h"

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/utility_protos.h>
#include <clib/graphics_protos.h>
#include <clib/diskfont_protos.h>
#include <clib/asl_protos.h>
#include "clib/dos_protos.h"
#include <clib/icon_protos.h>
#include <clib/locale_protos.h>
#include <clib/macros.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/locale_pragmas.h>


#include <workbench/startup.h>
#include <workbench/icon.h>
#include <workbench/workbench.h>

#include <libraries/locale.h>

#define CATCOMP_NUMBERS
#include "jisconvert_strings.h"


extern struct Library *SysBase;
struct Library *LocaleBase;


STRPTR __asm GetString(register __a0 struct LocaleInfo *li,
                       register __d0 LONG stringNum);

#include "jcc_protos.h"
#include "pragmas/jcc_pragmas.h"

char vers[] = VERSTAG;

struct Library *IntuitionBase;
struct Library *IconBase;
struct Library *GadToolsBase;
struct Library *UtilityBase;
struct Library *GfxBase;
struct Library *DiskfontBase;
struct Library *AslBase;
struct Library *DOSBase;

struct LocaleInfo li;

struct Library *JCCBase = NULL;
struct JConversionCodeSet *jcc = NULL;
struct JConversionCodeSet *jccID = NULL;


struct IntuiText BodyText = {0,1,JAM2,20,8,NULL,NULL,NULL};
struct IntuiText NegText  = {0,1,JAM2, 6,4,NULL,NULL,NULL};


struct GeneralData GData;               /* general data */
struct GeneralData *data = &GData;      /* pointer to GData */

struct strbuffer strbuf;                /* string buffers for status strings */

struct Gadget *jgadgets[9];		/* pointers to the first gadgets list */
struct Gadget *j2gadgets[4];		/* pointers to the 2nd gadgets list */

struct TextAttr coral16 =		/* text attribute used */
    {
    (STRPTR)"coral.font",
    16,
    FS_NORMAL,
    0x0
    };

int ArgCount;



LONG OutputCode = CTYPE_EUC;		/* Output code type */
LONG InputCode = CTYPE_UNKNOWN;		/* input code type */
BOOL DetInCode = TRUE;			/* try to detect input code */

LONG ctype[4] = {CTYPE_EUC, CTYPE_SJIS, CTYPE_NEWJIS, CTYPE_OLDJIS};
char  *ext[] = {".euc", ".sjs",".new", ".old" };

UWORD ext_index = EUC;
UWORD filter_flag = 1;
UBYTE err_convert;

struct FileRequester *in_req, *out_req;	/* pointers to source and target file requesters */

UBYTE *refline[] = {NULL, NULL, NULL};

VOID main (int argc, char **argv)
    {
    struct strbuffer *sbuf;
    struct WBStartup *WBenchMsg;
    struct DiskObject *diskobj = NULL;
    char  *str, *wbname;

    char *CodeSel[] = {"EUC", "SJIS", "NEWJIS", "OLDJIS"};

    int i;


    memset (data,0x00,sizeof(struct GeneralData));

    li.li_Catalog = NULL;

    /* First open locale library so messeges can be displayed */
    if (LocaleBase = OpenLibrary("locale.library",39L))
	{
        li.li_LocaleBase = LocaleBase;
        li.li_Catalog    = OpenCatalogA(NULL,"jisconvert.catalog",NULL);
	}


    BodyText.IText = (UBYTE *)GetString(&li,TEXT_NORELEASE2);
    NegText.IText = (UBYTE *)GetString(&li,TEXT_OK);

    sbuf = &strbuf;

    ArgCount = argc;

//    printf ("argc = %d\n", argc);		/* debug */
//    printf ("arg = %s\n", argv[1]);		/* debug */


    /* Pars command line */

    if ( (ArgCount > 9) || (ArgCount == 2 && *argv[1] == '?') )
	{
	DispHelp();
	exit (RETURN_FAIL);
	}

    if (ArgCount > 1)
	{
	while (--argc > 0)
	    {
            argv++;
	    if ( !(stricmp(*argv, "INCODE")) )
		{
		argv++;
		argc--;
		for (i=0; i<4; i++)
		    {
		    if ( !(stricmp (*argv, CodeSel[i])) )
			{
			DetInCode = FALSE;
			InputCode = ctype[i];
			}
		    }
		}

	    else if ( !(stricmp(*argv, "OUTCODE")) )
		{
		argv++;
		argc--;
		for (i=0; i<4; i++)
		    {
		    if ( !(stricmp (*argv, CodeSel[i])) )
			{
			OutputCode = ctype[i];
//			printf ("output code = %lx\n", OutputCode); /* debug */
			ext_index = i;
			}
		    }
		}

	    else if ( !(stricmp(*argv, "FILTER")) )
		{
		argv++;
		argc--;
		if ( !(stricmp(*argv, "ON")) )
		    filter_flag = 1;
		if ( !(stricmp(*argv, "OFF")) )
		    filter_flag = 0;
		}
	    else if ( !(stricmp(*argv, "EUC")) || !(stricmp(*argv, "SJIS")) ||
			!(stricmp(*argv, "NEWJIS")) || !(stricmp(*argv, "OLDJIS")) )
		{
		for (i=0; i<4; i++)
		    {
		    if ( !(stricmp(*argv, CodeSel[i])) )
			{
			if (OutputCode == CTYPE_UNKNOWN)
			    {
			    OutputCode = ctype[i];
			    ext_index = i;
			    }
			else
			    {
			    InputCode = OutputCode;
			    OutputCode = ctype[i];
			    DetInCode = FALSE;
			    }
			}
		    }
		}
	    else if ( !(stricmp(*argv, "ON")) )
		filter_flag = 1;
	    else if ( !(stricmp(*argv, "OFF")) )
		filter_flag = 0;

//	    else if (sbuf->fstr_in[0] == 0)
	    else if ( !(*(sbuf->fstr_in)) )
		{
		strcpy (sbuf->fstr_in, *argv);
//		printf ("inarg = %s\n",*argv);     /* debug */
//		printf ("argc = %d\n",argc);	/* debug */
		}
//	    else if (sbuf->fstr_out[0] == 0)
	    else if ( !(*(sbuf->fstr_out)) )
		strcpy (sbuf->fstr_out, *argv);
	    else
		{
//		printf ("infile: %s\n",sbuf->fstr_in);  /*debug */
//		printf ("inarg = %s\n",*argv);     /* debug */

		printf ("%s\n", GetString(&li,TEXT_ERRBADARGS) );
	    	DispHelp();
		exit (RETURN_FAIL);
		}

	    }   /* while */

//	if (sbuf->fstr_in[0] == 0)
	if ( !(*(sbuf->fstr_in)) )
	    {
	    printf ("%s\n", GetString(&li,TEXT_ERRNOINPUT));
	    DispHelp();
	    exit (RETURN_FAIL);
	    }

//	if (sbuf->fstr_out[0] == 0)
	if ( !(*(sbuf->fstr_out)) )
	    {
	    strcpy (sbuf->fstr_out, sbuf->fstr_in);
	    strcat (sbuf->fstr_out, ext[ext_index]);
	    }

        }


//  printf ("argv = %s\n", argv[0]); /* debug */

    /* Open all the libraries needed */

    if (ArgCount <= 1)
	{
	if(!(IntuitionBase = OpenLibrary((UBYTE *)"intuition.library", 39L)))
	    {
	    if(IntuitionBase = OpenLibrary((UBYTE *)"intuition.library", 0L))
		{
		AutoRequest(NULL, &BodyText, NULL, &NegText, 0, 0, 320, 80);
		CloseLibrary(IntuitionBase);
		}
	    cleanexit(data, RETURN_FAIL);
	    }

	if(!(GadToolsBase = OpenLibrary((UBYTE *)"gadtools.library", 39L)))
	    {
	    TM_Request(NULL, (UBYTE *)GetString(&li,TEXT_ERROR), (UBYTE *)GetString(&li,TEXT_NOLIBRARY), (UBYTE *)GetString(&li,TEXT_ABORT), NULL, "gadtools.library V39");
	    cleanexit(data, RETURN_FAIL);
	    }

	if(!(UtilityBase = OpenLibrary((UBYTE *)"utility.library", 39L)))
	    {
	    TM_Request(NULL, (UBYTE *)GetString(&li,TEXT_ERROR), (UBYTE *)GetString(&li,TEXT_NOLIBRARY), (UBYTE *)GetString(&li,TEXT_ABORT), NULL, "utility.library V39");
	    cleanexit(data, RETURN_FAIL);
	    }

	if(!(GfxBase = OpenLibrary((UBYTE *)"graphics.library", 39L)))
	    {
	    TM_Request(NULL, (UBYTE *)GetString(&li,TEXT_ERROR), (UBYTE *)GetString(&li,TEXT_NOLIBRARY), (UBYTE *)GetString(&li,TEXT_ABORT), NULL, "graphics.library V39");
	    cleanexit(data, RETURN_FAIL);
	    }

	if(!(DiskfontBase = OpenLibrary((UBYTE *)"diskfont.library", 39L)))
	    {
	    TM_Request(NULL, (UBYTE *)GetString(&li,TEXT_ERROR), (UBYTE *)GetString(&li,TEXT_NOLIBRARY), (UBYTE *)GetString(&li,TEXT_ABORT), NULL, "diskfont.library V39");
	    cleanexit(data, RETURN_FAIL);
	    }

	if( !(AslBase = OpenLibrary ("asl.library",39L)) )
	    {
	    TM_Request(NULL, (UBYTE *)GetString(&li,TEXT_ERROR), (UBYTE *)GetString(&li,TEXT_NOLIBRARY), (UBYTE *)GetString(&li,TEXT_ABORT), NULL, "asl.library V39");
	    cleanexit(data, RETURN_FAIL);
            }

	IconBase = OpenLibrary("icon.library",39L);


	}


    if( !(DOSBase = OpenLibrary ("dos.library",39L)) )
	{
	TM_Request(NULL, (UBYTE *)GetString(&li,TEXT_ERROR), (UBYTE *)GetString(&li,TEXT_NOLIBRARY), (UBYTE *)GetString(&li,TEXT_ABORT), NULL, "dos.library V39");
	cleanexit(data, RETURN_FAIL);
        }

    if  ( !(JCCBase = OpenLibrary("jcc.library", 0L) ) )
	{
	TM_Request(NULL, (UBYTE *)GetString(&li,TEXT_ERROR), (UBYTE *)GetString(&li,TEXT_NOLIBRARY), (UBYTE *)GetString(&li,TEXT_ABORT), NULL, "jcc.library V39");
	cleanexit(data, RETURN_FAIL);
        }

    if ( !(jcc = AllocConversionHandle(TAG_DONE) ) )
	{
	TM_Request(NULL, (UBYTE *)GetString(&li,TEXT_ERROR), (UBYTE *)GetString(&li,TEXT_NOJCCHANDLE), (UBYTE *)GetString(&li,TEXT_ABORT), NULL, NULL);
	cleanexit(data, RETURN_FAIL);
        }

    if ( !(jccID = AllocConversionHandle(TAG_DONE) ) )	/* wc */
	{
	TM_Request(NULL, (UBYTE *)GetString(&li,TEXT_ERROR), (UBYTE *)GetString(&li,TEXT_NOJCCHANDLE), (UBYTE *)GetString(&li,TEXT_ABORT), NULL, NULL);
	cleanexit(data, RETURN_FAIL);
        }



    if (ArgCount <= 1)

	{

	/* Open font */

	if( !(data->TextFont = OpenDiskFont(&coral16)) )
	    {
	    TM_Request(NULL, (UBYTE *)GetString(&li,TEXT_ERROR), (UBYTE *)GetString(&li,TEXT_NOFONT), (UBYTE *)GetString(&li,TEXT_ABORT), NULL, "coral.font", 16);
            cleanexit (data, RETURN_FAIL);
	    }

	/* Get WB startup stuff */

        WBenchMsg = (struct WBStartup *)argv;
	if (ArgCount == 0)
	    wbname = WBenchMsg->sm_ArgList->wa_Name;
	else
	    wbname = *argv;
//	printf ("wbname = %s\n",wbname);	/* debug */

	if ( diskobj = GetDiskObject(wbname) )
	    {
	    if (str = FindToolType(diskobj->do_ToolTypes, "INCODE") )
		{
//		printf ("input code = %c", str[0]); /*debug */

		for (i=0; i<4; i++)
		    {
		    if ( !(stricmp (str, CodeSel[i])) )
			{
			DetInCode = FALSE;
			InputCode = ctype[i];
			}
		    }
		}

	    if (str = FindToolType(diskobj->do_ToolTypes, "OUTCODE") )
		{
//		printf ("     output code = %c", str[0]); /* debug */

		for (i=0; i<4; i++)
		    {
		    if ( !(stricmp (str, CodeSel[i])) )
			{
			OutputCode = ctype[i];
//			printf ("output code = %lx\n", OutputCode); /* debug */
			ext_index = i;
			}
		    }
		}


	    if (str = FindToolType(diskobj->do_ToolTypes, "FILTER") )
		{
		if ( !(stricmp(str, "ON")) )
		    {
//		    printf ("     filter flag = %s\n", str); /* debug */
		    filter_flag = 1;
		    }
		else
		    filter_flag = 0;

		}

	    FreeDiskObject (diskobj);
	    }


	/* Lock Workbench screen and get VisualInfo needed for GadTools stuff */

	if(!(OpenScreenWB(data)))
	    {
	    TM_Request(NULL, (UBYTE *)GetString(&li,TEXT_ERROR), (UBYTE *)GetString(&li,TEXT_NOSCREEN), (UBYTE *)GetString(&li,TEXT_ABORT), NULL, NULL);
	    cleanexit(data, RETURN_FAIL);
	    }

	/* Open our window */

	if(!(OpenWin(data)))
	    {
	    TM_Request(NULL, (UBYTE *)GetString(&li,TEXT_ERROR), (UBYTE *)GetString(&li,TEXT_NOWINDOW), (UBYTE *)GetString(&li,TEXT_ABORT), NULL, NULL);
	    cleanexit(data, RETURN_FAIL);
	    }
	}


    /* Allocate memory buffers for conversion */

    if ( !(data->inbuf = AllocVec (BUFSIZE+16L, MEMF_CLEAR) ) )
	{
	TM_Request (NULL, (UBYTE *)GetString(&li,TEXT_ERROR), (UBYTE *)GetString(&li,TEXT_NOMEMORY), (UBYTE *)GetString(&li,TEXT_ABORT), NULL, NULL);
	cleanexit(data, RETURN_FAIL);
	}

    if ( !(data->outbuf = AllocVec (BUFSIZE*3+16L, MEMF_CLEAR) ) )
	{
	TM_Request (NULL, (UBYTE *)GetString(&li,TEXT_ERROR), (UBYTE *)GetString(&li,TEXT_NOMEMORY), (UBYTE *)GetString(&li,TEXT_ABORT), NULL, NULL);
	cleanexit(data, RETURN_FAIL);
	}

    /* Handle window events or pars command line arguments, do conversion work */

    if (ArgCount <= 1)
	HandleEvent(data);
    else
	{
//	printf ("incode: %lx\n", InputCode);	/* debug */
//      printf ("outcode: %lx\n", OutputCode);  /* debug */
//	printf ("input file: %s\n", sbuf->fstr_in); /* debug */
//	printf ("output file: %s\n", sbuf->fstr_out); /* debug */

	if (err_convert = ConvertFile (data, sbuf, 0)  )
	    {
	    switch (err_convert)
		{
		case UNKWNCODE:
		    printf ("\n%s\n",GetString(&li,TEXT_UNKWNCODE));
                    break;
		case NONTEXT:
		    printf ("\n%s\n",GetString(&li,TEXT_NONTEXT) );
                    break;
		case ERROPENIN:
		    printf ("\n%s\n",GetString(&li,TEXT_ERROPENIN));
                    break;
		case ERROPENOUT:
		    printf ("\n%s\n",GetString(&li,TEXT_ERROPENOUT));
                    break;
		case ERRCOPY:
		    printf ("\n%s\n",GetString(&li,TEXT_ERRCOPY));
                    break;
		case ASCIITEXT:
		    printf ("\n%s\n",GetString(&li,TEXT_ERRASCII));
                    break;
		default:
		    break;
		}
	    }

	}

    /* Done */

    if (!err_convert)
	cleanexit(data, RETURN_OK);
    else
	cleanexit(data, RETURN_FAIL);
    }


/**************************************************************************/

VOID DispHelp (VOID)
    {
    printf ("\n%s", GetString(&li,H_Usage) );
    printf ("  JISConvert [[INCODE] %s]", GetString(&li, H_Incode));
    printf (" [[OUTCODE] %s]\n %17c [[FILTER] ON|OFF] ", GetString(&li,H_Outcode), ' ');

    printf ("%s\n", GetString(&li,H_IOFileName) );
    printf ("\n%5c %s \n%16c %s \n%16c %s \n%16c %s \n%16c %s\n",
		' ', GetString(&li,H_INOUT),' ', GetString(&li,H_EUC),
		' ', GetString(&li,H_SJIS), ' ',GetString(&li, H_New),
		' ', GetString(&li,H_Old) );
    }

/* -------------------------------------------------------------------- */


BOOL OpenScreenWB(struct GeneralData *data)
    {
    if(data->screen)
	{
	ScreenToFront(data->screen);
	return(TRUE);
	}
    else
	{
	if((data->screen) = LockPubScreen(NULL))
	    {
	    if((data->VisualInfo) = GetVisualInfo(data->screen, TAG_DONE))
		{
		return(TRUE);
		}
	    UnlockPubScreen(NULL, data->screen);
	    }
	}

    return(FALSE);
    }


/* -------------------------------------------------------------------- */

VOID CloseScreenWB(struct GeneralData *data)
    {
    if(data->VisualInfo)
	{
	FreeVisualInfo(data->VisualInfo);
	data->VisualInfo = NULL;
	}
    if(data->screen)
	{
	UnlockPubScreen(NULL, data->screen);
	data->screen = NULL;
	}
    }

/* -------------------------------------------------------------------- */


VOID cleanexit(struct GeneralData *data, int returnvalue)
    {
    if (data->inbuf)	FreeVec (data->inbuf);
    if (data->outbuf)	FreeVec (data->outbuf);

    if (jccID)		FreeConversionHandle (jccID);
    if (jcc)		FreeConversionHandle (jcc);

    if (JCCBase)	CloseLibrary (JCCBase);
    if (DOSBase)	CloseLibrary(DOSBase);

    if (LocaleBase)
	{
        CloseCatalog(li.li_Catalog);
        CloseLibrary(LocaleBase);
	}

    if (ArgCount <= 1)
	{
	if (data->win)	CloseWin(data);
	if (data->screen)	CloseScreenWB(data);
	if (data->TextFont)	CloseFont(data->TextFont);

//  TM_Close(TMData);

	if (IconBase)	CloseLibrary (IconBase);
	if (AslBase)	CloseLibrary (AslBase);
	if (DiskfontBase)	CloseLibrary (DiskfontBase);
	if (GfxBase)	CloseLibrary(GfxBase);
	if (UtilityBase)	CloseLibrary (UtilityBase);
	if (GadToolsBase)	CloseLibrary (GadToolsBase);
	if (IntuitionBase)	CloseLibrary (IntuitionBase);
        }

    exit(returnvalue);
    }

/* -------------------------------------------------------------------- */

/* Routine to display requesters */

LONG TM_Request(struct Window *Window, UBYTE *Title, UBYTE *TextFormat, UBYTE *GadgetFormat, ULONG *IDCMP_ptr, APTR Arg1, ...)
    {
    struct EasyStruct es;

    es.es_StructSize = sizeof(struct EasyStruct);
    es.es_Flags = 0;
    es.es_Title = Title;
    es.es_TextFormat = TextFormat;
    es.es_GadgetFormat = GadgetFormat;

    return(EasyRequestArgs(Window, &es, IDCMP_ptr, &Arg1));
    }

/* -------------------------------------------------------------------- */


BOOL OpenWin(struct GeneralData *data)

    {
    struct strbuffer *sbuf;

    int i;
    WORD WA_Zoom_win[] = {10,20,70,30};		/* define zoom size */


    struct NewMenu jmenu[] =

        {
            { NM_TITLE,  NULL, 0, 0, 0, 0, },
            {   NM_ITEM,    NULL, 0, 0, 0, 0,},
	    { NM_TITLE,  NULL, 0, 0, 0, 0, },
	    {   NM_ITEM, NULL, 0, CHECKIT, ~1, 0,},
	    {   NM_ITEM, NULL, 0, CHECKIT, ~2, 0, },
	    {   NM_ITEM, NULL, 0, CHECKIT, ~4, 0,},
	    {   NM_ITEM, NULL, 0, CHECKIT, ~8, 0,},
	    {   NM_ITEM, NULL, 0, CHECKIT, ~16, 0,},
            { NM_END,	 NULL, 0, 0, 0, 0,},
        };


    struct TagItem frtags[] =                   /* tags for ASL requesters */
	{
	ASL_Hail,       (ULONG) "",
        ASL_Height,     FR_HEIGHT,
	ASL_Width,      FR_WIDTH,
	ASL_LeftEdge,   FR_LEFT,
	ASL_TopEdge,    FR_TOP,
	ASL_FuncFlags,	FILF_PATGAD,
	ASLFR_TextAttr,	(ULONG) &coral16,
        TAG_DONE
	};

    sbuf = &strbuf;

    jmenu[0].nm_Label = GetString(&li,MENUTITLE_project);
    jmenu[1].nm_Label = GetString(&li,MENUITEM_Quit);
    jmenu[2].nm_Label = GetString(&li,MENUTITLE_incode);
    jmenu[3].nm_Label = GetString(&li,MENUITEM_AutoDet);
//    printf("input code = %lx\n",InputCode);     /* debug */
    jmenu[4].nm_Label = GetString(&li,MENUSUB_EUC);
    jmenu[5].nm_Label = GetString(&li,MENUSUB_SJIS);
    jmenu[6].nm_Label = GetString(&li,MENUSUB_NEWJIS);
    jmenu[7].nm_Label = GetString(&li,MENUSUB_OLDJIS);

    frtags[1].ti_Data += data->screen->BarHeight;

    if (DetInCode)
	jmenu[3].nm_Flags |= CHECKED;
    else
	{
	for (i=0; i<4; i++)
	    {
	    if (InputCode == ctype[i])
		jmenu[4+i].nm_Flags |= CHECKED;
	    }
	}

    if(data->win)
	{
	WindowToFront(data->win);
	ActivateWindow(data->win);
	return(TRUE);
	}
    else
	{

	if ( CreateGadgetList1 (data) )
	    {
	    if((data->win) = OpenWindowTags(NULL,
					WA_PubScreen, data->screen,
					WA_Title, GetString(&li,WINDOWTEXT_jwin),
					WA_Gadgets, data->G1_FstGadget,
					WA_Left, 0,
					WA_Top, 0,
					WA_InnerWidth, WINDOW_W,
					WA_InnerHeight, WINDOW_H,
					WA_MinWidth, 70,
					WA_MinHeight, 30,
					WA_MaxWidth, -1,
				 	WA_MaxHeight, -1,
					WA_Zoom, WA_Zoom_win,
					WA_DragBar, TRUE,
					WA_CloseGadget, TRUE,
					WA_DepthGadget, TRUE,
					WA_Activate, TRUE,
//					WA_SimpleRefresh, TRUE,
					WA_SmartRefresh, TRUE,
					WA_IDCMP,IDCMP_REFRESHWINDOW |
					CHECKBOXIDCMP |
					BUTTONIDCMP |
					MXIDCMP |
					IDCMP_GADGETDOWN |
					IDCMP_GADGETUP |
                                        IDCMP_MENUPICK |
					IDCMP_CLOSEWINDOW,
					TAG_END) )
                {
                /* open window ok, continue */

	        GT_RefreshWindow(data->win, NULL);

		if (GetCurrentDirName (sbuf->fstr_out, 64))
		    GT_SetGadgetAttrs (jgadgets[G_OSTR], data->win,
		  			NULL, GTST_String, sbuf->fstr_out, TAG_END );


        	if (data->MStrip = CreateMenus (jmenu, TAG_END) )
		    if (LayoutMenus (data->MStrip, data->VisualInfo, GTMN_TextAttr, &coral16,  TAG_END) )
			SetMenuStrip (data->win, data->MStrip);


//        	WindowInfo_jwin.Flags |= TMWF_OPENED;


		/* Draw status box */

		DispText (data->win, GetString(&li,TEXT_PREVIEW), BBOX_LEFT + 8, BBOX_TOP -20);

		RenderBBox (data->win, BBOX_LEFT, BBOX_TOP, BBOX_W, BBOX_H,
				GT_VisualInfo, data->VisualInfo,
				GTBB_Recessed, TRUE, TAG_DONE);




		/* Allocate ASL request structures */

		in_req = AllocAslRequest (ASL_FileRequest, frtags);
		out_req = AllocAslRequest (ASL_FileRequest, frtags);
	        return(TRUE);
        	}

	    /* Open window failed, free gadgets and return with error */
	    FreeGadgets(data->G1_FstGadget);
	    }
	}
	return(FALSE);
    }

/* ---------------------------------------------------------------------- */


VOID CloseWin(struct GeneralData *data)
    {
    if (in_req)	 FreeAslRequest (in_req);
    if (out_req) FreeAslRequest (out_req);

    if (data->win)
	{
	ClearMenuStrip (data->win);
	if (data->MStrip)
	    FreeMenus (data->MStrip);

	CloseWindow(data->win);
	data->win = NULL;
	}
    if(data->G1_FstGadget)
	{
	FreeGadgets(data->G1_FstGadget);
	data->G1_FstGadget = NULL;
	}
    }

/* ---------------------------------------------------------------------- */


BOOL CreateGadgetList1 (struct GeneralData *data)

    {
    struct Gadget *gad;
    struct NewGadget ng;

    char *GTMX_Labels_code[5];
    BOOL ok;


    GTMX_Labels_code[0] = GetString(&li,GTMX_Labels_code_EUC);
    GTMX_Labels_code[1] = GetString(&li,GTMX_Labels_code_SJIS);
    GTMX_Labels_code[2] = GetString(&li,GTMX_Labels_code_NEWJIS);
    GTMX_Labels_code[3] = GetString(&li,GTMX_Labels_code_OLDJIS);
    GTMX_Labels_code[4] = NULL;

    (data->G1_FstGadget) = NULL;
    gad = CreateContext(&(data->G1_FstGadget));

    ng.ng_VisualInfo = data->VisualInfo;
    ng.ng_LeftEdge = 256;
    ng.ng_TopEdge = 78+(data->screen->BarHeight);
    ng.ng_Width = 26;
    ng.ng_Height = 20;
    ng.ng_Flags = PLACETEXT_RIGHT;
    ng.ng_TextAttr = &coral16;
    ng.ng_GadgetText = (UBYTE *) GetString(&li,GADGETTEXT_filter);
    ng.ng_GadgetID = ID_FILTER;

    if (filter_flag)
	jgadgets[G_FILTER] = gad = CreateGadget(CHECKBOX_KIND, gad, &ng, GTCB_Checked, TRUE, TAG_END);
    else
	jgadgets[G_FILTER] = gad = CreateGadget(CHECKBOX_KIND, gad, &ng, TAG_END);


    /* Gadget -- Select Input File */

    ng.ng_LeftEdge = 232;
    ng.ng_TopEdge = 20+(data->screen->BarHeight);
    ng.ng_Width = 220;
    ng.ng_Flags = PLACETEXT_IN;
    ng.ng_GadgetText = (UBYTE *)GetString(&li,GADGETTEXT_input);
    ng.ng_GadgetID = ID_INPUT;

    jgadgets[G_INPUT] = gad = CreateGadget(BUTTON_KIND, gad, &ng, TAG_END);


    /* Gadget -- Select Output File */

    ng.ng_LeftEdge = 232;
    ng.ng_TopEdge = 50+(data->screen->BarHeight);
    ng.ng_Width = 220;
    ng.ng_GadgetText = (UBYTE *)GetString(&li,GADGETTEXT_output);
    ng.ng_GadgetID = ID_OUTPUT;

    jgadgets[G_OUTPUT] = gad = CreateGadget(BUTTON_KIND, gad, &ng, TAG_END);


    /* Gadget -- Output Code */

    ng.ng_LeftEdge = 32;
    ng.ng_TopEdge = 26+(data->screen->BarHeight);
    ng.ng_Width = 18;
    ng.ng_Height = 18;
    ng.ng_Flags = PLACETEXT_RIGHT;
    ng.ng_GadgetText = (UBYTE *)GetString(&li,GADGETTEXT_code);
    ng.ng_GadgetID = ID_CODE;
    jgadgets[G_CODE] = gad = CreateGadget(MX_KIND, gad, &ng,
				GTMX_Active, ext_index,
    				GTMX_Labels, GTMX_Labels_code,
    				GTMX_Spacing, 2, TAG_END);


    /* Gadget -- text "Output Code:" */

    ng.ng_LeftEdge = 16;
    ng.ng_TopEdge = 6+(data->screen->BarHeight);
    ng.ng_Width = 88;
    ng.ng_Flags = PLACETEXT_IN | NG_HIGHLABEL;
    ng.ng_GadgetText = (UBYTE *)GetString(&li,GADGETTEXT_cstr);
    ng.ng_GadgetID = ID_CSTR;
    jgadgets[G_CSTR] = gad = CreateGadget(TEXT_KIND, gad, &ng,
    				GTTX_Text, GetString(&li,GTTX_Text_cstr), TAG_END);


    /* Gadget -- Convert */

    ng.ng_LeftEdge = 16;
    ng.ng_TopEdge = 248+(data->screen->BarHeight);
    ng.ng_Width = 72;
    ng.ng_Height = 20;
    ng.ng_Flags = PLACETEXT_IN;
    ng.ng_GadgetText = (UBYTE *)GetString(&li,GADGETTEXT_convert);
    ng.ng_GadgetID = ID_CONVERT;
    jgadgets[G_CONVERT] = gad = CreateGadget(BUTTON_KIND, gad, &ng, TAG_END);


    /* Gadget -- Quit */

    ng.ng_LeftEdge = 402;
    ng.ng_Flags = PLACETEXT_IN;
    ng.ng_GadgetText = (UBYTE *)GetString(&li,GADGETTEXT_quit);
    ng.ng_GadgetID = ID_QUIT;
    jgadgets[G_QUIT] = gad = CreateGadget(BUTTON_KIND, gad, &ng, TAG_END);


    /* Gadget -- string input */

    ng.ng_LeftEdge = 160;
    ng.ng_TopEdge = 106+(data->screen->BarHeight);
    ng.ng_Width = 312;
    ng.ng_Flags = PLACETEXT_LEFT; /* | NG_HIGHLABEL;*/
    ng.ng_GadgetText = (UBYTE *)GetString(&li,GADGETTEXT_istr);
    ng.ng_GadgetID = ID_ISSTR;
    jgadgets[G_ISTR] = gad = CreateGadget(STRING_KIND, gad, &ng, TAG_END);


    /* Gadget -- string output */

    ng.ng_TopEdge = 132+(data->screen->BarHeight);
    ng.ng_Flags = PLACETEXT_LEFT; /* | NG_HIGHLABEL;*/
    ng.ng_GadgetText = (UBYTE *)GetString(&li,GADGETTEXT_ostr);
    ng.ng_GadgetID = ID_OSSTR;
    jgadgets[G_OSTR] = gad = CreateGadget(STRING_KIND, gad, &ng, TAG_END);

    ok = (gad ? TRUE : FALSE);
    return (ok);

//    return (gad ? TRUE : FALSE);
    }


/* ---------------------------------------------------------------------- */


BOOL CreateGadgetList2 (struct GeneralData *data)
    {
    struct Gadget *gad;
    struct NewGadget ng;

    BOOL ok;

    (data->G2_FstGadget) = NULL;
    gad = CreateContext(&(data->G2_FstGadget));

    ng.ng_VisualInfo = data->VisualInfo;
    ng.ng_LeftEdge = 190;
    ng.ng_TopEdge = 156+(data->screen->BarHeight);
    ng.ng_Width = 100;
    ng.ng_Height = 20;
    ng.ng_Flags = PLACETEXT_IN;
    ng.ng_TextAttr = &coral16;
    ng.ng_GadgetText = (UBYTE *)GetString(&li,GADGETTEXT_abort);
    ng.ng_GadgetID = ID_abort;
    j2gadgets[G_ABORT] = gad = CreateGadget(BUTTON_KIND, gad, &ng, TAG_END);


    /* Gadget -- string "to" */

    ng.ng_LeftEdge = 190;
    ng.ng_TopEdge = 66+(data->screen->BarHeight);
    ng.ng_Width = 100;
    ng.ng_GadgetText = (UBYTE *)GetString(&li,GADGETTEXT_tostr);
    ng.ng_GadgetID = ID_tostr;
    j2gadgets[G_TOSTR] = gad = CreateGadget(TEXT_KIND, gad, &ng,
		GTTX_Text, GetString(&li,GTTX_Text_tostr), TAG_END);


    /* Gadget -- string "converting" */

    ng.ng_TopEdge = 6+(data->screen->BarHeight);
    ng.ng_GadgetText = (UBYTE *)GetString(&li,GADGETTEXT_constr);
    ng.ng_GadgetID = ID_constr;
    j2gadgets[G_CONSTR] = gad = CreateGadget(TEXT_KIND, gad, &ng,
		GTTX_Text, GetString(&li,GTTX_Text_constr), TAG_END);

//    return (gad ? TRUE : FALSE);

    ok = (gad ? TRUE : FALSE);
    return (ok);

    }


/* --------------------------------------------------------------------- */

VOID HandleEvent(struct GeneralData *data)
    {
    BOOL done=FALSE;
    ULONG windowsignal, signals;

    windowsignal = 1L << data->win->UserPort->mp_SigBit;

    while(!done)
	{
	signals = Wait(windowsignal);
	if(signals & windowsignal)
	    done = ProcWindowSignal(data);
	}
    }


/* --------------------------------------------------------------------- */


BOOL ProcWindowSignal (struct GeneralData *data)
    {
    struct IntuiMessage *msg;
    struct Gadget *gad;
    BOOL done = FALSE;
    ULONG class;
    UWORD code;
//    char str[64];

    struct strbuffer *sbuf;

    USHORT len;
//    struct WBArg *args;
//    int i;

    UWORD  menu_number, m_num, i_num, s_num;
    struct MenuItem *item;


    sbuf = &strbuf;
    while ( msg = (struct IntuiMessage *)GT_GetIMsg(data->win->UserPort) )
	{
        gad = msg->IAddress;
        class = msg->Class;
        code  = msg->Code;

	GT_ReplyIMsg (msg);

        switch (class)
	    {
	    case IDCMP_REFRESHWINDOW:
		GT_BeginRefresh(data->win);

		if (data->G1_FstGadget)		/* if not during conversion */
		    {
		    /* disp text "Sample Conversion Preview" */
		    DispText (data->win, GetString(&li,TEXT_PREVIEW), BBOX_LEFT + 8, BBOX_TOP -20);

		    /* disp bevel box */
		    RenderBBox (data->win, BBOX_LEFT, BBOX_TOP, BBOX_W, BBOX_H,
				GT_VisualInfo, data->VisualInfo,
				GTBB_Recessed, TRUE, TAG_DONE);

		    /* disp text in the box */
	            DispText (data->win, refline[0], BBOX_LEFT + 8, BBOX_TOP +2);
	            DispText (data->win, refline[1], BBOX_LEFT + 8, BBOX_TOP +20);
	            DispText (data->win, refline[2], BBOX_LEFT + 8, BBOX_TOP +40);

		    }
		else				/* during conversion */
		    {
		    /* disp input and output file name */

		    DispTextCenter (data->win, sbuf->fstr_in, FIN_TOP);
		    DispTextCenter (data->win, sbuf->fstr_out,FOUT_TOP);

		    /* disp status bar */
		    RenderBBox (data->win, SBAR_LEFT, SBAR_TOP, SBAR_W, SBAR_H,
				GT_VisualInfo, data->VisualInfo,
				GTBB_Recessed, TRUE, TAG_DONE);
		    }
		GT_EndRefresh(data->win, TRUE);
		break;

	    case IDCMP_GADGETDOWN:		/* select output code */
		switch(code)
		    {
		    case ID_CODE_EUC:
			ext_index = EUC;
			OutputCode = CTYPE_EUC;
			break;

		    case ID_CODE_SJIS:
			ext_index = SJIS;
			OutputCode = CTYPE_SJIS;
			break;

		    case ID_CODE_NEWJIS:
			ext_index = NEWJIS;
			OutputCode = CTYPE_NEWJIS;
			break;

		    case ID_CODE_OLDJIS:
			ext_index = OLDJIS;
			OutputCode = CTYPE_OLDJIS;
			break;

		    }

		break;

	    case IDCMP_GADGETUP:

		switch(gad->GadgetID)
		    {
		    case ID_FILTER:		/* Text Filter */
		    /* gadget_filter->Flags & GFLG_SELECTED */
			filter_flag = jgadgets[G_FILTER]->Flags & GFLG_SELECTED;
			break;

		    case ID_INPUT:		/* Select Input Files... */

			if (AslRequestTags (in_req, ASL_Hail, (ULONG) GetString(&li,TEXT_ASL_in),
						TAG_DONE) )
			    {
			    /* get dir name of input file */

			    strcpy ( sbuf->fstr_in, in_req->rf_Dir);

			    /* tag on the file name */

			    if ( ((len = strlen(sbuf->fstr_in)) > 0) && ((sbuf->fstr_in)[len - 1] != '/')
					 && ((sbuf->fstr_in)[len - 1] != ':') )
				strcat ( sbuf->fstr_in, "/");

			    strcat ( sbuf->fstr_in, in_req->rf_File);

//			    DispFSelStatus (data, sbuf, in_req);

			    /* show file name in input string gadget */

			    GT_SetGadgetAttrs (jgadgets[G_ISTR], data->win,
						NULL, GTST_String, sbuf->fstr_in, TAG_END );

			    /* show sample text according to input code */
			    DispSampleText (data, sbuf->fstr_in, BBOX_LEFT, BBOX_TOP);
                            }

			break;

		    case ID_OUTPUT:		/* Select Output Files... */
		        sbuf->sstr_out[0] = NULL;

			if (AslRequestTags (out_req, ASL_Hail, (ULONG) GetString(&li,TEXT_ASL_out),
						TAG_DONE) )
			    {
			    /* get dir name */
			    strcpy ( sbuf->fstr_out, out_req->rf_Dir);

			    /* use current dir if no dir specified */
			    if (!(*(sbuf->fstr_out)) )
				GetCurrentDirName (sbuf->fstr_out, 64);

			    /* tag on file name if specified */
			    if (*(out_req->rf_File) )
				{
				if ( ((len = strlen(sbuf->fstr_out)) > 0) && ((sbuf->fstr_out)[len - 1] != '/')
					 && ((sbuf->fstr_out)[len - 1] != ':') )
	    			    strcat ( sbuf->fstr_out, "/");
				strcat ( sbuf->fstr_out, out_req->rf_File);

				}

//			    DispDSelStatus (data, sbuf, out_req);

			    /* show output file name in output str gadget */

			    GT_SetGadgetAttrs (jgadgets[G_OSTR], data->win,
						NULL, GTST_String, sbuf->fstr_out, TAG_END );
			    }

			break;

		    case ID_ISSTR:		/* input file str gadget input */

			strcpy (sbuf->fstr_in,
				((struct StringInfo *)jgadgets[G_ISTR]->SpecialInfo)->Buffer );
			DispSampleText (data, sbuf->fstr_in, BBOX_LEFT, BBOX_TOP);
                        break;

		    case ID_OSSTR:		/* output file str gadget input */

			strcpy (sbuf->fstr_out,
				((struct StringInfo *)jgadgets[G_OSTR]->SpecialInfo)->Buffer );
                        break;

		    case ID_CONVERT:		/* Convert... */

			/* flash screen if no input file is selected */
			if (sbuf->fstr_in[0] == NULL)
			    {
			    DisplayBeep (data->screen);
			    break;
			    }

			/* remove first gadget list */

			RemoveGList (data->win, data->G1_FstGadget, -1);
		        FreeGadgets(data->G1_FstGadget);
			data->G1_FstGadget = NULL;
                        ClearRect (data->win, WIN_LEFT, WIN_TOP, WIN_RIGHT, WIN_BOT, 0);

			/* create 2nd gadget list for conversion status */

                    	if (CreateGadgetList2(data))
			    {
			    AddGList (data->win, data->G2_FstGadget, -1, -1, NULL);
			    RefreshGList (data->G2_FstGadget, data->win, NULL, -1);
			    GT_RefreshWindow (data->win, NULL);
			    RenderBBox (data->win, SBAR_LEFT, SBAR_TOP, SBAR_W, SBAR_H,
					GT_VisualInfo, data->VisualInfo,
					GTBB_Recessed, TRUE, TAG_DONE);

			    }
			else
			    ;

			/*  handle conversion */

			DoConvertion(data, sbuf, in_req, out_req);

			/* remove 2nd gadget list */

			RemoveGList (data->win, data->G2_FstGadget, -1);
			FreeGadgets (data->G2_FstGadget);
			data->G2_FstGadget = NULL;
			ClearRect (data->win, WIN_LEFT, WIN_TOP, WIN_RIGHT, WIN_BOT, 0);

			/* re-create 1st gadget list */

			CreateGadgetList1 (data);
			AddGList (data->win, data->G1_FstGadget, -1, -1, NULL);
			RefreshGList (data->G1_FstGadget, data->win, NULL, -1);
		        GT_RefreshWindow(data->win, NULL);

//			DispFSelStatus (data, sbuf, in_req);
//			DispDSelStatus (data, sbuf, out_req);

			/* redraw the sample conversion text box */

			DispText (data->win, GetString(&li,TEXT_PREVIEW), BBOX_LEFT + 8, BBOX_TOP -20);

			RenderBBox (data->win, BBOX_LEFT, BBOX_TOP, BBOX_W, BBOX_H,
					GT_VisualInfo, data->VisualInfo,
					GTBB_Recessed, TRUE, TAG_DONE);
	        	DispText (data->win, refline[0], BBOX_LEFT + 8, BBOX_TOP +2);
	        	DispText (data->win, refline[1], BBOX_LEFT + 8, BBOX_TOP +20);
	        	DispText (data->win, refline[2], BBOX_LEFT + 8, BBOX_TOP +40);

			/* re-show str gadgets contents */

			GT_SetGadgetAttrs (jgadgets[G_ISTR], data->win,
					NULL, GTST_String, sbuf->fstr_in, TAG_END );

			GT_SetGadgetAttrs (jgadgets[G_OSTR], data->win,
					NULL, GTST_String, sbuf->fstr_out, TAG_END );

			break;

		    case ID_QUIT:		/* Quit */
			done = TRUE;
			break;
		    }
                break;

	    case IDCMP_MENUPICK:

		menu_number = msg->Code;
		while (menu_number != MENUNULL && (!done) )
		    {
		    item = ItemAddress (data->MStrip, menu_number);
		    m_num = MENUNUM(menu_number);
		    i_num = ITEMNUM(menu_number);
		    s_num = SUBNUM(menu_number);

		    if ( (m_num == 0) && (i_num == 0) )  /* quit */
			done = TRUE;

		    if ( (m_num == 1) && (i_num == 0) )  /* input code auto detection */

			{
			DetInCode = TRUE;

			/* show conversion sample if input file already specified */

			if (*sbuf->fstr_in)
			    DispSampleText (data, sbuf->fstr_in, BBOX_LEFT, BBOX_TOP);
			}

		    if ( (m_num == 1) && (i_num != 0) )	 /* user force input code */
			{
			DetInCode = FALSE;
			switch (i_num-1)
			    {
			    case EUC:
				InputCode = CTYPE_EUC;
				break;
			    case SJIS:
				InputCode = CTYPE_SJIS;
				break;
			    case NEWJIS:
				InputCode = CTYPE_NEWJIS;
				break;
			    case OLDJIS:
				InputCode = CTYPE_OLDJIS;
				break;
			    }

			/* show conversion sample if input file already specified */

			if (*sbuf->fstr_in)
			    DispSampleText (data, sbuf->fstr_in, BBOX_LEFT, BBOX_TOP);

			}
		    menu_number = item->NextSelect;
		    }
		break;

	    case IDCMP_CLOSEWINDOW:
		done = TRUE;
		break;
            }
	}
    return(done);
    }

/* --------------------------------------------------------------------- */


VOID DoConvertion( struct GeneralData *data, struct strbuffer *sbuf,
		struct FileRequester *in_req, struct FileRequester *out_req)
    {
//    int i;
    char *str;
    SHORT len;
    struct FileInfoBlock *fib;
    BPTR lock;
    LONG ShowSize = 0;
    BOOL UsrAbort = FALSE;
//    BOOL temp; /* debug */
//    UBYTE *anch;

    /* allocate mem for file info block */
    fib = (struct FileInfoBlock *)AllocDosObject (DOS_FIB, TAG_DONE);

	/* clear display area */
        ClearRect (data->win, FIN_LEFT, FIN_TOP, FIN_RIGHT, FIN_BOT, 0);
        ClearRect (data->win, FOUT_LEFT, FOUT_TOP, FOUT_RIGHT, FOUT_BOT, 0);
	ClearRect (data->win, SBAR_LEFT+1, SBAR_TOP+1, SBAR_RIGHT, SBAR_BOT, 0);

	/* Display source file name */

	DispTextCenter (data->win, sbuf->fstr_in, FIN_TOP);

	/* Display target file name */

//	strcpy ( sbuf->fstr_out, out_req->rf_Dir);

	/* get lock of target */
	lock = Lock (sbuf->fstr_out, ACCESS_READ);

	/* check if target specified is a directory */

	if (lock && Examine(lock,fib) && (fib->fib_DirEntryType > 0) )

	    {

	    /* if only dir is giving, tag on file name taken from input file */

	    if ( ((len = strlen(sbuf->fstr_out)) > 0) && ((sbuf->fstr_out)[len - 1] != '/')
						  && ((sbuf->fstr_out)[len - 1] != ':') )
		strcat ( sbuf->fstr_out, "/");

	    str = FilePart (sbuf->fstr_in);
	    strcat (sbuf->fstr_out, str);

	    /* tag on the new ext according to output code */
	    strcat ( sbuf->fstr_out, ext[ext_index]);
	    }

//	printf("Examine result = %lx", temp); /*debug */
//	printf ("dir entry type = %lx\n", fib->fib_DirEntryType); /* debug */

	DispTextCenter (data->win, sbuf->fstr_out,FOUT_TOP);  /* show it */

	UnLock (lock);


	/* get source file size -- will show proceding status about every 1/16 of the file */

	lock = Lock (sbuf->fstr_in, ACCESS_READ);
	if (Examine (lock, fib))
	    {
	    ShowSize = fib->fib_Size / 16;
            if (ShowSize == 0)
		ShowSize++;
	    }
	UnLock (lock);

	/* finally, convert the file */

	if (err_convert = ConvertFile (data, sbuf, ShowSize)  )
	    {
	    switch (err_convert)
		{
		case UNKWNCODE:
		    TM_Request(NULL, (UBYTE *)GetString(&li,TEXT_ERROR), (UBYTE *)GetString(&li,TEXT_UNKWNCODE), (UBYTE *)GetString(&li,TEXT_ABORT), NULL, NULL);
                    break;
		case NONTEXT:
		    TM_Request(NULL, (UBYTE *)GetString(&li,TEXT_ERROR), (UBYTE *)GetString(&li,TEXT_NONTEXT), (UBYTE *)GetString(&li,TEXT_ABORT), NULL, NULL);
                    break;
		case ERROPENIN:
		    TM_Request(NULL, (UBYTE *)GetString(&li,TEXT_ERROR), (UBYTE *)GetString(&li,TEXT_ERROPENIN), (UBYTE *)GetString(&li,TEXT_ABORT), NULL, NULL);
                    break;
		case ERROPENOUT:
		    TM_Request(NULL, (UBYTE *)GetString(&li,TEXT_ERROR), (UBYTE *)GetString(&li,TEXT_ERROPENOUT), (UBYTE *)GetString(&li,TEXT_ABORT), NULL, NULL);
                    break;
		case ERRCOPY:
		    TM_Request(NULL, (UBYTE *)GetString(&li,TEXT_ERROR), (UBYTE *)GetString(&li,TEXT_ERRCOPY), (UBYTE *)GetString(&li,TEXT_ABORT), NULL, NULL);
                    break;
		case ASCIITEXT:
		    TM_Request(NULL, (UBYTE *)GetString(&li,TEXT_ERROR), (UBYTE *)GetString(&li,TEXT_ERRASCII), (UBYTE *)GetString(&li,TEXT_OK), NULL, NULL);
                    break;

		case USRABORT:
		    UsrAbort = TRUE;
		    break;

		default:
		    break;
		}
	    }

        if (UsrAbort)
	    DeleteFile (sbuf->fstr_out);

	if (fib)
	    FreeDosObject (DOS_FIB, fib);

    }


/* --------------------------------------------------------------------- */

UBYTE ConvertFile (struct GeneralData *data, struct strbuffer *sbuf, ULONG ShowSize)
    {
    ULONG r, length = 0, InCodeSave;
    int	len, l;
    BPTR InHandle = 0, OutHandle = 0;
    UBYTE ErrCode = NOERROR;
    LONG  WrtCount;

    /* clear internal buffer for identifying data code set. */
    SetJConversionAttrs(jcc, JCC_FirstByte, 0x00, JCC_ESC, 0, JCC_HanKata, 0, TAG_DONE);

    /* open input file */
    if ( !(InHandle = Open (sbuf->fstr_in, MODE_OLDFILE)) )
	{
	ErrCode = ERROPENIN;
	return (ErrCode);
	}

    /* First scan the whole source file to determine its code type */

    InCodeSave = DetCodeSet (jccID, InHandle, data->inbuf);

    /* if auto detect is selected, use the detected input code */
    if (DetInCode)
	InputCode = InCodeSave;
//	InputCode = DetCodeSet (jccID, InHandle, data->inbuf);

    switch (InputCode)
        {
	case CTYPE_BINARY:
	    ErrCode = NONTEXT;
            break;
	case CTYPE_UNKNOWN:
	    ErrCode = UNKWNCODE;
            break;
	case CTYPE_ASCII:
//	    ErrCode = ASCIITEXT;
	    InputCode = OutputCode;   /* just copy text if ASCII file */
            break;
        default:
            break;
        }

    /* disp warning if input code giving differ from detected code */

    if ( (InCodeSave != InputCode) && (InCodeSave != CTYPE_ASCII) )
	{
	if (!(TM_Request(NULL, (UBYTE *)GetString(&li,TEXT_WARNING), (UBYTE *)GetString(&li,TEXT_DIFFCODE), (UBYTE *)GetString(&li,TEXT_CON_ABORT), NULL, NULL) ) )

	    /* set error code if user choose to abort */
	    ErrCode = DIFFCODE;
	}


    if (!ErrCode)	/* no problem found in input code */
	{
//	printf ("input code = %lx\n", InputCode); /* debug */

	/* source code type determined.  It's time to convert */

	Seek (InHandle, 0, OFFSET_BEGINNING);	/* back to the beginning of the file */

//	printf ("file pos = %lx\n", Seek(InHandle, OFFSET_CURRENT, 0) );	/* debug */

	/* open output file */
	if ( !(OutHandle = Open (sbuf->fstr_out, MODE_NEWFILE)) )
	    ErrCode = ERROPENOUT;
	}

//  printf ("ErrCode = %lx\n", ErrCode);	/* debug */


    if (!ErrCode )	/* output file opened ok */

	{

	/* conversion loop */
	while ( (len = Read (InHandle, data->inbuf, BUFSIZE)) > 0  )
	    {

	    data->inbuf[len] = 0;		/* NULL terminate */
	    length += len;			/* total bytes read */

	    /* show conversion status if not run from CLI */
	    if (ArgCount <= 1)

		{
		r = length / ShowSize;

		if (r > 0 && r < 16)
		    ClearRect (data->win, SBAR_LEFT+1, SBAR_TOP+1, 20 * r, SBAR_BOT, 3);
		if (r >= 16)
		    ClearRect (data->win, SBAR_LEFT+1, SBAR_TOP+1, SBAR_RIGHT, SBAR_BOT, 3);
		}

	    if (filter_flag)	/* convert with filter tag if the flag is set */
		{
		WrtCount = JStringConvert (jcc, data->inbuf, data->outbuf, InputCode, OutputCode, -1, JCT_TextFilter, TRUE, TAG_END);
//		printf ("text filter selected\n");  /* debug */
		}
	    else		/* convert with no filter tag */
		WrtCount = JStringConvert (jcc, data->inbuf, data->outbuf, InputCode, OutputCode, -1, TAG_END );

//          printf ("inbuf[0] = %lx\n", data->inbuf[0]);   /* debug */
//          printf ("WrtCount = %lx\n", WrtCount);   /* debug */

	    /* write out to output file */
	    if ( (l = Write(OutHandle, data->outbuf, WrtCount)) < 0)

		{
//              printf ("write error. l = %lx\n", l);   /* debug */
		ErrCode = ERRCOPY;
		break;
		}

	    if (ArgCount <= 1)
		{
		if ( CheckUsrAbort(data) )
		    {
		    ErrCode = USRABORT;
		    break;
		    }
		}
	    }   /* while */

	if (len < 0)
	    ErrCode = ERRCOPY;

	}

    if (InHandle)	Close (InHandle);
    if (OutHandle)	Close (OutHandle);

    return (ErrCode);

    }


/* --------------------------------------------------------------------- */

/* routine to identify codeset of a file */

LONG DetCodeSet (struct JConversionCodeSet *jcc, BPTR handle, UBYTE *inbuf)
    {
    int len = 0;
    LONG InputCode = CTYPE_UNKNOWN, InputCodeSave = CTYPE_UNKNOWN;

    /* clear internal buffer for identifying data code set. */
    SetJConversionAttrs(jcc, JCC_FirstByte, 0x00, JCC_ESC, 0, JCC_HanKata, 0, TAG_DONE);

    if ( (len = Read (handle, inbuf, BUFSIZE)) > 0)
	InputCode = IdentifyCodeSet (jccID, inbuf, len);

    if (InputCode == CTYPE_BINARY)
	return (InputCode);

    Seek (handle, 0, OFFSET_BEGINNING);	 /* back to the beginning of the file */

    InputCode = CTYPE_UNKNOWN;

    while ( (len = Read (handle, inbuf, BUFSIZE)) > 0  )
	{

        inbuf[len] = 0;

	InputCode = IdentifyCodeSet(jccID, inbuf, -1);

	if (InputCode != CTYPE_CONTINUE && InputCode != CTYPE_ASCII
					&& InputCodeSave != CTYPE_SJIS )
	    InputCodeSave = InputCode;

	if (InputCode == CTYPE_BINARY)
	    break;

	}
    if (InputCodeSave != CTYPE_UNKNOWN && InputCode != CTYPE_BINARY)
	InputCode = InputCodeSave;
    return (InputCode);
    }

/* ---------------------------------------------------------------------- */

BOOL CheckUsrAbort (struct GeneralData *data)

    {
    BOOL abort = FALSE;
    struct IntuiMessage *msg;
    struct Gadget *gad;
    ULONG class;
    UWORD code;

    while ( msg = (struct IntuiMessage *)GT_GetIMsg(data->win->UserPort) )
	{
        gad = msg->IAddress;
        class = msg->Class;
        code  = msg->Code;

	GT_ReplyIMsg (msg);

        switch (class)
	    {
	    case IDCMP_GADGETUP:
		{
		if (gad->GadgetID == ID_abort)
		    abort = TRUE;
		break;
		}
	    break;
	    }
	}
    return (abort);
    }

/* ---------------------------------------------------------------------- */



VOID RenderBBox (struct Window *win, SHORT left, SHORT top, SHORT w, SHORT h, ULONG tags, ...)
    {
    DrawBevelBoxA (win->RPort, left, top+(win->BorderTop), w, h,
		(struct TagItem *)&tags);
    }

/* --------------------------------------------------------------------- */


VOID ClearRect (struct Window *win, SHORT left, SHORT top, SHORT right, SHORT bottom, UBYTE pen)
    {
    SetAPen (win->RPort, pen);
    RectFill (data->win->RPort, left, top+(win->BorderTop), right, bottom+(win->BorderTop) );
    SetAPen (win->RPort, 1);

    }


/* --------------------------------------------------------------------- */

//VOID DispFSelStatus ( struct GeneralData *data, struct strbuffer *sbuf,
//		struct FileRequester *in_req)

//    {

//    GT_SetGadgetAttrs (jgadgets[G_ISTR], data->win,
//			NULL, GTST_String, sbuf->fstr_in, TAG_END );

//    }

/* --------------------------------------------------------------------- */

//VOID DispDSelStatus ( struct GeneralData *data, struct strbuffer *sbuf,
//		struct FileRequester *out_req)

//    {

//    GT_SetGadgetAttrs (jgadgets[G_OSTR], data->win,
//			NULL, GTST_String, sbuf->fstr_out, TAG_END );
//    }


/* --------------------------------------------------------------------- */


VOID DispText (struct Window *win, UBYTE *str, WORD left, WORD top)
    {
    struct IntuiText intext = {0,0,JAM2,0,0,&coral16,NULL,NULL};

    intext.FrontPen = 1;
    intext.BackPen = win->RPort->BgPen;
    intext.IText = str;
    PrintIText (win->RPort, &intext, left, top+(win->BorderTop) );
    }

/* --------------------------------------------------------------------- */

VOID DispTextCenter (struct Window *win, UBYTE *str, WORD strtop)

    {
    WORD left;
    short len, line;
    int j;
    UBYTE *s;

    line = 1;
    if ( (len = strlen(str) * 8) > WINDOW_W)
	{
	line = len / WINDOW_W + (len % WINDOW_W ? 1 : 0);
	len /= line;
	}
    left = (WINDOW_W - len) / 2;
    for (j=0,s=str; j < line; j++, s+=len)
	DispText (win, s, left, strtop + j*20);
    }


/* -------------------------------------------------------------------- */

/* Show sample conversion text according to input code */

VOID DispSampleText (struct GeneralData *data, UBYTE *filename, WORD left, WORD top)

    {
    BPTR InHandle = 0;
    LONG InCode = CTYPE_UNKNOWN;
    int i;

    /* clear the display area */

    ClearRect (data->win, BBOX_LEFT+1, BBOX_TOP+1, BBOX_RIGHT, BBOX_BOT, 0);

    /* clear the fresh arrays */
    for (i=0; i<3; i++)
	refline[i] = NULL;

    if ( InHandle = Open (filename, MODE_OLDFILE) )
        {

	InCode = DetCodeSet (jccID, InHandle, data->inbuf);
	Seek (InHandle, 0, OFFSET_BEGINNING);	/* back to the beginning of the file */

        if (DetInCode)
	    {
            switch (InCode)
        	{
		case CTYPE_BINARY:
	            DispText (data->win, GetString (&li, TEXT_NONTEXT), BBOX_LEFT + 8, BBOX_TOP +20);
		    refline[1] = GetString (&li, TEXT_NONTEXT);
                    break;
		case CTYPE_UNKNOWN:
	            DispText (data->win, GetString (&li, TEXT_UNKWNCODE), BBOX_LEFT + 8, BBOX_TOP +20);
		    refline[1] = GetString (&li, TEXT_UNKWNCODE);
                    break;
//		case CTYPE_ASCII:
//            	    ;/* fall through */

        	default:
		    DispCharLine (data, InCode, OutputCode, InHandle);
                    break;
        	}
	    }
	else
	    {

	    /* if input file is all ASCII, just display ASCII */

	    if (InCode == CTYPE_ASCII)
		DispCharLine (data, InCode, OutputCode, InHandle);
	    else
		DispCharLine (data, InputCode, OutputCode, InHandle);
	    }
        Close (InHandle);

        }
    }

/* --------------------------------------------------------------------- */

/* display 3 line of converted text. If the file is not plain ASCII, try */
/* to find lines with non ASCII characters */

VOID DispCharLine (struct GeneralData *data, LONG incode, LONG outcode, BPTR InHandle)

    {
    static UBYTE line1[LLENGTH+8];
    static UBYTE line2[LLENGTH+8];
    static UBYTE line3[LLENGTH+8];
    UBYTE *str = NULL, *str1 = NULL;
    UBYTE *s[3] = {line1, line2, line3};
    int len, l, i;
    LONG tmpcode, WrtCount;
    BOOL FoundNonAscii = FALSE;


    /* clear internal buffer for identifying data code set. */
    SetJConversionAttrs(jccID, JCC_FirstByte, 0x00, JCC_ESC, 0, JCC_HanKata, 0, TAG_DONE);

    /* clear text buffer */

    for (i=0; i<3; i++)
	memset (s[i],0,LLENGTH+1);


    /* first find the first non ASCII portion */

    while ( (len = Read (InHandle, data->inbuf, BUFSIZE)) > 0  )
	{

	/* try to start from a line feed */

	if (!(str = memchr (data->inbuf, 0x0A, len)) )
	    /* start at beginning of buffer is no line feed fount */
	    str = data->inbuf;

	while (str < (data->inbuf + len) )
	    {
	    /* determine display length -- min (show length, bytes remain in buffer) */
	    l = MIN(LLENGTH, data->inbuf + len - str);

	    /* if ASCII file, don't bother to find non-ASCII char */
	    if (incode == CTYPE_ASCII)
		{
		FoundNonAscii = TRUE;
		break;
		}

	    /* look for non ASCII chars */
	    if ( (tmpcode = IdentifyCodeSet (jccID, str, l) ) != CTYPE_ASCII )
		{
		FoundNonAscii = TRUE;
//		printf ("first chars to display: %lx %lx %lx %lx\n", *str, *(str+1), *(str+2), *(str+3)); /* debug */
		break;
		}
	    /* go on looking if not found */
	    str += l;

	    /* make sure start at new line */
	    if (!(str = memchr (str, 0x0A, data->inbuf+len-str)) )
		{
//		printf ("no lf\n");  /* debug */
		break; 		 /* stop look if no lf found */
		}
	    else
		{
//		printf ("found lf, *str = %lx %lx\n", *str, *(str+1) ); /*debug */
		str++;
		}
	    }

	/* if non ASCII not found, keep reading in more text. Otherwise stop */

	if (FoundNonAscii)
	    break;
	}

//    if (!FoundNonAscii)
//	str = data->inbuf;

    /* for ASCII file, just use output code as input, so no conversion is done */

    if ( incode == CTYPE_ASCII )
	incode = outcode;


    /* now str points to the line with non ASCII char, display 3 lines from here */

    for (i=0; i<3; i++)
	{
//	printf ("incode = %lx  outcode = %lx\n",incode, outcode); /* debug */

	/* skip leading CR and LF */
	while ((*str == 0x0A) || (*str == 0x0D) )
	    str++;
//	printf ("*str = %lx  *(str-1) = %lx\n", *str, *(str-1)); /* debug */

	/* cut off trailing CR and LF */
        str1 = str;
	if (str1 = memchr (str, 0x0A, l))
	    l = l - (str + l - str1);
	if (str1 = memchr (str, 0x0D, l))
	    l = l - (str + l - str1);

	/* convert the line */
	WrtCount = JStringConvert (jcc, str, data->outbuf, incode, outcode,
				l, TAG_END);

	/* if resulting more char, cut the length and do it again */
	while (WrtCount > l && WrtCount > LLENGTH)
	    {
	    l = l - (WrtCount - l);

	    WrtCount = JStringConvert (jcc, str, data->outbuf, incode, outcode,
					l, TAG_END);
	    }

	/* if output code is not euc, convert it to euc before display it */

	if (outcode != CTYPE_EUC)
	    {
	    memcpy (str, data->outbuf, WrtCount);
	    WrtCount = JStringConvert (jcc, str, data->outbuf, outcode, CTYPE_EUC,
					WrtCount, TAG_END);
	    }

	/* copy it to the display buffer */
	memcpy (s[i], data->outbuf, WrtCount);

	/* make sure end with NULL */

	s[i][WrtCount] = 0;

	/* show text */
        DispText (data->win, s[i], BBOX_LEFT + 8, BBOX_TOP+2 +i*20);

	/* set refresh array pointer */
	refline[i] = s[i];
	str += l;

	/* look for next LF */
	if(!(str = memchr (str, 0x0A, data->inbuf+len - str)) )
	    break;

	if ( (l = MIN(LLENGTH, data->inbuf + len - str)) == 0)
	    break;
	}

    Seek (InHandle, 0, OFFSET_BEGINNING);	/* back to the beginning of the file */

    }
