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
/*  ProportionalTools.c - Make Window Props with arrows
*
*   Mitchell/Ware Systems       Version 1.15        4/19/88
*
*   NOTE: Compile with the -ad option (using lc) to move arrows to chip ram.
*______________________________________________________________________________
*
*   For Windows:
*
*   struct Gadget *MakeVertProp (key, gadlist) 
*                                       - VPROP -> UP -> DOWN
*       struct Remember **key;   
*       struct Gadget *gadlist;  add these gadgets to this list if nonNULL
*
*   struct Gadget *MakeHorizProp (key, gadlist) 
*                                       - HPROP -> LEFT -> RIGHT
*       struct Remember **key;   
*       struct Gadget *gadlist;  add these gadgets to this list if nonNULL
*
*   struct Gadget *MakeProps (key, gadlist) 
*                   - HPROP -> LEFT -> RIGHT -> VPROP -> UP -> DOWN
*           userid      0       1        2        3      4      5
*       struct Remember **key;   
*       struct Gadget *gadlist;  add these gadgets to this list if nonNULL
*       (set the GadgetIDCounter to zero first)
*
*______________________________________________________________________________
*
*   For Independent Props:
*
*   struct Gadget *MakeIndHorizProp(key, im, LeftEdge, TopEdge, Length, F, A)
*                                       - HPROP -> LEFT -> RIGHT
*       struct Remember **key;
*       struct Image *im            Image to use for the knob, or null
*       int LeftEdge, TopEdge;      Location of the prop
*       int Length;                 Length of Prop - including arrows
*       ULONG   F;                  Flags
*       ULONG   A;                  Activation
*
*   struct Gadget *MakeIndVertProp(key, im, LeftEdge, TopEdge, Length F, A)
*                                       - VPROP -> UP -> DOWN
*       struct Remember **key;
*       struct Image *im            Image to use for the knob, or null
*       int LeftEdge, TopEdge;      Location of the prop
*       int Length;                 Length of Prop - including arrows
*       ULONG   F;                  Flags
*       ULONG   A;                  Activation
*
*   For either case, a copy of the "im" image struct is made to prevent
*   conflicts arising out of the mutiple use of the same image struct.
*/

#include <exec/types.h>
#include <exec/memory.h>
#include <intuition/intuition.h>
#include <Tools.h>

#define HPLANES 1
#define VPLANES 2

#define CVP_FLAGS           (GRELHEIGHT | GRELRIGHT)
#define CVP_ACTIVATION      (GADGIMMEDIATE | FOLLOWMOUSE | RIGHTBORDER | RELVERIFY)
#define CVP_UP_FLAGS        (GADGHCOMP | GRELRIGHT)
#define CVP_UP_ACTIVATION   (GADGIMMEDIATE | RIGHTBORDER)
#define CVP_DOWN_FLAGS      (GADGHCOMP | GRELRIGHT | GRELBOTTOM)
#define CVP_DOWN_ACTIVATION (GADGIMMEDIATE | RIGHTBORDER)

#define CHP_FLAGS           (GRELWIDTH | GRELBOTTOM)
#define CHP_ACTIVATION      (GADGIMMEDIATE | FOLLOWMOUSE | BOTTOMBORDER | RELVERIFY)
#define CHP_LEFT_FLAGS      (GADGHCOMP | GRELBOTTOM)
#define CHP_LEFT_ACTIVATION (GADGIMMEDIATE | BOTTOMBORDER)
#define CHP_RIGHT_FLAGS     (GADGHCOMP | GRELBOTTOM | GRELRIGHT)
#define CHP_RIGHT_ACTIVATION (GADGIMMEDIATE | BOTTOMBORDER)


USHORT chip RightBody[] = 
{
   0xfff8,0xfe38,0xff88,0xffe0,0xc000,0xffe0,0xff88,0xfe38,
   0xfff8,
};

USHORT chip LeftBody[] = 
{
   0xfff8,0xe3f8,0x8ff8,0x3ff8,0x0018,0x3ff8,0x8ff8,0xe3f8,
   0xfff8,
};

