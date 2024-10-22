/*
 *  $Id$
 *
 *  label.image
 *  Text Layout and Rendering Module
 *  Copyright (C) 1994 Commodore-Amiga, Inc.
 *  All Rights Reserved
 *  Unauthorized Duplication or Distribution Prohibited
 *
 */

/*
$Log$
*/

/*****************************************************************************
 *
 *  Includes
 *
 *****************************************************************************/

#include <exec/types.h>
#include <exec/memory.h>

#include <intuition/intuition.h>
#include <intuition/imageclass.h>
#include <intuition/classusr.h>
#include <intuition/classes.h>
#include <intuition/cghooks.h>

#include <graphics/displayinfo.h>
#include <graphics/gfxmacros.h>

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/utility_protos.h>
#include <clib/diskfont_protos.h>
#include <clib/macros.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/diskfont_pragmas.h>

#include <math.h>
#include <string.h>

#include "classbase.h"
#include "classdata.h"

/*****************************************************************************
 *
 *  Local Prototypes
 *
 *****************************************************************************/

static struct textFragment *newTextFragment(
    struct ClassLib *ClassBase,
    struct textFragment **pFragmentList,
    STRPTR text,
    UWORD x, UWORD y,
    UWORD width, UWORD height,
    ULONG softStyle,
    ULONG fgPen, ULONG bgPen,
    BOOL fUnderscore
);

/*****************************************************************************
 *
 *  Local Types
 *
 *****************************************************************************/

typedef enum {
    JustifyLeft,
    JustifyRight,
    JustifyCenter
} TJustification;

/*****************************************************************************
 *
 *  newTextFragment()   -   add new fragment to text fragment list
 *
 *****************************************************************************/

static struct textFragment *newTextFragment(
    struct ClassLib *ClassBase,
    struct textFragment **pFragmentList,
    STRPTR text,
    UWORD x, UWORD y,
    UWORD width, UWORD height,
    ULONG softStyle,
    ULONG fgPen, ULONG bgPen,
    BOOL fUnderscore
)
{

    struct textFragment *pTextFragment, *pLastFragment;

    /* Allocate text fragment */
    pTextFragment=
        AllocVec(
            sizeof(struct textFragment),
            MEMF_CLEAR
        );
    if (pTextFragment) {

        /* Initialize text fragment */
        pTextFragment->text=createString(ClassBase,text);
        if (pTextFragment->text) {

            pTextFragment->x=x;
            pTextFragment->y=y;
            pTextFragment->width=width;
            pTextFragment->height=height;
            pTextFragment->softStyle=softStyle;
            pTextFragment->fgPen=fgPen;
            pTextFragment->bgPen=bgPen;
            pTextFragment->fUnderscore=fUnderscore;

            /* Link text fragment to list */
            pLastFragment=*pFragmentList;
            if (pLastFragment) {

                while (pLastFragment->next) {

                    pLastFragment=pLastFragment->next;

                }

                pLastFragment->next=pTextFragment;

            } else {

                *pFragmentList=pTextFragment;

            }

        } else {

            FreeVec(pTextFragment);
            pTextFragment=NULL;

        }

    }

    /* Return new text fragment */
    return pTextFragment;

}

/*****************************************************************************
 *
 *  parseTextCode() -   parse text attribute code
 *
 *****************************************************************************/

