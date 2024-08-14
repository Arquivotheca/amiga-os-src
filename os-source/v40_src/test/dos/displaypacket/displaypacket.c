#include <exec/types.h>
#include <exec/memory.h>
#include <graphics/text.h>
#include <graphics/gfxbase.h>
#include <intuition/intuition.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/filehandler.h>
#include <devices/trackdisk.h>
#include "Derror.h"
#include "pkt.h"
#include "test.h"

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/alib_protos.h>

#include "displayPacket_protos.h"
#include "packmoncommon_protos.h"

#include <stdlib.h>
#include <stdio.h>  /* only for sprintf() */
#include <string.h>

extern struct Library *DOSBase;

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/graphics_pragmas.h>

#define RES1_ARGNUM  -1
#define RES2_ARGNUM  -2

/* packet types */
#define _ACTION_NIL                ACTION_NIL                   /*  0L */
#define _ACTION_GET_BLOCK          ACTION_GET_BLOCK             /*  2L */
#define _ACTION_SET_MAP            ACTION_SET_MAP               /*  4L */
#define _ACTION_DIE                ACTION_DIE                   /*  5L */
#define _ACTION_EVENT              ACTION_EVENT                 /*  6L */
#define _ACTION_CURRENT_VOLUME     ACTION_CURRENT_VOLUME        /*  7L */
#define _ACTION_LOCATE_OBJECT      ACTION_LOCATE_OBJECT         /*  8L */
#define _ACTION_RENAME_DISK        ACTION_RENAME_DISK           /*  9L */
#define _ACTION_FREE_LOCK         ACTION_FREE_LOCK              /* 15L */
#define _ACTION_DELETE_OBJECT     ACTION_DELETE_OBJECT          /* 16L */
#define _ACTION_RENAME_OBJECT     ACTION_RENAME_OBJECT          /* 17L */
#define _ACTION_MORE_CACHE        ACTION_MORE_CACHE             /* 18L */
#define _ACTION_COPY_DIR          ACTION_COPY_DIR               /* 19L */
#define _ACTION_WAIT_CHAR         ACTION_WAIT_CHAR              /* 20L */
#define _ACTION_SET_PROTECT       ACTION_SET_PROTECT            /* 21L */
#define _ACTION_CREATE_DIR        ACTION_CREATE_DIR             /* 22L */
#define _ACTION_EXAMINE_OBJECT    ACTION_EXAMINE_OBJECT         /* 23L */
#define _ACTION_EXAMINE_NEXT      ACTION_EXAMINE_NEXT           /* 24L */
#define _ACTION_DISK_INFO         ACTION_DISK_INFO              /* 25L */
#define _ACTION_INFO              ACTION_INFO                   /* 26L */
#define _ACTION_FLUSH             ACTION_FLUSH                  /* 27L */
#define _ACTION_SET_COMMENT       ACTION_SET_COMMENT            /* 28L */
#define _ACTION_PARENT            ACTION_PARENT                 /* 29L */

/* This is normally a returning timer device request. (internal) */
#define _ACTION_TIMER             ACTION_TIMER                  /* 30L */

#define _ACTION_INHIBIT           ACTION_INHIBIT                /* 31L */
#define _ACTION_DISK_TYPE         ACTION_DISK_TYPE              /* 32L */
#define _ACTION_DISK_CHANGE       ACTION_DISK_CHANGE            /* 33L */
#define _ACTION_SET_FILE_DATE     ACTION_SET_DATE               /* 34L     */
#define _ACTION_READ              ACTION_READ                   /* 82L 'R' */
#define _ACTION_WRITE             ACTION_WRITE                  /* 87L 'W' */
#define _ACTION_SET_SCREEN_MODE   ACTION_SCREEN_MODE            /* 994L */
/*
 * When a handler internally sends a device i/o request they are sent using
 * their process port and in the form of a "packet" the packet types below:
 * (ACTION_TIMER above also falls into this category)
 */
#define _ACTION_READ_INTERNAL   ACTION_READ_RETURN          /* 1001L */
#define _ACTION_WRITE_INTERNAL  ACTION_WRITE_RETURN         /* 1002L */

#define _ACTION_FIND_INPUT      ACTION_FINDINPUT            /* 1005L */
#define _ACTION_FIND_OUTPUT     ACTION_FINDOUTPUT           /* 1006L */
#define _ACTION_CLOSE           ACTION_END                  /* 1007L */
#define _ACTION_SEEK            ACTION_SEEK                 /* 1008L */
#define _ACTION_IS_FILESYSTEM   ACTION_IS_FILESYSTEM        /* 1027L */

/* From this point onwards, we'll use the real ACTION_WHATEVERS instead
   of the above junk.  Aargh.
 */


/* Comments?  Don't ask me, I maintain it, I didn't write it. */
#define D_STEP                  1
#define D_MODIFY                2
#define D_SELECTIVE             3
#define D_SINGLE                4
#define D_ERROR                 5
#define D_PACKET                6

/* Display coordinates for Move(), Text(), etc. */
#define S_data_left             81
#define S_packet_type           79
#define S_arg1                  93              
#define S_arg2                  103     /* 107 */
#define S_arg3                  113     /* 121 */
#define S_arg4                  123     /* 135 */
#define S_arg5                  133   /* kludge! */
#define S_arg6                  143   /* kludge! */
#define S_arg7                  153   /* kludge! */
#define S_res1                  163   /* kludge! */
#define S_res2                  173   /* kludge! */
               


/*
 *   Output generic packet info and  ID string
 *
 */


#define HOST_PORT_NAME   "packet_info"

struct Window *w1 = 0;
struct Window *w2 = 0;
struct RastPort *rp1;
struct RastPort *rp2;

struct IntuiMessage *message;
struct IntuiMessage *message1;

struct TextFont *textfont = 0;
struct TextFont *textfont1 = 0;


struct HAND_MESSAGE *h_message;
struct MsgPort *HandDataPort=0;


struct MsgPort *setup_port();
extern VOID kprintf(STRPTR,...);
extern VOID KPrintF(STRPTR,...);

ULONG dos_error = 0;
UWORD selective_display =0;

#define P_H_size        5000
UBYTE *packet_hit = 0;

char txt[48];


extern struct Library *IntuitionBase; 
struct Library *GfxBase = 0;


SHORT BorderVectors5[] = {
        0,0,
        58,0,
        58,26,
        0,26,
        0,0
};
struct Border Border5 = {
        -2,-1,  /* XY origin relative to container TopLeft */
        2,0,JAM1,       /* front pen, back pen and drawmode */
        5,      /* number of XY vectors */
        BorderVectors5, /* pointer to XY vectors */
        NULL    /* next border in list */
};