USHORT chip UpBody[] = 
{
   0xf81f,0xe667,0x9e79,0xfe7f,0xfe7f,0xfe7f,0xffff,
};

USHORT chip DownBody[] = 
{
   0xffff,0xfe7f,0xfe7f,0xfe7f,0x9e79,0xe667,0xf81f,
};


struct Image HiRight = {
  0,0,      /*  LeftEdge,TopEdge  */
  13,9,1,   /*  Width,Height,Depth  */
  RightBody,  /* Pointer to Image data */
  0x01,0x00,      /* PlanePick, PlaneOnOff  */
  NULL,           /* Ptr to next image */
};

struct Image HiLeft = {
  0,0,      /*  LeftEdge,TopEdge  */
  13,9,1,   /*  Width,Height,Depth  */
  LeftBody,  /* Pointer to Image data */
  0x01,0x00,      /* PlanePick, PlaneOnOff  */
  NULL,           /* Ptr to next image */
};

struct Image HiUp = {
  0,0,      /*  LeftEdge,TopEdge  */
  16,7,1,   /*  Width,Height,Depth  */
  UpBody,  /* Pointer to Image data */
  0x01,0x00,      /* PlanePick, PlaneOnOff  */
  NULL,           /* Ptr to next image */
};

struct Image HiDown = {
  0,0,      /*  LeftEdge,TopEdge  */
  16,7,1,   /*  Width,Height,Depth  */
  DownBody,  /* Pointer to Image data */
  0x01,0x00,      /* PlanePick, PlaneOnOff  */
  NULL,           /* Ptr to next image */
};

struct Gadget *MakeVertProp (key, gadlist)
struct Remember **key;
struct Gadget *gadlist;
{
    struct Gadget *g, *gu, *gd, *glast;
    
    /* find the end of gadget list */
    for (glast = gadlist; glast && glast->NextGadget; glast = glast->NextGadget)
        ;
    /* either glast is NULL or points to last gadget in list. */

    /* Proportional Gadget */
    g = MakePropGadget(key, NULL, -VWIDTH+1, VHEIGHT+DRAG, VWIDTH, -(2*VHEIGHT+DRAG+HHEIGHT),
                            CVP_FLAGS, AUTOKNOB | FREEVERT, CVP_ACTIVATION);
    if (g && glast)
    {
        glast->NextGadget = g;
    }
    else if (!g)
        return NULL;    /* out of memory */
    
    /* Arrows */
    gu = MakeBGadget(key, &HiUp, NULL, -VWIDTH+1, DRAG, 
                            CVP_UP_FLAGS, CVP_UP_ACTIVATION);
                            
    gd = MakeBGadget(key, &HiDown, NULL, -VWIDTH+1, -(HHEIGHT + VHEIGHT)+1, 
                            CVP_DOWN_FLAGS, CVP_DOWN_ACTIVATION);
    
    if (!gd || !gu)
        return NULL;    /* out of memory */
    
    g->NextGadget = gu;
    gu->NextGadget = gd;
    return g;
}

struct Gadget *MakeHorizProp (key, gadlist)
struct Remember **key;
struct Gadget *gadlist;
{
    struct Gadget *g, *gl, *gr, *glast;
    
    /* find the end of gadget list */
    for (glast = gadlist; glast && glast->NextGadget; glast = glast->NextGadget)
        ;
    /* either glast is NULL or points to last gadget in list. */

    /* Proportional Gadget */
    g = MakePropGadget(key, NULL, HWIDTH+1, -HHEIGHT+1, -(2*HWIDTH+VWIDTH-1), HHEIGHT,
                            CHP_FLAGS, AUTOKNOB | FREEHORIZ, CHP_ACTIVATION);
    if (g && glast)
    {
        glast->NextGadget = g;
    }
    else if (!g)
        return NULL;    /* out of memory */
    
    /* Arrows */
    gl = MakeBGadget(key, &HiLeft, NULL, 1, -HHEIGHT+1,
                            CHP_LEFT_FLAGS, CHP_LEFT_ACTIVATION);
                            