enum textCode parseTextCode(
    STRPTR codeString,
    STRPTR *pEnd
)
{

    static struct codeEntry {
        STRPTR          string;
        enum textCode   code;
    } codeTable[] ={

        "APEN", code_APen,
        "B", code_BoldOn,
        "BG", code_Background,
        "CLEARTABS", code_ClearTabs,
        "FG", code_Foreground,
        "I", code_ItalicOn,
        "JCENTER", code_JustifyCenter,
        "JLEFT", code_JustifyLeft,
        "JRIGHT", code_JustifyRight,
        "PLAIN", code_Plain,
        "SETTABS", code_SetTabs,
        "TAB", code_Tab,
        "U", code_UnderlineOn,
        "UB", code_BoldOff,
        "UI", code_ItalicOff,
        "UU", code_UnderlineOff,
        NULL, code_Bad,

    };
    struct codeEntry *pCodeEntry;

    STRPTR pCodeChar;
    STRPTR pCodeStart;
    UBYTE oldTerminator;

    /* Initialize end character as start (temporarily
       assuming code is invalid) */
    *pEnd=codeString;

    /* Verify leading '@' */
    if (*codeString!='@') {

        return code_Bad;

    }

    /* Verify leading '{' */
    if (codeString[1]!='{') {

        return code_Bad;

    }

    /* Start of code is after leading '{' */
    pCodeStart=codeString+2;

    /* Loop until space, '}', or NUL */
    for (pCodeChar=pCodeStart;
         *pCodeChar!=' ' && *pCodeChar!='}' && *pCodeChar;
         pCodeChar++) {
    }

    /* If terminated on NUL ... */
    if (*pCodeChar=='\0') {

        /* Not a valid attribute code */
        return code_Bad;

    }

    /* Temporarily terminate string at end of token */
    oldTerminator=*pCodeChar;
    *pCodeChar='\0';

    /* Find matching entry in code table, using case-insensitive
       string compare */
    pCodeEntry=codeTable;
    while (pCodeEntry->string) {

        if (stricmp(pCodeEntry->string, pCodeStart)==0) {

            break;

        }
        pCodeEntry++;
    }

    /* Restore character at end of token */
    *pCodeChar=oldTerminator;

    /* If code is good ... */
    if (pCodeEntry->code!=code_Bad) {

        /* End of code is character after terminator */
        *pEnd=pCodeChar;

    }

    /* Return matching code */
    return pCodeEntry->code;

    /* !!! Need to handle arguments in attribute codes !!! */

}

/*****************************************************************************
 *
 *  layoutText()    -   generate layout of text into text fragments
 *
 *****************************************************************************/