struct IntuiText IText15a = {
        1,0,JAM2,       /* front and back text pens, drawmode and fill byte */
        5,14,   /* XY origin relative to container TopLeft */
        &TOPAZ80,       /* font pointer or NULL for default */
        "PACKET",       /* pointer to text */
        NULL    /* next IntuiText structure */
};

struct IntuiText IText15 = {
        1,0,JAM2,       /* front and back text pens, drawmode and fill byte */
        4,4,    /* XY origin relative to container TopLeft */
        &TOPAZ80,       /* font pointer or NULL for default */
        "SELECT",       /* pointer to text */
        &IText15a       /* next IntuiText structure */
};

struct Gadget Gadget6 = {
        NULL,   /* next gadget */
        16,103, /* origin XY of hit box relative to window TopLeft */
        55,25,  /* hit box width and height */
        NULL,   /* gadget flags */
        RELVERIFY,      /* activation flags */
        BOOLGADGET,     /* gadget type flags */
        (APTR)&Border5, /* gadget border or image to be rendered */
        NULL,   /* alternate imagery for selection */
        &IText15,       /* first IntuiText structure */
        NULL,   /* gadget mutual-exclude long word */
        NULL,   /* SpecialInfo structure */
        D_PACKET,       /* user-definable data */
        NULL    /* pointer to user-definable data */
};



SHORT BorderVectors6[] = {
        0,0,
        58,0,
        58,26,
        0,26,
        0,0
};


struct Border Border6 = {
        -2,-1,  /* XY origin relative to container TopLeft */
        2,0,JAM1,       /* front pen, back pen and drawmode */
        5,      /* number of XY vectors */
        BorderVectors6, /* pointer to XY vectors */
        NULL    /* next border in list */
};

struct IntuiText IText16a = {
        1,0,JAM2,       /* front and back text pens, drawmode and fill byte */
        8,14,   /* XY origin relative to container TopLeft */
        &TOPAZ80,       /* font pointer or NULL for default */
        "ERROR",        /* pointer to text */
        NULL    /* next IntuiText structure */
};

struct IntuiText IText16 = {
        1,0,JAM2,       /* front and back text pens, drawmode and fill byte */
        3,4,    /* XY origin relative to container TopLeft */
        &TOPAZ80,       /* font pointer or NULL for default */
        "SELECT",       /* pointer to text */
        &IText16a       /* next IntuiText structure */
};

struct Gadget Gadget5 = {
        &Gadget6,       /* next gadget */
        16,73,  /* origin XY of hit box relative to window TopLeft */
        55,25,  /* hit box width and height */
        NULL,   /* gadget flags */
        RELVERIFY,      /* activation flags */
        BOOLGADGET,     /* gadget type flags */
        (APTR)&Border6, /* gadget border or image to be rendered */
        NULL,   /* alternate imagery for selection */
        &IText16,       /* first IntuiText structure */
        NULL,   /* gadget mutual-exclude long word */
        NULL,   /* SpecialInfo structure */
        D_ERROR,        /* user-definable data */
        NULL    /* pointer to user-definable data */
};



SHORT BorderVectors1[] = {
        0,0,
        41,0,
        41,34,
        0,34,
        0,0
};
struct Border Border1 = {
        -2,-1,  /* XY origin relative to container TopLeft */
        2,0,JAM1,       /* front pen, back pen and drawmode */
        5,      /* number of XY vectors */
        BorderVectors1, /* pointer to XY vectors */
        NULL    /* next border in list */
};

struct IntuiText IText1 = {
        1,0,JAM2,       /* front and back text pens, drawmode and fill byte */
        3,13,   /* XY origin relative to container TopLeft */
        &TOPAZ80,       /* font pointer or NULL for default */
        "STEP", /* pointer to text */
        NULL    /* next IntuiText structure */
};

struct Gadget Gadget4 = {
        &Gadget5,       /* next gadget */
        18,145, /* origin XY of hit box relative to window TopLeft */
        38,33,  /* hit box width and height */
        NULL,   /* gadget flags */
        RELVERIFY,      /* activation flags */
        BOOLGADGET,     /* gadget type flags */
        (APTR)&Border1, /* gadget border or image to be rendered */
        NULL,   /* alternate imagery for selection */
        &IText1,        /* first IntuiText structure */
        NULL,   /* gadget mutual-exclude long word */
        NULL,   /* SpecialInfo structure */
        D_STEP, /* user-definable data */
        NULL    /* pointer to user-definable data */
};

SHORT BorderVectors2[] = {
        0,0,
        112,0,
        112,21,
        0,21,
        0,0
};
struct Border Border2 = {
        -2,-1,  /* XY origin relative to container TopLeft */
        2,0,JAM1,       /* front pen, back pen and drawmode */
        5,      /* number of XY vectors */
        BorderVectors2, /* pointer to XY vectors */
        NULL    /* next border in list */
};

struct IntuiText IText2 = {
        1,0,JAM2,       /* front and back text pens, drawmode and fill byte */
        3,7,    /* XY origin relative to container TopLeft */
        &TOPAZ80,       /* font pointer or NULL for default */
        "MODIFY PACKET",        /* pointer to text */
        NULL    /* next IntuiText structure */
};

struct Gadget Gadget3 = {
        &Gadget4,       /* next gadget */
        265,39, /* origin XY of hit box relative to window TopLeft */
        109,20, /* hit box width and height */
        GADGDISABLED,   /* gadget flags */
        RELVERIFY ,     /* activation flags */
        BOOLGADGET,     /* gadget type flags */
        (APTR)&Border2, /* gadget border or image to be rendered */
        NULL,   /* alternate imagery for selection */
        &IText2,        /* first IntuiText structure */
        NULL,   /* gadget mutual-exclude long word */
        NULL,   /* SpecialInfo structure */
        D_MODIFY,       /* user-definable data */
        NULL    /* Pointer to user-definable data */
};

SHORT BorderVectors3[] = {
        0,0,
        144,0,
        144,21,
        0,21,
        0,0
};
struct Border Border3 = {
        -2,-1,  /* XY origin relative to container TopLeft */
        2,0,JAM1,       /* front pen, back pen and drawmode 3,0,JAM1, */
        5,      /* number of XY vectors */
        BorderVectors3, /* pointer to XY vectors */
        NULL    /* next border in list */
};

struct IntuiText IText3 = {
        1,0,JAM2,       /* front and back text pens, drawmode and fill byte 3,0,JAM2 */
        2,7,    /* XY origin relative to container TopLeft */
        &TOPAZ80,       /* font pointer or NULL for default */
        "SELECTIVE DISPLAY",    /* pointer to text */
        NULL    /* next IntuiText structure */
};