    gr = MakeBGadget(key, &HiRight, NULL, -(HWIDTH + VWIDTH)+1, -HHEIGHT+1,
                            CHP_RIGHT_FLAGS, CHP_RIGHT_ACTIVATION);
    
    if (!gr || !gl)
        return NULL;    /* out of memory */
    
    g->NextGadget = gl;
    gl->NextGadget = gr;
    return g;
}

struct Gadget *MakeProps(key, gadlist)
struct Remember **key;
struct Gadget *gadlist;
{
    struct Gadget *g;
    
    SetGadgetIDCounter(0);
    g = MakeHorizProp(key, gadlist);
    
    if (g && MakeVertProp(key, g))
        return g;
    else
        return NULL;    /* out of memory */
}
#
/*****************************************************************************

    Independent Props and Arrows
    
    Create these independent of any purpose.

*****************************************************************************/

#define NOGREL ~(GRELWIDTH | GRELHEIGHT | GRELBOTTOM | GRELRIGHT)

struct Gadget *MakeIndHorizProp(key, im, Left, Top, Length, Flags, Activation)
struct Remember **key;
struct Image *im;       /* Null if AUTOKNOB is to be used             */
int Left, Top;          /* Location of prop                           */
int Length;             /* total Length of prop, including the arrows */
ULONG Flags, Activation;    /* These are common among Props and Arrows */
{
    struct Gadget *g, *gl, *gr;
    struct Image *image;
    
    if (im && (image = AllocRemember(key, sizeof(struct Image), MEMF_PUBLIC)))
        *image = *im;
    else
        image = im; /* insufficent memory, so use same structure; or no im. */
    
    /* Proportional Gadget */
    g = MakePropGadget(key, image, Left+HWIDTH, Top, Length - 2*HWIDTH, HHEIGHT,
                            Flags, FREEHORIZ | ((image) ? NULL : AUTOKNOB), Activation);
    if (!g)
        return NULL;    /* out of memory */
    
    /* Arrows */
    gl = MakeBGadget(key, &HiLeft, NULL, Left, Top, Flags & NOGREL, Activation | GADGIMMEDIATE);
    gr = MakeBGadget(key, &HiRight, NULL, Left + Length - HWIDTH, Top,
                        Flags | (Length < 0) ? GRELRIGHT : NULL, Activation | GADGIMMEDIATE);
    
    if (!gr || !gl)
        return NULL;    /* out of memory */
    
    g->NextGadget = gl;
    gl->NextGadget = gr;
    return g;
}

struct Gadget *MakeIndVertProp(key, im, Left, Top, Length, Flags, Activation)
struct Remember **key;
struct Image *im;       /* Null if AUTOKNOB is to be used.            */
int Left, Top;          /* Location of prop                           */
int Length;             /* total Length of prop, including the arrows */
ULONG Flags, Activation;    /* These are common among Props and Arrows */
{
    struct Gadget *g, *gu, *gd;
    struct Image *image;
    
    if (im && (image = AllocRemember(key, sizeof(struct Image), MEMF_PUBLIC)))
        *image = *im;
    else
        image = im; /* insufficent memory, so use same structure; or no im. */
    
    /* Proportional Gadget */
    g = MakePropGadget(key, image, Left, Top + VHEIGHT, VWIDTH, Length -2 * VHEIGHT,
                            Flags, FREEVERT | ((image) ? NULL : AUTOKNOB), Activation);
    if (!g)
        return NULL;    /* out of memory */
    
    /* Arrows */
    gu = MakeBGadget(key, &HiUp, NULL, Left, Top, Flags & NOGREL, Activation | GADGIMMEDIATE);
    gd = MakeBGadget(key, &HiDown, NULL, Left, Top + Length - VHEIGHT, 
                        Flags | (Length < 0) ? GRELBOTTOM : NULL, Activation | GADGIMMEDIATE);
    
    if (!gd || !gu)
        return NULL;    /* out of memory */
    
    g->NextGadget = gu;
    gu->NextGadget = gd;
    return g;

}
