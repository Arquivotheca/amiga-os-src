
#include "keytoy.h"
#include "keyboard.h"

extern struct Window *window;
extern struct RastPort *rport;         /* pointer to clock-window RastPort */
extern struct TextAttr Topaz;
extern struct TextAttr TopazBI;
extern struct Gadget LShift;
extern struct KeyTop Key[NUMKEYS];
extern struct KeyMap KMap;
extern struct Image UpImage;
extern UWORD version;			/* console.device version number */
extern WORD BoardType;
extern WORD Mode;

struct Image CheckImage = {0, 0, 28, 12, 1, NULL, 0x2, 0x1, NULL};

UBYTE Indexes[8][8] = {
    {3, 3, 3, 3, 3, 3, 3, 3},	/* NOQUAL	 */
    {3, 2, 3, 2, 3, 2, 3, 2},	/* SHIFT	 */
    {3, 3, 2, 2, 3, 3, 2, 2},	/* ALT		 */
    {3, 2, 1, 0, 3, 2, 1, 0},	/* SHIFT-ALT	 */
    {3, 3, 3, 3, 2, 2, 2, 2},	/* CONTROL	 */
    {3, 2, 3, 2, 1, 0, 1, 0},	/* SHIFT-CONTROL */
    {3, 3, 2, 2, 1, 1, 0, 0},	/* ALT-CONTROL	 */
    {3, 2, 1, 0, 7, 6, 5, 4}	/* SHIFT-ALT-CONTROL (VANILLA) */
};

UBYTE DIndexes[8][8] = {
    {0, 8, 8, 8, 8, 8, 8, 8},
    {0, 1, 8, 8, 8, 8, 8, 8},
    {0, 8, 1, 8, 8, 8, 8, 8},
    {0, 1, 2, 3, 8, 8, 8, 8},
    {0, 8, 8, 8, 1, 8, 8, 8},
    {0, 1, 8, 8, 2, 3, 8, 8},
    {0, 8, 1, 8, 2, 8, 3, 8},
    {0, 1, 2, 3, 4, 5, 6, 7}
};

UBYTE Accents[] = {0, 0x27, '`', '^', '~', 0xa8};

UBYTE CtrlChrs[36][3] = {
    "^@", "^A", "^B", "^C", "^D", "^E", "^F", "^G",
    "^H", "^I", "^J", "^K", "^L", "^M", "^N", "^O",
    "^P", "^Q", "^R", "^S", "^T", "^U", "^V", "^W",
    "^X", "^Y", "^Z", "^[", "^\\", "^]", "^^", "^_"
};


DrawBoard()
{
    /* clear raster */
    SetAPen(rport, WHITE);
    SetDrMd(rport, JAM1);
    RectFill(rport, window->BorderLeft, window->BorderTop, window->Width -
        window->BorderRight - 1, window->Height - window->BorderBottom - 1);

    /* draw the keyboard */
    DrawBorder(rport, Key[0].KBorder, 0, 0);
    DrawKeys();
    DrawImage(rport, &UpImage, 0, 0);
    RefreshGList(&LShift, window, NULL, -1);
}


DrawKeys()
{   /* draw keys for current Mode */
    WORD i;
    UBYTE *LoTypes, *HiTypes;
    ULONG *LoMap, *HiMap;
    /* clear keycaps text */
    for(i = 0; i < NUMKEYS; i++)
    {
	if(Key[i].Flag & K_TEXT)
	    Key[i].KCapTxt[0] = ' ';
	else Key[i].KCapTxt[0] = NULL;
	Key[i].KCapTxt[1] = ' ';
	Key[i].KCapTxt[2] = NULL;
	if((!(Key[i].Flag & K_PREDEFINED)) && (Key[i].Flag
	  != K_NADA))
	{
	    Key[i].KText->ITextFont = &Topaz;
	    Key[i].KText->FrontPen = BLACK;
	}
    }
    LoTypes = KMap.km_LoKeyMapTypes;
    HiTypes = KMap.km_HiKeyMapTypes;
    LoMap = KMap.km_LoKeyMap;
    HiMap = KMap.km_HiKeyMap;
    for(i = 0; i < 0x40; i++, LoTypes++, LoMap++)
	SetKey(i, LoTypes, LoMap);
    for(i = 0x40; i < NUMKEYS; i++, HiTypes++, HiMap++)
	SetKey(i, HiTypes, HiMap);
    PrintIText(rport, Key[0].KText, 3, 2);
}