struct Gadget Gadget2 = {
        &Gadget3,       /* next gadget */
        117,39, /* origin XY of hit box relative to window TopLeft */
        141,20, /* hit box width and height */
        NULL,   /* gadget flags */
        RELVERIFY+TOGGLESELECT, /* activation flags */
        BOOLGADGET,     /* gadget type flags */
        (APTR)&Border3, /* gadget border or image to be rendered */
        NULL,   /* alternate imagery for selection */
        &IText3,        /* first IntuiText structure */
        NULL,   /* gadget mutual-exclude long word */
        NULL,   /* SpecialInfo structure */
        D_SELECTIVE,    /* user-definable data */
        NULL    /* pointer to user-definable data */
};

SHORT BorderVectors4[] = {
        0,0,
        97,0,
        97,21,
        0,21,
        0,0
};
struct Border Border4 = {
        -2,-1,  /* XY origin relative to container TopLeft */
        2,0,JAM1,       /* front pen, back pen and drawmode */
        5,      /* number of XY vectors */
        BorderVectors4, /* pointer to XY vectors */
        NULL    /* next border in list */
};

struct IntuiText IText4 = {
        1,0,JAM2,       /* front and back text pens, drawmode and fill byte */
        4,7,    /* XY origin relative to container TopLeft */
        &TOPAZ80,       /* font pointer or NULL for default */
        "SINGLE STEP",  /* pointer to text */
        NULL    /* next IntuiText structure */
};

struct Gadget Gadget1 = {
        &Gadget2,       /* next gadget */
        16,39,  /* origin XY of hit box relative to window TopLeft */
        94,20,  /* hit box width and height */
        NULL,   /* gadget flags */
        RELVERIFY+TOGGLESELECT, /* activation flags */
        BOOLGADGET,     /* gadget type flags */
        (APTR)&Border4, /* gadget border or image to be rendered */
        NULL,   /* alternate imagery for selection */
        &IText4,        /* first IntuiText structure */
        NULL,   /* gadget mutual-exclude long word */
        NULL,   /* SpecialInfo structure */
        D_SINGLE,       /* user-definable data */
        NULL    /* pointer to user-definable data */
};

#define GadgetList1 Gadget1

struct IntuiText IText5 = {
        3,1,COMPLEMENT, /* front and back text pens, drawmode and fill byte */
        0,0,    /* XY origin relative to container TopLeft */
        &TOPAZ80,       /* font pointer or NULL for default */
        "dummy",        /* pointer to text */
        NULL    /* next IntuiText structure */
};

struct MenuItem MenuItem1 = {
        NULL,   /* next MenuItem structure */
        0,0,    /* XY of Item hitbox relative to TopLeft of parent hitbox */
        40,8,   /* hit box width and height */
        ITEMTEXT+HIGHCOMP,      /* Item flags */
        0,      /* each bit mutually-excludes a same-level Item */
        (APTR)&IText5,  /* Item render  (IntuiText or Image or NULL) */
        NULL,   /* Select render */
        NULL,   /* alternate command-key */
        NULL,   /* SubItem list */
        MENUNULL        /* filled in by Intuition for drag selections */
};

struct Menu Menu1 = {
        NULL,   /* next Menu structure */
        0,0,    /* XY origin of Menu hit box relative to screen TopLeft */
        12,0,   /* Menu hit box width and height */
        MENUENABLED,    /* Menu flags */
        "Arg1", /* text of Menu name */
        &MenuItem1      /* MenuItem linked list pointer */
};

#define MenuList1 Menu1

struct IntuiText IText14 = {
        1,0,JAM2,       /* front and back text pens, drawmode and fill byte */
        316,187,        /* XY origin relative to container TopLeft */
        &TOPAZ80,       /* font pointer or NULL for default */
        "Replyed",      /* pointer to text */
        NULL    /* next IntuiText structure */
};

struct IntuiText IText13 = {
        1,0,JAM2,       /* front and back text pens, drawmode and fill byte */
        259,187,        /* XY origin relative to container TopLeft */
        &TOPAZ80,       /* font pointer or NULL for default */
        "Sent", /* pointer to text */
        &IText14        /* next IntuiText structure */
};

struct IntuiText IText12 = {
        1,0,JAM2,       /* front and back text pens, drawmode and fill byte */
        150,187,        /* XY origin relative to container TopLeft */
        &TOPAZ80,       /* font pointer or NULL for default */
        "Intercepted",  /* pointer to text */
        &IText13        /* next IntuiText structure */
};

struct IntuiText IText11 = {
        1,0,JAM2,       /* front and back text pens, drawmode and fill byte */
        10,186, /* XY origin relative to container TopLeft */
        &TOPAZ80,       /* font pointer or NULL for default */
        "Packet Status",        /* pointer to text */
        &IText12        /* next IntuiText structure */
};

struct IntuiText IText10 = {
        1,0,JAM2,       /* front and back text pens, drawmode and fill byte */
        83,124, /* XY origin relative to container TopLeft */
        &TOPAZ80,       /* font pointer or NULL for default */
        "Res1", /* pointer to text */
        &IText11        /* next IntuiText structure */
};

struct IntuiText IText9 = {
        1,0,JAM2,       /* front and back text pens, drawmode and fill byte */
        82,108, /* XY origin relative to container TopLeft */
        &TOPAZ80,       /* font pointer or NULL for default */
        "Arg3", /* pointer to text */
        &IText10        /* next IntuiText structure */
};

struct IntuiText IText8 = {
        1,0,JAM2,       /* front and back text pens, drawmode and fill byte */
        82,94,  /* XY origin relative to container TopLeft */
        &TOPAZ80,       /* font pointer or NULL for default */
        "Arg2", /* pointer to text */
        &IText9 /* next IntuiText structure */
};

struct IntuiText IText7 = {
        1,0,JAM2,       /* front and back text pens, drawmode and fill byte */
        81,79,  /* XY origin relative to container TopLeft */
        &TOPAZ80,       /* font pointer or NULL for default */
        "Arg1", /* pointer to text */
        &IText8 /* next IntuiText structure */
};

struct IntuiText IText6 = {
        1,0,JAM2,       /* front and back text pens, drawmode and fill byte */
        137,18, /* XY origin relative to container TopLeft */
        &TOPAZ80,       /* font pointer or NULL for default */
        "Packet Monitor",       /* pointer to text */
        &IText7 /* next IntuiText structure */
};

/* This has already been defined in Derror.h.  It doesn't seem to be used,
   so kill it here.

#define IntuiTextList1 IText6
*/