struct textFragment *layoutText(
    struct ClassLib *ClassBase,
    struct objectData *pInstanceData,
    SHORT *pWidth,
    SHORT *pHeight,
    struct DrawInfo *pDrawInfo
)
{

    WORD x, y;
    UWORD cx, cy;
    ULONG softStyle;
    LONG fgPen, bgPen;
    TJustification justify;
    BOOL fUnderscore;

    BOOL fEscape;

    STRPTR text;
    UBYTE *pThisChar, *pThisToken, *pCodeStart;
    UBYTE storeChar;
    enum textCode textCode;

    struct RastPort rastport;

    struct textFragment *pFragmentList;

    struct TextFont *pTextFont;

    UWORD maxX, maxY;

#ifdef DEBUG
    kprintf("layoutText(): Entry\n");
#endif /* DEBUG */

    /* Fail if no drawing information */
    if (!pDrawInfo) {

#ifdef DEBUG
        kprintf("   No DrawInfo\n");
#endif /* DEBUG */

        return NULL;

    }

    /* Initialize RastPort */
    InitRastPort(&rastport);

    /* Initialize fragment list */
    pFragmentList=NULL;

    /* Initialize position */
    x=0;
    y=0;

    /* Initialize soft style */
    softStyle=NULL;

    /* Initialize pens */
    fgPen=pInstanceData->od_TextPen;
    bgPen=pInstanceData->od_BackgroundPen;

    /* Initialize justification */
    justify=JustifyLeft;

    /* Initialize underscore flag */
    fUnderscore=FALSE;

    /* Initialize escape flag */
    fEscape=FALSE;

    /* Initialize extent */
    maxX=maxY=0;

    /* Fetch drawing info */
    pDrawInfo=pInstanceData->od_DrawInfo;

    /* Fetch (or open) font */
    if (pInstanceData->od_TextAttr) {

#ifdef DEBUG
        kprintf("   Using custom font (%s/%ld)\n",
            pInstanceData->od_TextAttr->ta_Name,
            pInstanceData->od_TextAttr->ta_YSize
        );
#endif /* DEBUG */
        pTextFont=OpenDiskFont(pInstanceData->od_TextAttr);
        if (!pTextFont) {

#ifdef DEBUG
        kprintf("   Error opening font\n");
#endif /* DEBUG */

            return(NULL);

        }

    } else {

        pTextFont=pDrawInfo->dri_Font;
#ifdef DEBUG
        kprintf("   Using default font (%s/%ld)\n",
            pTextFont->tf_Message.mn_Node.ln_Name,
            pTextFont->tf_YSize);
#endif /* DEBUG */

    }

    /* Set font of working rastport */
    SetFont(&rastport, pTextFont);

    /* Create working copy of text */
    text=createString(ClassBase,pInstanceData->od_Text);

    if (text) {

        /* Begin with first character */
        pThisChar=text;
        pThisToken=text;

        /* Loop until end of text */
        while (*pThisChar) {

            /* Switch on character */
            switch (*pThisChar) {

                /* Attribute code */
                case '@':

                    if (!fEscape) {

#ifdef DEBUG
                        kprintf("   Attribute\n");
#endif /* DEBUG */

                        /* Save pointer to start of code */
                        pCodeStart=pThisChar;

                        textCode=parseTextCode(pThisChar, &pThisChar);

                        /* If not bad code ... */
                        if (textCode!=code_Bad) {

                            /* Output fragment before attribute code */
                            *pCodeStart='\0';
#ifdef DEBUG
                            kprintf("   Attribute fragment: %s at (%ld, %ld) with style $%08lx\n",
                                pThisToken, x, y, softStyle);
#endif /* DEBUG */
                            cx=TextLength(&rastport, pThisToken,
                                strlen(pThisToken));
                            cy=pTextFont->tf_YSize;
                            if (
                                !newTextFragment(
                                    ClassBase,
                                    &pFragmentList,
                                    pThisToken,
                                    x, y,
                                    cx, cy,
                                    softStyle,
                                    fgPen,
                                    bgPen,
                                    fUnderscore
                                )
                            ) {

                                unlayoutText(ClassBase, pFragmentList);
                                if (pInstanceData->od_TextAttr) {

                                    CloseFont(pTextFont);

                                }
                                return NULL;

                            }
                            x+=cx;
                            maxX=max(maxX, x);

                            /* Switch on code */
                            switch (textCode) {

                                /* APen */
                                case code_APen:
                                    /* !!! */
                                    break;

                                /* Bold On */
                                case code_BoldOn:
#ifdef DEBUG
                                    kprintf("   BoldOn\n");
#endif /* DEBUG */
                                    softStyle|=FSF_BOLD;
                                    break;

                                /* Background */
                                case code_Background:
                                    /* !!! */
                                    break;

                                /* ClearTabs */
                                case code_ClearTabs:
                                    /* !!! */
                                    break;

                                /* Foreground */
                                case code_Foreground:
                                    /* !!! */
                                    break;

                                /* Italic On */
                                case code_ItalicOn:
#ifdef DEBUG
                                    kprintf("   ItalicOn\n");
#endif /* DEBUG */
                                    softStyle|=FSF_ITALIC;
                                    break;

                                /* Justify Center */
                                case code_JustifyCenter:
                                    /* !!! */
                                    break;

                                /* Justify Left */
                                case code_JustifyLeft:
                                    /* !!! */
                                    break;

                                /* Justify Right */
                                case code_JustifyRight:
                                    /* !!! */
                                    break;

                                /* Plain */
                                case code_Plain:
                                    softStyle=NULL;
                                    break;

                                /* SetTabs */
                                case code_SetTabs:
                                    /* !!! */
                                    break;

                                /* Tab */
                                case code_Tab:
                                    /* !!! */
                                    break;

                                /* Underline On */
                                case code_UnderlineOn:
#ifdef DEBUG
                                    kprintf("   UnderlineOn\n");
#endif /* DEBUG */
                                    softStyle|=FSF_UNDERLINED;
                                    break;

                                /* Bold Off */
                                case code_BoldOff:
#ifdef DEBUG
                                    kprintf("   BoldOff\n");
#endif /* DEBUG */
                                    softStyle&=~FSF_BOLD;
                                    break;

                                /* Italic Off */
                                case code_ItalicOff:
#ifdef DEBUG
                                    kprintf("   ItalicOff\n");
#endif /* DEBUG */
                                    softStyle&=~FSF_ITALIC;
                                    break;

                                /* Underline Off */
                                case code_UnderlineOff:
#ifdef DEBUG
                                    kprintf("   UnderlineOff\n");
#endif /* DEBUG */
                                    softStyle&=~FSF_UNDERLINED;
                                    break;

                            }

                            /* Set soft style of rastport */
                            SetSoftStyle(
                                &rastport,
                                softStyle,
                                AskSoftStyle(&rastport)
                            );

                            /* New token */
                            pThisToken=pThisChar+1;

                        }

                    }

                    break;

                /* Escape */
                case '\\':

                    /* Output this fragment */
                    *pThisChar='\0';

#ifdef DEBUG
                    kprintf("   Escape fragment: %s at (%ld, %ld) with style $%08lx\n",
                        pThisToken, x, y, softStyle);
#endif /* DEBUG */
                    cx=TextLength(&rastport, pThisToken,
                        strlen(pThisToken));
                    cy=pTextFont->tf_YSize;
                    if (
                        !newTextFragment(ClassBase, &pFragmentList,
                            pThisToken,
                            x, y,
                            cx, cy,
                            softStyle,
                            fgPen,
                            bgPen,
                            fUnderscore
                        )
                    ) {

                        unlayoutText(ClassBase, pFragmentList);
                        if (pInstanceData->od_TextAttr) {

                            CloseFont(pTextFont);

                        }
                        return NULL;

                    }
                    x+=cx;
                    maxX=max(maxX, x);

                    /* New fragment */
                    pThisToken=pThisChar+1;

                    /* Set escape flag */
                    fEscape=TRUE;

                    break;

                /* Newline */
                case '\n':

                    /* Output this fragment */
                    *pThisChar='\0';
#ifdef DEBUG
                    kprintf("   Newline fragment: %s at (%ld, %ld) with style $%08lx\n",
                        pThisToken, x, y, softStyle);
#endif /* DEBUG */
                    cx=TextLength(&rastport, pThisToken,
                        strlen(pThisToken));
                    cy=pTextFont->tf_YSize;
                    if (
                        !newTextFragment(ClassBase, &pFragmentList,
                            pThisToken,
                            x, y,
                            cx, cy,
                            softStyle,
                            fgPen,
                            bgPen,
                            fUnderscore
                        )
                    ) {

                        unlayoutText(ClassBase, pFragmentList);
                        if (pInstanceData->od_TextAttr) {

                            CloseFont(pTextFont);

                        }
                        return NULL;

                    }

                    /* Update extent */
                    x+=cx;
                    maxX=max(maxX, x);

                    /* Advance to next line */
                    y+=cy;
                    x=0;
                    maxY=max(maxY, y);

                    /* New token */
                    pThisToken=pThisChar+1;

                    break;

                /* Other */
                default:

                    /* If in underscore flag is set ... */
                    if (fUnderscore) {

                        /* Store next character */
                        storeChar=*(pThisChar+1);

                        /* Output this fragment */
                        *(pThisChar+1)='\0';
#ifdef DEBUG
                        kprintf("   Underscore fragment: %s at (%ld, %ld) with style $%08lx\n",
                            pThisToken, x, y, softStyle);
#endif /* DEBUG */
                        cx=TextLength(&rastport, pThisToken,
                            strlen(pThisToken));
                        cy=pTextFont->tf_YSize;
                        if (
                            !newTextFragment(ClassBase, &pFragmentList,
                                pThisToken,
                                x, y,
                                cx, cy,
                                softStyle,
                                fgPen,
                                bgPen,
                                fUnderscore
                            )
                        ) {

                            unlayoutText(ClassBase, pFragmentList);
                            if (pInstanceData->od_TextAttr) {

                                CloseFont(pTextFont);

                            }
                            return NULL;

                        }
                        x+=cx;
                        maxX=max(maxX, x);

                        /* New token */
                        pThisToken=pThisChar+1;

                        /* Restore next character */
                        *(pThisChar+1)=storeChar;

                        /* Clear underscore flag */
                        fUnderscore=FALSE;

                    /* ... else if underscore character found ... */
                    } else if (*pThisChar==pInstanceData->od_Underscore) {

                        /* Output this fragment */
                        *pThisChar='\0';

#ifdef DEBUG
                        kprintf("   Underscore break fragment: %s at (%ld, %ld) with style $%08lx\n",
                            pThisToken, x, y, softStyle);
#endif /* DEBUG */
                        cx=TextLength(&rastport, pThisToken,
                            strlen(pThisToken));
                        cy=pTextFont->tf_YSize;
                        if (
                            !newTextFragment(ClassBase, &pFragmentList,
                                pThisToken,
                                x, y,
                                cx, cy,
                                softStyle,
                                fgPen,
                                bgPen,
                                fUnderscore
                            )
                        ) {

                            unlayoutText(ClassBase, pFragmentList);
                            if (pInstanceData->od_TextAttr) {

                                CloseFont(pTextFont);

                            }
                            return NULL;

                        }
                        x+=cx;
                        maxX=max(maxX, x);

                        /* New token */
                        pThisToken=pThisChar+1;

                        /* Set underscore flag */
                        fUnderscore=TRUE;

                    }

                    break;

            }

            /* Go to next character */
            pThisChar++;

        }

        /* Output last fragment (if any) */
        if (*pThisToken) {

#ifdef DEBUG
            kprintf("   Final fragment: %s at (%ld, %ld) with style $%08lx\n",
                pThisToken, x, y, softStyle);
#endif /* DEBUG */

            cx=TextLength(&rastport, pThisToken,
                strlen(pThisToken));
            cy=pTextFont->tf_YSize;
            if (!newTextFragment(ClassBase, &pFragmentList,
                pThisToken,
                x, y,
                cx, cy,
                softStyle,
                fgPen,
                bgPen,
                fUnderscore
            )) {

                unlayoutText(ClassBase, pFragmentList);
                if (pInstanceData->od_TextAttr) {

                    CloseFont(pTextFont);

                }
                return NULL;

            }
            x+=cx;
            maxX=max(maxX, x);

        }

        /* Return width and height */
        *pWidth=maxX;
        *pHeight=maxY+pTextFont->tf_YSize;

        /* Release working copy of text */
        destroyString(ClassBase,text);

        /* Close font (if opened) ... */
        if (pInstanceData->od_TextAttr) {

            CloseFont(pTextFont);

        }

    }

    return pFragmentList;

}

