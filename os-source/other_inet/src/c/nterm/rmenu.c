/* handle menu processing */

#include "st.h"
#include "libraries/dos.h"
#include "menu.h"
#include "term.h"
#include "colors.h"
#include "meta.h"

/* #define DEBUG /* */
#define NUMSAVEALL (NUMTERMINAL*2+NUMWRAP*2+NUMCTRL*2+NUMBELL*2+NUMCOLUMN*2+NUMROW*2+NUMDEL*2+3+3+MAXCOLUMNS+NUMFUNC*(FLEN-1))

extern int TabReq(), VTBeep(), SetDelAndBksp();
extern int set_linefeed_mode(), set_newline_mode();
extern QuitReq(), WhoReq();
extern AmigaNormal(), Msg(), StringRequest(), BreakSerial();
extern struct Window *w;
extern struct Menu *menus;

/* menu items */
extern struct MenuItem terminalitems[];
extern struct MenuItem setupitems[], projectitems[];
extern struct MenuItem ctrl_charsitems[], meta_keysitems[];
extern struct MenuItem line_wrapitems[];
extern struct MenuItem bellitems[];
extern struct MenuItem columnitems[], rowitems[];
extern struct MenuItem delandbkspitems[];
/* menu intuitext */
extern struct IntuiText terminaltext[];
extern struct IntuiText ctrl_charstext[];
extern struct IntuiText croutputtext[], line_wraptext[];
extern struct IntuiText belltext[];
extern struct IntuiText columntext[], rowtext[];
extern struct IntuiText delandbksptext[];

extern UBYTE done, tabs[], row_mode, col_mode;
extern UBYTE terminal_mode, Back[], Fore[];
extern UBYTE executed, beep, numcolumns;
extern char fkeys[FMAX][FLEN+2];
extern unsigned char pbuf[];

static UBYTE terminalmodes[] = {TTY, VT52, VT100, AMIGA};
static UBYTE ctrls[] = {TRUE, FALSE};
static UBYTE bells[] = {AUDIBLE, SILENT, NOFLASH};
static UBYTE wraps[] = {SUBITEMLINEWRAPON,SUBITEMLINEWRAPOFF};

DoMenus(msg)
struct IntuiMessage *msg;
{
ULONG ThisOne, NextOne, choice;
LONG i, j;
unsigned char c;

	NextOne = msg->Code;
	do {
		ThisOne = NextOne;
		switch (MENUNUM(ThisOne)) {
		case MENUPROJECT:
			choice = ITEMNUM(ThisOne); /* get selection */
			switch (choice) {
			case ITEMSAVE: 
				Save_Settings();
				break;
			case ITEMLOAD: 
				Load_Settings(1);
				break;
			case ITEMRESET: 
				Reset_Terminal();
				break;
			case ITEMBREAK:
				BreakSerial();
				Msg("BREAK SENT"); 
				break;
			case ITEMWHO: 
				WhoReq(w);
				break;
			case ITEMQUIT:	
				done = TRUE;
				break;
			default: break;
			}
			NextOne = ((struct MenuItem *)ItemAddress(&menus, ThisOne))->NextSelect;
			break;

		case MENUTERMINAL:
			choice = ITEMNUM(ThisOne); /* get selection */
			if (terminalitems[choice].Flags & CHECKED) { /* if this one is checked */
				Set_Terminal(terminalmodes[choice]); /* set it */
				sprintf(pbuf,"%s TERMINAL SELECTED.",
					terminaltext[choice].IText); /* display it */
				Msg(pbuf);
			}
			NextOne = ((struct MenuItem *)ItemAddress(&menus, ThisOne))->NextSelect;
			break;

		case MENUSETUP:
			switch (ITEMNUM(ThisOne)) {
	
			case ITEMLINE_WRAP:
				choice = SUBNUM(ThisOne); /* get selection */
				if (line_wrapitems[choice].Flags & CHECKED) { /* if this one is checked */
					Set_Line_Wrap(choice);
					sprintf(pbuf,"LINEWRAP %s SELECTED.",
						line_wraptext[choice].IText); /* display it */
					Msg(pbuf);
				}
				break;
			case ITEMCTRL_CHARS:
				choice = SUBNUM(ThisOne); /* get selection */
				if (ctrl_charsitems[choice].Flags & CHECKED) { /* if this one is checked */
					executed = ctrls[choice]; /* set it */
					sprintf(pbuf,"%s CONTROL CHARACTERS SELECTED.",
						ctrl_charstext[choice].IText); /* display it */
					Msg(pbuf);
				}
				break;
			case ITEMTABS: TabReq(w);
				break;
			case ITEMBELL:
				choice = SUBNUM(ThisOne); /* get selection */
				if (bellitems[choice].Flags & CHECKED) { /* if this one is checked */
					beep = bells[choice]; /* set it */
					sprintf(pbuf,"BELL IS %s.",
						belltext[choice].IText); /* display it */
					Msg(pbuf);
				}
				break;
			case ITEMCOLUMN:
				choice = SUBNUM(ThisOne); /* get selection */
				if (columnitems[choice].Flags & CHECKED)
					col_mode = choice;
				break;
			case ITEMROW:
				choice = SUBNUM(ThisOne); /* get selection */
				if (rowitems[choice].Flags & CHECKED) { /* if this one is checked */
					row_mode = choice; /* flag that we want to change rows */
				}
				break;

			case ITEMDELANDBKSP:
				choice = SUBNUM(ThisOne); /* get selection */
				if (delandbkspitems[choice].Flags & CHECKED) { /* if this one is checked */
					SetDelAndBksp(choice); /* invoke it */
					sprintf(pbuf, "DELETE AND BACKSPACE %s SELECTED", delandbksptext[choice].IText); /* display it */
					Msg(pbuf);
				}
				break;
			case ITEMCOLOR:
				ColorReq(w);
				break;
			case ITEMFUNC:
				MetaRequest(w);
				break;
			default: break;
			}
			NextOne = ((struct MenuItem *)ItemAddress(&menus, ThisOne))->NextSelect;
			break;
		default:
			NextOne = MENUNULL;
			break;
		}
	} while (NextOne != MENUNULL);
}