struct NewWindow nw1 = {
        0,0,    /* window XY origin relative to TopLeft of screen */
        414,200,        /* window width and height */
        0,1,    /* detail and block pens */
        GADGETUP,       /* IDCMP flags */
        WINDOWDRAG+WINDOWDEPTH, /* other window flags */
        &Gadget1,       /* first gadget in gadget list */
        NULL,   /* custom CHECKMARK imagery */
        NULL,   /* window title */
        NULL,   /* custom screen pointer */
        NULL,   /* custom bitmap */
        640,200,        /* minimum width and height */
        640,200,        /* maximum width and height */
        WBENCHSCREEN    /* destination screen type */
};


long    step = 0;
long    single_step = 0;
long    code;

void handle_gadgets();


char *s_arg;


main(int argc, UBYTE *argv[]) {
        ULONG   handbit;
        ULONG   Imask;
        ULONG   waitmask;
        struct  HAND_MESSAGE *h_message=NULL; 

        IntuitionBase = 0;
        if ((IntuitionBase = OpenLibrary("intuition.library",0l)) == NULL) 
                eexit(20);
        if ((GfxBase = OpenLibrary("graphics.library",0l)) == NULL) 
                eexit(20);

        s_arg = argv[1];

/*kprintf("%s\n",s_arg);*/


        if (!(packet_hit = AllocMem(P_H_size,MEMF_PUBLIC|MEMF_CLEAR)))
                eexit(20);

        if (!(w1 = (struct Window *)OpenWindow( &nw1)))
                eexit(20);
        rp1 = w1->RPort;                /* establish its rastport for later */


        if (textfont = OpenFont(&TOPAZ80))
                SetFont(rp1,textfont);

        if (!(HandDataPort = setup_port()))
                eexit(20);
                


        Execute("mount test1: from testmountlist",NULL,NULL);
/*      Delay(50);
        Execute("cd test:",NULL,NULL);
*/

        handbit = 1L<<HandDataPort->mp_SigBit;
        Imask   = 1L << w1->UserPort->mp_SigBit;
        waitmask = Imask | handbit; 

        {
        int quit;
        for (quit = 0; !quit; )
                {
                if (selective_display) {
                        grab_until_hit(handbit,h_message,HandDataPort);
                        /* note that h_message hasn't been initted: correct? */
                }

                else
                        {
                        Wait(waitmask); /* wait for  DOS packet activity */
                        handle_gadgets();
                        if (h_message = (struct HAND_MESSAGE *)GetMsg(HandDataPort))
                                execute_command(h_message);
                        }

                step = 0;
                if (single_step && !(step)) /* If single-stepping, wait for step */
                        {
                        while((!step) && single_step)
                                {
                                Wait(Imask); 
                                handle_gadgets();
                                }
                        }
                if (h_message)
                        {
                        /*kprintf("main loop reply DISPLAY PACKET \n");*/
                        ReplyMsg((struct Message *)h_message);
                        h_message = 0; /* not really necessary but... */
                        }

                step = 0;


                }
        eexit(0);
        }
}

VOID grab_until_hit (ULONG waitm, struct HAND_MESSAGE *h_message, struct MsgPort  *HandDataPort) {
        int hit = 0;

        /*kprintf("g U H\n");*/

        do
                {
                Wait(waitm); 
                handle_gadgets();
                if (h_message = (struct HAND_MESSAGE *)GetMsg(HandDataPort))
                        hit = execute_command(h_message);
                if ((!hit) && h_message)
                        ReplyMsg((struct Message *)h_message);
                if (!selective_display)
                                return;

                }
        while(!hit);
}


VOID handle_gadgets(VOID) {

    while ((message = (struct IntuiMessage *)GetMsg(w1->UserPort))) {
        ULONG class = message->Class; 
        code = message->Code;
        if (class != GADGETUP) {
            ReplyMsg((struct Message *)message); 
        }

        switch (class) {
            case GADGETUP:
            {
                ULONG gid = (((struct Gadget *)message->IAddress)->GadgetID);
                UWORD select = (((struct Gadget *)message->IAddress)->Flags) && SELECTED;                       
                ReplyMsg((struct Message *)message); 
                switch (gid) {

                    case D_STEP:
                        step = 1;
                        break;
                    case D_ERROR:
                    {
                        int quit = !0;

                        if (!(w2 = (struct Window *)OpenWindow( &nw2))) {
                            eexit(10);
                        }

                        rp2 = w2->RPort;
                        if (textfont1 = OpenFont(&TOPAZ80)) {
                            SetFont(rp2,textfont1);
                        }
                        SetAPen(rp2,1L);
                        Move(rp2,8,8);
                        sprintf(txt,"%s","Error returned by Selected Packets"); 
                        Text(rp2,txt,34);
                /*      sprintf(txt,"%s","Other Error Code"); 
                        Move(rp2,358,188);
                        Text(rp2,txt,16);
                */
                        while (quit) {
                            WaitPort(w2->UserPort);
                            while((message1 = (struct IntuiMessage *)GetMsg(w2->UserPort))) {
                                switch(message1->Class) {
                                    case GADGETUP:
                                        quit = 0;
                                        dos_error = (((struct Gadget *)message1->IAddress)->GadgetID);
                                        if (dos_error == MANUAL_ENTRY) {
                                            dos_error = ((struct StringInfo *)(((struct Gadget *)message1->IAddress)->SpecialInfo))->LongInt;
                                            break;
                                        }
                                        break;
                                    default:
                                        break;
                                }
                                ReplyMsg((struct Message *)message1); 
                            }
                        } /* while quit */
                    }       
                        CloseWindow(w2);
                        if (textfont1) {
                                CloseFont(textfont1);
                                textfont1 = 0;
                        }
                        break;

                    case D_PACKET:
                      {
                        int quit = !0;

                        if (!(w2 = (struct Window *)OpenWindow( &nw3)))
                                eexit(10);
                        rp2 = w2->RPort;
                        if (textfont1 = OpenFont(&TOPAZ80))
                                SetFont(rp2,textfont1);
                        SetAPen(rp2,1L);
                        Move(rp2,8,8);
                        sprintf(txt,"%s","Select Packet(s) then EXIT"); 
                        Text(rp2,txt,26);

                        while (quit) {
                            WaitPort(w2->UserPort);
                            while ((message1 = (struct IntuiMessage *)GetMsg(w2->UserPort))) {
                                switch (message1->Class) {
                                    ULONG g;

                                    case GADGETUP:
                                            g = (((struct Gadget *)message1->IAddress)->GadgetID);
                                            if (g == P_EXIT) {
                                                quit = 0;
                                            }
                                            else {
                                                if (g > P_H_size) {
                                                    packet_hit[0] = !0;
                                                }
                                                else {
                                                    packet_hit[g] = !0;
                                                }
                                            }
                                        break;
                                    default:
                                        break;
                                }
                                ReplyMsg((struct Message *)message1); 
                            }
                        } /* while quit */

                      }  /* scope */
                        CloseWindow(w2);
                        if (textfont1) {
                                CloseFont(textfont1);
                                textfont1 = 0;
                        }
                        break;
    
                    case D_SELECTIVE:
                        selective_display = select;
                        break;                  
                    case D_SINGLE:
                        if (single_step) {
                            step = 1;
                        }
                        single_step = select;
                        break;
                }  /* switch gid */
            } /* case GADGETUP */
        }  /* switch class */
    }  /* while msg = GetMsg */
}