/*****************************************************************************
 *
 *  unlayoutText()    -   release text fragment list created by layoutText()
 *
 *****************************************************************************/

void unlayoutText(
    struct ClassLib *ClassBase,
    struct textFragment *pFragmentList
)
{

    struct textFragment *pTextFragment, *pNextFragment;

    /* Release text fragment nodes */
    pNextFragment=pFragmentList;
    while (pTextFragment=pNextFragment) {

        pNextFragment=pTextFragment->next;

        destroyString(ClassBase,pTextFragment->text);
        FreeVec(pTextFragment);

    }

}

/*****************************************************************************
 *
 *  renderText()    -   render text fragment list created by layoutText()
 *
 *****************************************************************************/

BOOL renderText(
    struct ClassLib *ClassBase,
    struct objectData *pInstanceData,
    struct RastPort *rastPort,
    struct textFragment *pFragmentList,
    UWORD offsetX, UWORD offsetY,
    struct DrawInfo *pDrawInfo
)
{

    struct textFragment *pTextFragment;

    struct TextFont *pTextFont;
    UWORD x, y;
    ULONG softStyle;
    STRPTR text;

    UWORD underscoreX, underscoreY, underscoreW;

    /* Fetch (or open) font */
    if (pInstanceData->od_TextAttr) {

        pTextFont=OpenDiskFont(pInstanceData->od_TextAttr);
        if (!pTextFont) {

            return FALSE;

        }

    } else {

        pTextFont=pDrawInfo->dri_Font;

    }

    /* Set font of rastport */
    SetFont(rastPort, pTextFont);

    /* Set rastport to two-color rendering */
    SetDrMd(rastPort, JAM2);

    /* Loop through text fragments */
    pTextFragment=pFragmentList;
    while (pTextFragment) {

        /* Fetch position */
        x=pTextFragment->x;
        y=pTextFragment->y;

        /* Adjust Y-axis position for baseline */
        y+=pTextFont->tf_Baseline;

        /* Adjust position for offset */
        x+=offsetX;
        y+=offsetY;

        /* Fetch soft styles */
        softStyle=pTextFragment->softStyle;

        /* Fetch text */
        text=pTextFragment->text;

        /* Set soft styles */
        SetSoftStyle(
            rastPort,
            softStyle,
            AskSoftStyle(rastPort)
        );

        /* Set foreground and background pens */
        SetAPen(rastPort, pTextFragment->fgPen);
        SetBPen(rastPort, pTextFragment->bgPen);

        /* Position rendering cursor */
        Move(rastPort,
            x,
            y
        );

        /* Output text fragment */
        Text(
            rastPort,
            text,
            strlen(text)
        );

        /* If fragment is underscored ... */
        if (pTextFragment->fUnderscore) {

            underscoreW=TextLength(
                rastPort, text, strlen(text)
            );
            underscoreX=x;
            underscoreY=y+LABEL_UNDERSCORE_YOFFSET;

            Move(rastPort, underscoreX, underscoreY);
            Draw(rastPort, underscoreX+underscoreW-1, underscoreY);

        }

        /* Render next text fragment */
        pTextFragment=pTextFragment->next;

    }

    /* Close font (if opened) ... */
    if (pInstanceData->od_TextAttr) {

        CloseFont(pTextFont);

    }

    /* Success */
    return TRUE;

}