SetKey(k, Types, Map)
WORD k;
UBYTE *Types;
ULONG *Map;
{
    WORD i;
    BYTE index;
    UBYTE ckey, cindex;
/*     UBYTE ckey, index, cindex; */
    UBYTE *Entry, *Dead;

    if(Key[k].Flag & K_PREDEFINED)return;

    if(*Types & KCF_DEAD)
    {
	Dead = (UBYTE *)(*Map);
	index = (DIndexes[(*Types & 7)][Mode]) * 2;
	if(index < 16)
	{
	    if(Dead[index] == 0)
		Parse(k, Dead[index+1]);
	    else if(Dead[index] == DPF_DEAD)
	    {
		if((Dead[index+1] < 6))
		{
		    Key[k].KText->FrontPen = RED;
		    Parse(k, Accents[Dead[index+1]]);
		}
		else if((Dead[index+1] & 0x07) < 6)
		{
		    Key[k].KText->FrontPen = RED;
		    Parse(k, Accents[(Dead[index+1] & 0x07)]);
		}
	    }
	    else if(Dead[index] == DPF_MOD)
	    {
		Key[k].KText->ITextFont = &TopazBI;
		index = Dead[index+1];
		Parse(k, *(Dead + index));	/* Parse(k, Dead[index]); */
	    }
	}
    }

    else if(*Types & KCF_STRING)
    {
	Dead = (UBYTE *)(*Map);
	index = (DIndexes[(*Types & 7)][Mode]) * 2;
	if(index < 16)
	{
	    if(Dead[index] != 1)
	    {
		Key[k].KCapTxt[0] = '$';
		Key[k].KCapTxt[1] = '$';
		Key[k].KText->FrontPen = BLUE;
	    }
	    else
	    {
		index = Dead[index+1];
		Parse(k, Dead[index]);
	    }
	}
    }

    else if(*Types != KCF_NOP)
    {
	index = Indexes[*Types][Mode];
	Entry = (UBYTE *)Map;
	if(index > 3)
	{
	    index &= 0x03;
	    for (i = 0; i < 4; i++)
	    {
		if ((Entry[index] & 0xc0) == 0x40)	/* CONTROLable? */
		{
		    ckey = Entry[index] & 0x1f;
		    /* If version of console.device is 35 or greater, then */
		    /* make sure ALT and CONTROL correctly work together   */
		    if (version >= 35)
		    {
			if (*Types & (KCF_ALT | KCF_CONTROL))
			   ckey |= 0x80;
		    }
		    break;
		}
		else
		    if ((--index) < 0)index = 3;
	    }
	    if (i == 4) ckey = Entry[index];
	}
	else ckey = Entry[index];
	Parse(k, ckey);
    }
}


Parse(k, iso)
WORD k;
UBYTE iso;
{
    UBYTE chr[2];

    if(iso < 0x20)
    {
	Key[k].KCapTxt[0] = CtrlChrs[iso][0];
	Key[k].KCapTxt[1] = CtrlChrs[iso][1];
    }
    else if((iso < 0x7f) || (iso > 0x9f))
    {
	Key[k].KCapTxt[0] = iso;
	Key[k].KCapTxt[1] = ' ';
    }
    else if(iso == 0x7f)
    {
	Key[k].KCapTxt[0] = 'D';
	Key[k].KCapTxt[1] = 'L';
    }
    else
    {
	Key[k].KCapTxt[0] = '~';
	Key[k].KCapTxt[1] = (UBYTE)(iso - 0x40);
    }
}