VOID eexit(LONG rcode) {
    if (packet_hit) {
        FreeMem(packet_hit,P_H_size);
    }
    if (HandDataPort) {
        deletePort(HandDataPort);
    }
    if (textfont) {
        CloseFont(textfont);
    }
    if (textfont1) {
        CloseFont(textfont1);
    }
    if (w2) {
        CloseWindow(w2);
    }
    if (w1) {
        CloseWindow(w1);
    }
    if (IntuitionBase) {
        CloseLibrary(IntuitionBase);
    }
    if (GfxBase) {
        CloseLibrary(GfxBase);
    }

    exit(rcode);
}


struct MsgPort *setup_port(VOID) {
   struct MsgPort *the_port;

   Forbid();

   /* look for someone else that looks just like us! */
   if (FindPort("Test_Handler")) {
       Permit();
       return(NULL);
   }

   /* allocate the port */
   the_port = createPort("Test_Handler",0L);

   Permit();

   return(the_port);
}

int execute_command(struct HAND_MESSAGE *h_message) {

   int  done = 0;
   struct DosPacket *packet;

        switch (h_message->m_type) {

            case STARTUP:
                h_message->pid = (ULONG)DeviceProc(s_arg);
                /*kprintf("in DISP  startup %ld\n",h_message->pid);*/
                break;

            case SENDMG:
                packet = h_message->pkt_pointer;
                /*kprintf("DISPLAY packet %ld",packet);*/

                if (selective_display) {
                    if (packet_hit[packet->dp_Type]) {
                        done = !0;
                    }
                }

                h_message->error = givepkt(packet);
                break;
            default:
                break;
        }

        return(done);
}

/* printtype ================================================================
 * prints packet type to window
 */
void printtype(STRPTR string) {
    char buff[64];

    SetAPen(rp1,1L);
    /* KPrintF("printtype($%08lx);\n", string); */
    sprintf(buff,"%-25s%12s",string,"         HEX");
    Move(rp1,S_data_left,S_packet_type);
    Text(rp1,buff,37);

    return;
}

/* printarg ========================================================================
   Print a single argument
       string - descriptive name
       packet - packet with args
       arg_num- where to print it (as arg1..7) etc.
 */
VOID printarg(char *string, struct DosPacket *packet, int arg_num) {
    char buff[64];  

    SetAPen(rp1,1L);
    /* KPrintF("printarg($%08lx,$%08lx,$%08lx);\n", string, packet, arg_num); */

    switch(arg_num) {
        case 1:
            /*sprintf(&buff[0],"%-17s %.10ld %08x",string,packet->dp_Arg1,packet->dp_Arg1); */
            sprintf(&buff[0],"%-17s            %08x",string,packet->dp_Arg1);
            Move(rp1,S_data_left,S_arg1);
            break;

        case 2:
            /*sprintf(&buff[0],"%-17s %.10ld %08x",string,packet->dp_Arg2,packet->dp_Arg2);*/
            sprintf(&buff[0],"%-17s            %08x",string,packet->dp_Arg2);
            Move(rp1,S_data_left,S_arg2);
            break;

        case 3:
            /*sprintf(&buff[0],"%-17s %.10ld %08x",string,packet->dp_Arg3,packet->dp_Arg3);*/
            sprintf(&buff[0],"%-17s            %08x",string,packet->dp_Arg3);
            Move(rp1,S_data_left,S_arg3);
            break;

        case 4:
            sprintf(&buff[0],"%-17s            %08x",string,packet->dp_Arg4);
            Move(rp1,S_data_left,S_arg4);
            break;

        case 5:
            sprintf(&buff[0],"%-17s            %08x",string,packet->dp_Arg5);
            Move(rp1,S_data_left,S_arg5);
            break;

        case 6:
            sprintf(&buff[0],"%-17s            %08x",string,packet->dp_Arg6);
            Move(rp1,S_data_left,S_arg6);
            break;

        case 7:
            sprintf(&buff[0],"%-17s            %08x", string, packet->dp_Arg7);
            Move(rp1,S_data_left,S_arg7);
            break;

        case RES1_ARGNUM:
            sprintf(&buff[0],"%-17s            %08x", string, packet->dp_Res1);
            Move(rp1,S_data_left,S_res1);
            break;

        case RES2_ARGNUM:
            sprintf(&buff[0],"%-17s            %08x", string, packet->dp_Res2);
            Move(rp1,S_data_left,S_res2);
            break;

        default:
            return;
            break;
    }

    Text(rp1,&buff[0],37);

    return;
}
        
        
/* printall ==========================================================================
   Prints contents of packet to window
   Inputs: 
        string - descriptive bit, for example, "ACTION_EXAMINE"
        packet - packet to print
 */