static LONG setting_file;
char *setting_name;

Save_Settings() /* save terminal settings */
{
UBYTE sbuf[NUMSAVEALL], *sbufptr = sbuf, i, j;
   if (!StringRequest(w,setting_name,"Save Settings")) return(FALSE); /* exit if not sure */
	if (Exists(setting_name)) /* if it exits, does he/she want to overwrite? */
		if (!OverReq(w)) return(FALSE); /* don't want to overwrite */
	if ((setting_file = Open(setting_name,MODE_NEWFILE))==0) {
		Msg("CAN'T OPEN SETTING FILE");
		return(FALSE);
	}
	Msg("SAVING SETTINGS...");
	for (i=0; i<NUMTERMINAL; i++) {
		*sbufptr++ = (terminalitems[i].Flags & 0xff00) >> 8;
		*sbufptr++ = terminalitems[i].Flags;
	}
	for (i=0; i<NUMWRAP; i++) {
		*sbufptr++ = (line_wrapitems[i].Flags & 0xff00) >> 8;
		*sbufptr++ = line_wrapitems[i].Flags;
	}
	for (i=0; i<NUMCTRL; i++) {
		*sbufptr++ = (ctrl_charsitems[i].Flags & 0xff00) >> 8;
		*sbufptr++ = ctrl_charsitems[i].Flags;
	}
	for (i=0; i<NUMBELL; i++) {
		*sbufptr++ = (bellitems[i].Flags & 0xff00) >> 8;
		*sbufptr++ = bellitems[i].Flags;
	}
	for (i=0; i<NUMCOLUMN; i++) {
		*sbufptr++ = (columnitems[i].Flags & 0xff00) >> 8;
		*sbufptr++ = columnitems[i].Flags;
	}
	for (i=0; i<NUMROW; i++) {
		*sbufptr++ = (rowitems[i].Flags & 0xff00) >> 8;
		*sbufptr++ = rowitems[i].Flags;
	}

	for (i=0; i<NUMDEL; i++) {
		*sbufptr++ = (delandbkspitems[i].Flags & 0xff00) >> 8;
		*sbufptr++ = delandbkspitems[i].Flags;
	}
	*sbufptr++ = Back[0];
	*sbufptr++ = Back[1];
	*sbufptr++ = Back[2];
	*sbufptr++ = Fore[0];
	*sbufptr++ = Fore[1];
	*sbufptr++ = Fore[2];
	for (i=0; i<MAXCOLUMNS; i++) /* save all tab posns */
		*sbufptr++ = tabs[i];
	for (i=0; i<NUMFUNC; i++) { /* for all fkeys */
		for (j=2; j<=FLEN; j++) { /* for each char */
			*sbufptr++ = fkeys[i][j]; /* save it */
		}
	}
	Write(setting_file,sbuf,sbufptr-sbuf);
	Close(setting_file);
	if (sbufptr - sbuf != NUMSAVEALL) {
		sprintf(pbuf, "INTERNAL ERROR: NUMSAVEALL=%ld, WROTE %ld BYTES!",
			NUMSAVEALL, sbufptr - sbuf);
		Msg(pbuf);
		return(FALSE);
	}
	else {
		Msg("SETTINGS SAVED");
		return(TRUE);
	}
}

Load_Settings(reqflag) /* load terminal settings and set flags */
int reqflag; /* 0-do not bring up requestor else do */
{
char s[FLEN];
UWORD count = 0;
UBYTE i, j;

	if (reqflag) { /* if want to bring up requestor */
		if (!StringRequest(w,setting_name,"Load Settings")) {
			return(FALSE); /* exit if not sure */
		}
	}

	if ((setting_file = Open(setting_name,MODE_OLDFILE))==0) {
		Msg("CAN'T OPEN SETTING FILE");
		return(FALSE);
	}
	Msg("LOADING SETTINGS...");

	for (i=0; i<NUMTERMINAL; i++) {
		if (Read(setting_file,&terminalitems[i].Flags,2) != 2) {
			goto err;
		}
		if (terminalitems[i].Flags & CHECKED) {
			Set_Terminal(terminal_mode = terminalmodes[i]);
	/* erase to end of screen (erases entire screen as the cursor is home) */
			CDPutStr("\233J");
			VTBeep(beep!=SILENT); /* beep the terminal (maybe) */
		}
	}
	count += i*2;
	for (i=0; i<NUMWRAP; i++) {
		if (Read(setting_file,&line_wrapitems[i].Flags,2) != 2) {
			goto err;
		}
		if (line_wrapitems[i].Flags & CHECKED) Set_Line_Wrap(wraps[i]);
	}
	count += i*2;

	for (i=0; i<NUMCTRL; i++) {
		if (Read(setting_file,&ctrl_charsitems[i].Flags,2) != 2) {
			goto err;
		}
		if (ctrl_charsitems[i].Flags & CHECKED) executed = ctrls[i];
	}
	count += i*2;

	for (i=0; i<NUMBELL; i++) {
		if (Read(setting_file,&bellitems[i].Flags,2) != 2) {
			goto err;
		}
		if (bellitems[i].Flags & CHECKED) beep = bells[i];
	}
	count += i*2;

	for (i=0; i<NUMCOLUMN; i++) {
		if (Read(setting_file,&columnitems[i].Flags,2) != 2) {
			goto err;
		}
		if (columnitems[i].Flags & CHECKED) col_mode = i;
	}
	count += i*2;

	for (i=0; i<NUMROW; i++) {
		if (Read(setting_file,&rowitems[i].Flags,2) != 2) {
			goto err;
		}
		if (rowitems[i].Flags & CHECKED) row_mode = i;
	}
	count += i*2;



	for (i=0; i<NUMDEL; i++) {
		if (Read(setting_file,&delandbkspitems[i].Flags,2) != 2) {
			goto err;
		}
		if (delandbkspitems[i].Flags & CHECKED) SetDelAndBksp(i);
	}
	count += i*2;

	for (i=0; i<3; i++) {
		/* background color */
		if (Read(setting_file,&Back[i],1) != 1) goto err;
	}
	count += i;

	for (i=0; i<3; i++) {
		/* foreground color */
		if (Read(setting_file,&Fore[i],1) != 1) goto err;
	}
	count += i;
	Screen_Normal(); /* install the new colors */

	/* set new tabs */
	if (Read(setting_file,tabs,MAXCOLUMNS) != MAXCOLUMNS) goto err;
	count += MAXCOLUMNS;

	for (i=0; i<NUMFUNC; i++) { /* for all fkeys */
		/* read in each string */
		if (Read(setting_file,s,FLEN-1) != FLEN-1) goto err;
		Set_Function_Key(s,i+1); /* set the function key */
	}
	count += i*(FLEN-1);

	Close(setting_file);

err:
	if (count != NUMSAVEALL) {
		Msg("WARNING: SETTINGS FILE WRONG SIZE!  RESAVE SETTINGS!");
		return(FALSE);
	}
	else {
	    Msg("SETTINGS LOADED");
	    return(TRUE);
	}
}

Reset_Terminal()
{
	Set_Terminal(terminal_mode); /* re-init the terminal mode */
	CDPutStr("\233H\233J"); /* erase entire screen */
	VTBeep(beep!=SILENT); /* beep the terminal (maybe) */
	Msg("TERMINAL RESET"); /* show status */
}


#include <libraries/dos.h>

Exists(name)
char *name;
{
	int *lock;
	extern char title[];
	extern UBYTE title_length;

	UnLock((lock = (int *)Lock(name,ACCESS_READ)));
	if (lock!=0) return(TRUE);
	else {
		UnLock((lock = (int *)Lock(name,ACCESS_WRITE)));
		if (lock!=0) return(TRUE);
	}
	return(FALSE);
}

Msg(s) /* display string on screen title bar */
char *s;
{
UBYTE i;
	strcpy(&title[title_length],s); /* copy message */
	for (i=title_length+strlen(s); i<strlen(title); i++)
		title[i] = ' '; /* clean rest of msg area */
	SetWindowTitles(w,-1,title); /* show title & message */
}