VOID printall(char *string, struct DosPacket *packet) {
    char buff[640];
    char *buf = &buff[0];

    SetAPen(rp1,1L);

/*  KPrintF("printall($%08lx, $%08lx);\n", string, packet); */

    sprintf(buf,"%-25s%12s",string,"         HEX"); 
    Move(rp1,S_data_left,S_packet_type);
    Text(rp1,buf,37);


    /*sprintf(buf,"%-17s %.10ld %08x","Arg1",packet->dp_Arg1,packet->dp_Arg1);*/
    sprintf(buf, "%-17s            %08x", "Arg1", packet->dp_Arg1);    
    Move(rp1,S_data_left,S_arg1);
    Text(rp1,buf,37);

    /*sprintf(buf,"%-17s %.10ld %08x","Arg2",packet->dp_Arg2,packet->dp_Arg2);*/
    sprintf(buf,"%-17s            %08x", "Arg2", packet->dp_Arg2);    
    Move(rp1,S_data_left,S_arg2);
    Text(rp1,buf,37);

    /*sprintf(buf,"%-17s %.10ld %08x","Arg3",packet->dp_Arg3,packet->dp_Arg3);*/
    sprintf(buf,"%-17s            %08x", "Arg3", packet->dp_Arg3);    
    Move(rp1,S_data_left,S_arg3);
    Text(rp1,buf,37);

    sprintf(buf,"%-17s            %08x", "Arg4", packet->dp_Arg4);
    Move(rp1,S_data_left,S_arg4);
    Text(rp1,buf,37);

/* all stuff below was added 05 May 1993 */
    sprintf(buf,"%-17s            %08x", "Arg5", packet->dp_Arg5);    
    Move(rp1,S_data_left,S_arg5);
    Text(rp1,buf,37);

    sprintf(buf,"%-17s            %08x", "Arg6", packet->dp_Arg6);
    Move(rp1,S_data_left,S_arg6);
    Text(rp1,buf,37);

    sprintf(buf,"%-17s            %08x", "Arg7", packet->dp_Arg7);    
    Move(rp1,S_data_left,S_arg7);
    Text(rp1,buf,37);

    sprintf(buf,"%-17s            %08x", "Res1", packet->dp_Res1);
    Move(rp1,S_data_left,S_res1);
    Text(rp1,buf,37);

    sprintf(buf,"%-17s            %08x", "Res2", packet->dp_Res2);    
    Move(rp1,S_data_left,S_res2);
    Text(rp1,buf,37);


    return;
}


/* printB ====================================================================
 * Print BSTRing to window
 * Inputs:
 *       string - descriptive string (ie "Name", "Old lock", etc.)
 *     abstring - BSTRring, as a LONG (ie, dp_ArgX)
 *            i - packet arg number (dp_Arg1, etc)
 */
VOID printB(STRPTR string, LONG abstring, int i) {
    char buff[64];
    char no_b[256]; 
    STRPTR bstring = (STRPTR)abstring;


    SetAPen(rp1,2L);

/*  KPrintF("printB($%08lx, $%08lx, $%08lx);\n", string, abstring, i); */

    btos(bstring, &no_b[0]);
    sprintf(&buff[0],"%-6s %-31s",string,&no_b[0]); 
    switch(i) {
        case 1:
            Move(rp1,S_data_left,S_arg1);
            break;
        case 2:
            Move(rp1,S_data_left,S_arg2);
            break;
        case 3:
            Move(rp1,S_data_left,S_arg3);
            break;
        case 4:
            Move(rp1,S_data_left,S_arg4);
            break;
        case 5:
            Move(rp1,S_data_left,S_arg5);
            break;
        case 6:
            Move(rp1,S_data_left,S_arg6);
            break;
        case 7:
            Move(rp1,S_data_left,S_arg7);
            break;
        case RES1_ARGNUM:
            Move(rp1,S_data_left,S_res1);
            break;
        case RES2_ARGNUM:
            Move(rp1,S_data_left,S_res2);
            break;
        default:
            return;
            break;
    }
    Text(rp1,&buff[0],38);

}
                


/* givepkt =================================================================
   Seems to print out whatever packet we get
 */
ULONG givepkt(struct DosPacket *pkt) {

    /* printf("Packet: %3ld %08lx %08lx %08lx ",pkt->dp_Type,pkt->dp_Arg1, pkt->dp_Arg2,pkt->dp_Arg3 ); */

    SetAPen(rp1,0L);
    RectFill(rp1,S_data_left,S_packet_type-8,S_data_left+296,S_packet_type+100);
    
    if (selective_display) {
        if (!packet_hit[pkt->dp_Type]) {
                return(0);
        }
    } 
            
    switch (pkt->dp_Type) {
        case _ACTION_NIL:
                printall("ACTION_NIL",pkt);
                break;

        case _ACTION_GET_BLOCK:
                printall("ACTION_GET_BLOCK (OBSOLETE!)",pkt);
                break;

        case _ACTION_SET_MAP:
                printall("ACTION_SET_MAP (OBSOLETE!)",pkt);
                break;

        case _ACTION_DIE:
                printall("ACTION_DIE",pkt);
                break;

        case _ACTION_EVENT:
                printall("ACTION_EVENT (OBSOLETE!)",pkt);
                break;

        case _ACTION_CURRENT_VOLUME:
                printtype("ACTION_CURRENT_VOLUME");
                printarg("Volume node", pkt, 1);
                break;

        case _ACTION_LOCATE_OBJECT:
                printtype("ACTION_LOCATE_OBJECT");
                printarg("Lock",pkt,1);
                printB("Name",pkt->dp_Arg2,2);
                printarg("Mode",pkt,3);
                break;

        case _ACTION_RENAME_DISK:
                printtype("ACTION_RENAME_DISK");
                printB("New Name",pkt->dp_Arg1,1);
                printarg("Success",       pkt, RES1_ARGNUM);
                printarg("Code",          pkt, RES2_ARGNUM);
                break;

        case _ACTION_FREE_LOCK:
                printtype("ACTION_FREE_LOCK");
                printarg("Lock",pkt,1);
                break;

        case _ACTION_DELETE_OBJECT:
                printtype("ACTION_DELETE_OBJECT");
                printarg("Lock",pkt,1);
                printB("Name",pkt->dp_Arg2,2);
                printarg("Success",       pkt, RES1_ARGNUM);
                printarg("Code",          pkt, RES2_ARGNUM);
                break;

        case _ACTION_RENAME_OBJECT:
                printtype("ACTION_RENAME_OBJECT");
                printarg("Lock (From)",pkt,1);
                printB("Old name",pkt->dp_Arg2,2);
                printarg("Lock (To)",pkt,3);
                printB("New name",pkt->dp_Arg4,4);
                printarg("Success",       pkt, RES1_ARGNUM);
                printarg("Code",          pkt, RES2_ARGNUM);
                break;

        case _ACTION_MORE_CACHE:
                printtype("ACTION_MORE_CACHE");
                printarg("Number of buffers",     pkt, 1);
                printarg("Success",               pkt, RES1_ARGNUM);
                printarg("New number of buffers", pkt, RES2_ARGNUM);
                break;

        case _ACTION_COPY_DIR:
                printtype("ACTION_COPY_DIR");
                printarg("Lock",pkt,1);
                break;

        case _ACTION_WAIT_CHAR:
                printtype("ACTION_WAIT_CHAR");
                printarg("Timeout", pkt, 1);
                break;

        case _ACTION_SET_PROTECT:
                printtype("ACTION_SET_PROTECT");
                printarg("Lock", pkt, 2);
                printB("Name", pkt->dp_Arg3, 3);
                printarg("Mask", pkt, 4);
                printarg("Success",       pkt, RES1_ARGNUM);
                printarg("Code",          pkt, RES2_ARGNUM);
                break;

        case _ACTION_CREATE_DIR:                
                printtype("ACTION_CREATE_DIR");
                printarg("Lock", pkt, 1);
                printB("Name", pkt->dp_Arg2, 2);
                printarg("Lock", pkt, RES1_ARGNUM);
                printarg("Code", pkt, RES2_ARGNUM);
                break;

        case _ACTION_EXAMINE_OBJECT:
                printtype("ACTION_EXAMINE_OBJECT");
                printarg("Lock",          pkt, 1);
                printarg("FileInfoBlock", pkt, 2);
                printarg("Success",       pkt, RES1_ARGNUM);
                printarg("Code",          pkt, RES2_ARGNUM);
                break;

        case _ACTION_EXAMINE_NEXT:
                printtype("ACTION_EXAMINE_NEXT");
                printarg("Lock", pkt, 1);
                printarg("FileInfoBlock", pkt, 2);
                printarg("Success",       pkt, RES1_ARGNUM);
                printarg("Code",          pkt, RES2_ARGNUM);
                break;

        case _ACTION_DISK_INFO:
                printtype("ACTION_DISK_INFO");
                printarg("InfoData",             pkt, 1);
                printarg("Success/Window",       pkt, RES1_ARGNUM);
                printarg("Code",                 pkt, RES2_ARGNUM);
                break;

        case _ACTION_INFO:
                printtype("ACTION_INFO");
                printarg("Lock", pkt, 1);
                printarg("InfoData", pkt, 2);
                printarg("Success",       pkt, RES1_ARGNUM);
                printarg("Code",          pkt, RES2_ARGNUM);
                break;

        case _ACTION_FLUSH:
                printall("ACTION_FLUSH",pkt);
                break;

        case _ACTION_SET_COMMENT:
                printtype("ACTION_SET_COMMENT");
                /* arg 1 is unused */
                printarg("Lock",pkt,2);
                printB("Object Name", pkt->dp_Arg3, 3);
                printB("Comment", pkt->dp_Arg4, 4);
                printarg("Success",       pkt, RES1_ARGNUM);
                printarg("Code",          pkt, RES2_ARGNUM);
                break;

        case _ACTION_PARENT:
                printtype("ACTION_PARENT");
                printarg("Lock",pkt,1);
                printarg("Success",       pkt, RES1_ARGNUM);
                printarg("Code",          pkt, RES2_ARGNUM);
                break;

        case _ACTION_TIMER:
                printall("ACTION_TIMER",pkt);
                break;

        case _ACTION_INHIBIT:
                printtype("ACTION_INHIBIT");
                printarg("Inhibit?",pkt,1);
                printarg("Success", pkt, RES1_ARGNUM);
                printarg("Code", pkt, RES2_ARGNUM);
                break;

        case _ACTION_DISK_TYPE:
                printall("ACTION_DISK_TYPE (OBSOLETE!)",pkt);
                break;

        case _ACTION_DISK_CHANGE:
                printall("ACTION_DISK_CHANGE (OBSOLETE!)", pkt);
                break;

        case _ACTION_SET_FILE_DATE: /* ACTION_SET_DATE */
                printtype("ACTION_SET_FILE_DATE");
                /* arg 1 unused */
                printarg("Lock",pkt,2);
                printB("Object Name", pkt->dp_Arg3, 3);
                printarg("DateStamp",     pkt, 4);              /* cptr */
                printarg("Success",       pkt, RES1_ARGNUM);
                printarg("Code",          pkt, RES2_ARGNUM);
                break;

        case _ACTION_READ:
                printtype("ACTION_READ");
                printarg("FileHandle",pkt,1);
                printarg("Buffer",pkt,2);
                printarg("Length",pkt,3);
                printarg("Bytes Read",pkt,RES1_ARGNUM);
                printarg("Code",pkt,RES2_ARGNUM);
                break;

        case _ACTION_WRITE:
                printtype("ACTION_WRITE");
                printarg("FileHandle",pkt,1);
                printarg("Buffer",pkt,2);
                printarg("Length",pkt,3);
                printarg("Bytes Written",pkt,RES1_ARGNUM);
                printarg("Code",pkt,RES2_ARGNUM);
                break;

        case _ACTION_SET_SCREEN_MODE:  /* ACTION_SCREEN_MODE */
                printall("ACTION_SET_SCREEN_MODE",pkt);
                break;

        case _ACTION_READ_INTERNAL:
                printall("ACTION_READ_INTERNAL",pkt);
                break;

        case _ACTION_WRITE_INTERNAL:
                printall("ACTION_WRITE_INTERNAL",pkt);
                break;

        case _ACTION_FIND_INPUT:
                printtype("ACTION_FIND_INPUT");
                printarg("File Handle",pkt,1);
                printarg("Lock",pkt,2);
                printB("Name",pkt->dp_Arg3,3);
                break;

        case _ACTION_FIND_OUTPUT:
                printtype("ACTION_FIND_OUTPUT");
                printarg("File Handle",pkt,1);
                printarg("Lock",pkt,2);
                printB("Name",pkt->dp_Arg3,3);
                break;

        case _ACTION_CLOSE:    /* ACTION_END */
                printall("ACTION_CLOSE",pkt);
                break;

        case _ACTION_SEEK:
                printtype("ACTION_SEEK");
                printarg("File Handle",pkt,1);
                printarg("Position",pkt,2);
                printarg("Mode",pkt,3);
                printarg("Old Position",pkt,RES1_ARGNUM);
                printarg("Code",pkt,RES2_ARGNUM);
                break;

        /* REAL THINGS FROM DOSEXTENS.H FOLLOW: */
        case ACTION_FINDUPDATE:
            printall("ACTION_FINDUPDATE",pkt);
            break;

        case ACTION_SET_FILE_SIZE:
            printtype("ACTION_SET_FILE_SIZE");
            printarg("FileHandle",pkt,1);
            printarg("New EOF from mode",pkt,2);
            printarg("Mode",pkt,3);
            printarg("Success", pkt, RES1_ARGNUM);
            printarg("Code", pkt, RES2_ARGNUM);
            break;

        case ACTION_WRITE_PROTECT:
            printtype("ACTION_WRITE_PROTECT");
            printarg("Protect?",pkt,1);
            printarg("PassKey",pkt,2);
            printarg("Success", pkt, RES1_ARGNUM);
            printarg("Code", pkt, RES2_ARGNUM);
            break;

        case ACTION_SAME_LOCK:
            printtype("ACTION_SAME_LOCK");
            printarg("Lock 1", pkt, 1);
            printarg("Lock 2", pkt, 2);
            printarg("Comparison Result", pkt, RES1_ARGNUM);
            printarg("Code", pkt, RES2_ARGNUM);
            break;

        case ACTION_CHANGE_SIGNAL:
            printall("ACTION_CHANGE_SIGNAL",pkt);
            break;

        case ACTION_FORMAT:
            printtype("ACTION_FORMAT");
            printB("Device name", pkt->dp_Arg1, 1);
            printB("Volume name", pkt->dp_Arg2, 2);
            printarg("Format type", pkt, 3);
            printarg("Success", pkt, RES1_ARGNUM);
            printarg("Code", pkt, RES2_ARGNUM);
            break;

        case ACTION_MAKE_LINK:
            printtype("ACTION_MAKE_LINK");
            printarg("Lock", pkt, 1);
            printB("Link Name", pkt->dp_Arg2, 2);
            printarg("Lock", pkt, 3);
            printarg("Mode", pkt, 4);
            printarg("Success", pkt, RES1_ARGNUM);
            printarg("Code", pkt, RES2_ARGNUM);
            break;

        case ACTION_READ_LINK:
            printtype("ACTION_READ_LINK");
            printarg("Lock", pkt, 1);
            printarg("Path/Name", pkt, 2);
            printarg("Buffer", pkt, 3);
            printarg("Size of Buffer", pkt, 4);
            printarg("String Length", pkt, RES1_ARGNUM);
            printarg("Code", pkt, RES2_ARGNUM);
            break;

        case ACTION_FH_FROM_LOCK:
            printtype("ACTION_FH_FROM_LOCK");
            printarg("FileHandle",pkt,1);
            printarg("Lock",pkt,2);
            printarg("Success", pkt, RES1_ARGNUM);
            printarg("Code", pkt, RES2_ARGNUM);
            break;

        case ACTION_IS_FILESYSTEM:
            printtype("ACTION_IS_FILESYSTEM");
            printarg("Success", pkt, RES1_ARGNUM);
            printarg("Code", pkt, RES2_ARGNUM);
            break;

        case ACTION_CHANGE_MODE:
            printtype("ACTION_CHANGE_MODE");
            printarg("Object type",pkt,1);
            printarg("Object",pkt,2);
            printarg("New Mode",pkt,3);
            printarg("Success", pkt, RES1_ARGNUM);
            printarg("Code", pkt, RES2_ARGNUM);
            break;

        case ACTION_COPY_DIR_FH:
            printtype("ACTION_COPY_DIR_FH");
            printarg("File Handle's fh_Arg1", pkt, 1);
            printarg("Lock", pkt, RES1_ARGNUM);
            printarg("Code", pkt, RES2_ARGNUM);
            break;

        case ACTION_PARENT_FH:
            printtype("ACTION_PARENT_FH");
            printarg("File Handle's fh_Arg1", pkt, 1);
            printarg("Lock on Parent", pkt, RES1_ARGNUM);
            printarg("Code", pkt, RES2_ARGNUM);
            break;

        case ACTION_EXAMINE_ALL:                             /* ! */
            printtype("ACTION_EXAMINE_ALL");
            printarg("Lock",         pkt, 1);
            printarg("Buffer",       pkt, 2);
            printarg("Length",       pkt, 3);
            printarg("Request Type", pkt, 4);
            printarg("Ctrl struct",  pkt, 5);
            printarg("Continue?",    pkt, RES1_ARGNUM);
            printarg("FailCode",     pkt, RES2_ARGNUM);
            break;

        case ACTION_EXAMINE_FH:
            printtype("ACTION_EXAMINE_FH");
            printarg("FileHandle",      pkt,    1);
            printarg("FileInfoBlock",   pkt,    2);
            printarg("Success",         pkt,    RES1_ARGNUM);
            printarg("FailCode",        pkt,    RES2_ARGNUM);
            break;

        case ACTION_LOCK_RECORD:
            printtype("ACTION_LOCK_RECORD");
            printarg("FileHandle",pkt, 1);
            printarg("Start Position", pkt,2);
            printarg("Record length", pkt,3);
            printarg("Mode", pkt, 4);
            printarg("Timeout", pkt, 5);
            printarg("Success", pkt, RES1_ARGNUM);
            printarg("Code", pkt, RES2_ARGNUM);
            break;

        case ACTION_FREE_RECORD:
            printtype("ACTION_FREE_RECORD");
            printarg("FileHandle",pkt, 1);
            printarg("Start Position", pkt,2);
            printarg("Record length", pkt,3);
            printarg("Success", pkt, RES1_ARGNUM);
            printarg("Code", pkt, RES2_ARGNUM);
            break;

        case ACTION_ADD_NOTIFY:
            printtype("ACTION_ADD_NOTIFY");
            printarg("NotifyRequest",pkt,1);
            printarg("Success", pkt, RES1_ARGNUM);
            printarg("Code", pkt, RES2_ARGNUM);
            break;

        case ACTION_REMOVE_NOTIFY:
            printtype("ACTION_REMOVE_NOTIFY");
            printarg("NotifyRequest",pkt,1);
            printarg("Success", pkt, RES1_ARGNUM);
            printarg("Code", pkt, RES2_ARGNUM);
            break;

        case ACTION_EXAMINE_ALL_END:                             /* ! */
            /* According to Randell, same args as exall */
            /* printall("ACTION_EXAMINE_ALL_END",pkt); */
            printtype("ACTION_EXAMINE_ALL_END");
            printarg("Lock",         pkt, 1);
            printarg("Buffer",       pkt, 2);
            printarg("Length",       pkt, 3);
            printarg("Request Type", pkt, 4);
            printarg("Ctrl struct",  pkt, 5);
            printarg("Continue?",    pkt, RES1_ARGNUM);
            printarg("FailCode",     pkt, RES2_ARGNUM);
            break;

        case ACTION_SET_OWNER:
            /* According to Randell, args are the same as setprotect, 
               but bits are group and owner 
             */
            /* printall("ACTION_SET_OWNER",pkt); */
            printtype("ACTION_SET_OWNER");
            printarg("Lock", pkt, 2);
            printB("Name", pkt->dp_Arg3, 3);
            printarg("Group|User ID", pkt, 4);
            break;

        case ACTION_SERIALIZE_DISK:
            printall("ACTION_SERIALIZE_DISK",pkt);
            break;

        default:
                /* enhancement: print pkt->type */
                printall("Unknown Type",pkt);
                break;
    }
    if (dos_error)  {
        if (packet_hit[pkt->dp_Type]) {
            return(dos_error);
        }
    }
    return(0);

}

VOID btos(UBYTE *bstr, UBYTE *buf) {

    bstr = BADDR(bstr);
    CopyMem(bstr+1,buf,*bstr);
    buf[*bstr] = 0;
}
