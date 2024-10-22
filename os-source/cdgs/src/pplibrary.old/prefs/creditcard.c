
#include <devices/inputevent.h>
#include <libraries/dos.h>

#include "/playerprefs.h"
#include "/cdtvkeys.h"
#include "cardprefs.h"

#include <clib/exec_protos.h>

#include "cdtv/card_lib_pragmas.h"
#include <pragmas/exec_old_pragmas.h>


/********      System Definitions      ********/

extern void                   *SysBase;
extern struct GfxBase         *GfxBase;
extern struct DeBoxBase       *DeBoxBase;
extern struct PlayerPrefsBase *PlayerPrefsBase;


        /* Card handler stuff */

struct Library       *CardResource;
struct CardHandle     CHandle;
struct CardMemoryMap *CardMemMap;
struct Interrupt      CardInsInt;
struct Interrupt      CardRemInt;
struct Interrupt      CardStatInt;
struct CardStatus     CardStatus;

extern struct GadDir PrefsGADList[];

extern struct Render2S_Info d_prpcardR[2];
extern struct Render2S_Info d_prpramR[2];
extern struct Render2S_Info d_prpstopR[2];
extern struct Render2S_Info d_prpgoR[2];

extern struct BitMap   *bm;


void __asm __saveds CardRemSrv(register __a1 struct CardStatus *cs) {

    cs->CardIn = -1;
    cs->WasRemoved = 1;
    cs->CStatus = -1;
    }

void __asm __saveds CardInsSrv(register __a1 struct CardStatus *cs) {

    cs->CardIn = 1;
    cs->CStatus = -1;
    }

UBYTE __asm __saveds  CardStatSrv(register __a1 struct CardStatus *cs, register __d0 UBYTE status) {

    cs->CStatus = -1;
    return(status);
    }
        
void InstallCard() {

    MasterF.gstate[PEGAD_CAUTION] = CAU_NOCARD;
    CardResource = OpenResource(CARDRESNAME);
    if (CardResource == NULL) return;                /* Get the card.resource */
    CardStatus.CStatus = -1;
    CardStatus.CardIn = 0;
    CardStatus.WasRemoved = 0;

    CardRemInt.is_Code = CardRemSrv;
    CardRemInt.is_Data = &CardStatus;
    CardInsInt.is_Code = CardInsSrv;
    CardInsInt.is_Data = &CardStatus;
    CardStatInt.is_Code = (void *)CardStatSrv;
    CardStatInt.is_Data = &CardStatus;              /* Setup card notification */

    CHandle.cah_CardNode.ln_Pri = 120;
    CHandle.cah_CardNode.ln_Type = 0;
    CHandle.cah_CardNode.ln_Name = "Playerprefs";
    CHandle.cah_CardRemoved = &CardRemInt;
    CHandle.cah_CardInserted = &CardInsInt;
    CHandle.cah_CardStatus = &CardStatInt;
    CHandle.cah_CardFlags = CARDF_DELAYOWNERSHIP;

    OwnCard(&CHandle);
    }


void RemoveCard() {

    if (CardResource) ReleaseCard(&CHandle, CARDF_REMOVEHANDLE);

    MasterF.gstate[PEGAD_CAUTION] = 0;
    }


void SetCautionMode(UWORD on) {

struct GadDir *g;

    if (on) {

        g = &PrefsGADList[PEGAD_PRPRAM];
        g->Left = PEGAD_PRPCARD;
        g->Right = PEGAD_PRPCARD;
        g->Up = PEGAD_PRPRAM;
        g->Down = PEGAD_PRPRAM;
        g->RenderData = &d_prpgoR;
        g->routine = 10;

        g = &PrefsGADList[PEGAD_PRPCARD];
        g->Left = PEGAD_PRPRAM;
        g->Right = PEGAD_PRPRAM;
        g->Up = PEGAD_PRPCARD;
        g->Down = PEGAD_PRPCARD;
        g->RenderData = &d_prpstopR;
        g->routine = 10;
        }

    else {

        g = &PrefsGADList[PEGAD_PRPRAM];
        g->Left = PEGAD_CLICK;
        g->Right = PEGAD_PRPCARD;
        g->Up = PEGAD_SCRSAVE;
        g->Down = PEGAD_SCRSAVE;
        g->RenderData = &d_prpramR;
        g->routine = 9;

        g = &PrefsGADList[PEGAD_PRPCARD];
        g->Left = PEGAD_PRPRAM;
        g->Right = PEGAD_LANG;
        g->Up = PEGAD_SCRSAVE;
        g->Down = PEGAD_SCRSAVE;
        g->RenderData = &d_prpcardR;
        g->routine = 9;
        }

    UpdateDisplay();
    }


void UpdateCardStatus() {

    if (CardStatus.CStatus == -1) {

        if (CardStatus.CardIn == -1) {

            ReleaseCard(&CHandle, 0);
            CardStatus.CardIn = 0;
            }

        if (CardStatus.CardIn) {

            MasterF.gstate[PEGAD_CAUTION] |= CAU_CARDIN;
            CardStatus.CStatus = ReadCardStatus();
            }

        else {

            CardStatus.CStatus = 0;
            MasterF.gstate[PEGAD_CAUTION] &= ~(CAU_CARDIN | CAU_CAUTION);
            SetCautionMode(0);
            }

        if ((CardStatus.CStatus & CARD_STATUSF_WR) == 0) MasterF.gstate[PEGAD_CAUTION] |= CAU_PROTECT;

        else                                             MasterF.gstate[PEGAD_CAUTION] &= ~(CAU_PROTECT);

        if ((CardStatus.CStatus & CARD_STATUSF_BVD1)
            || (CardStatus.CStatus & CARD_STATUSF_BVD2)) MasterF.gstate[PEGAD_CAUTION] |= CAU_BATTERY;

        else                                             MasterF.gstate[PEGAD_CAUTION] &= ~(CAU_BATTERY);

        UpdateDisplay();
        }
    }





static ULONG usizes[8] = {

    512, 2*1024, 8*1024, 32*1024, 128*1024, 512*1024, 2*1024*1024
    };

static UBYTE CISEND_tup[2] = {

    CISTPL_END, CISTPL_END
    };

static UBYTE CISTAR_tup[5] = {

    CISTPL_LINKTARGET, 3, 'C','I','S'
    };

#define VER2_SIZE 29

static UBYTE CISVER2_tup[VER2_SIZE] = {

    CISTPL_VERS_2,
    25,                     /* link */
    0,                      /* struct version */
    0,                      /* full comply */
    00,02,                  /* data start at 512 bytes */
    0,0,                    /* reserved */
    0,0,                    /* VS bytes */
    1,                      /* 1 CIS */
    'C','o','m','m','o','d','o','r','e',0x0,
    'A','m','i','g','a',0x0,
    CISTPL_END, CISTPL_END
    };




/* Figure out what kind of attribute memory we have -
 *
 * None, or too small an amount to use
 *
 * Unique, writable memory.  (e.g., would be the HP PalmTop cards ).
 *
 * Overlapped (e.g., Poquet cards).
 *
 * ROM (we have none, but we have to deal with this case of Device
 * tuple, or worse [ARGG!!!] tupleS in attribute memory ROM!  This
 * case will be dealt with by trying to salvage the CISTPL_DEVICE
 * tuple.  To salvage it, we have to check it, and determine if it
 * describes an SRAM, or DRAM card at least as large as our memory
 * sizing operation.  If not, well...
 *
 * Worse, we need to then look after the CISTPL_DEVICE tuple, and
 * scan for the following tuples in ROM attribute memory -
 *
 *  CISTPL_FORMAT, CISTPL_GEOMETRY, and maybe CISTPL_VERS_2.
 *
 * If any of these are in non-writeable ROM, ACK!!!!!  We can't
 * change them, and there is no point trying to write our tuples since
 * they will be caught after any of the ROM tuples.
 *
 */



void __asm SetupCardmark(register __d1 ULONG size, register __a0 UBYTE *loc);

ULONG PrepCard(BOOL mark) {

register UWORD *volatile temp;
register UWORD pattern;
ULONG size;
UBYTE *volatile attr;
UBYTE *volatile comm;
UBYTE status;
UWORD error;
ULONG failat;
UWORD attrmemtype;
UBYTE tuple[8];
struct DeviceTData dt;
int x;
UBYTE link;
BOOL isADEVICE;
BOOL isFUJI;
UBYTE memsetcnt;
ULONG targetat;

    error = PREP_ERROR_NOCARD;
    targetat = 0L;

    if (CardStatus.CardIn) {

        BeginCardAccess(&CHandle);                                                                              /* do not allow card release while writing info */

        error = PREP_ERROR_WP;

        status = ReadCardStatus();                                                                              /* start by trying to write CISTPL_DEVICE tuple */

        if (status & CARD_STATUSF_WR) {

            CardMemMap = GetCardMap();
            attr = CardMemMap->cmm_AttributeMemory;
            comm = CardMemMap->cmm_CommonMemory;

            size = 0L;

            *(temp = (UWORD *volatile)comm) = 0;                                                                /* size common memory, and leave as 0's so CRC/CheckSum disks are already right */

            if (*temp == 0) {

                pattern = 0xAA55;                                                                               /* invert pattern (if needed) as we do a reasonably quick mem size, looking for memory wrapping */

                while(temp < (UWORD *volatile)attr) {

                    temp+=256;
                    if (*temp == pattern) pattern ^= 0xFFFF;

                    *temp = pattern;

                    if (*temp == pattern && *comm == 0) {
                        size++;
                        *temp = 0;
                        }

                    else temp = (UWORD *volatile)attr;
                    }

                error = PREP_ERROR_NOTWRITABLE;

                if (size) {
                    size++;
                    size <<= 9;

                    isADEVICE = FALSE;                                                                          /* make a copy of the CISTPL_DEVICE tuple which already exists, if one exists */
                    isFUJI = FALSE;

                    attrmemtype = ATTR_NONE;                                                                    /* assume there is no ATTR mem to start */

                    if (*attr == (UBYTE)CISTPL_DEVICE) {
                        link = *(attr + 2);

                        if (link > 4) link = 4;

                        for(x=0; x<(link+3); x++) tuple[x] = attr[x*2];

                        if (DeviceTuple(tuple,&dt)) isADEVICE = TRUE;

                        attrmemtype = ATTR_ISROM;                                                               /* assume if its not 0xFF, then its at least ROM memory */
                        }

                    for(x=0,memsetcnt=0; x<(CHECKATTRCNT*2); x+=2) {                                            /* See if we can set at least 16 bytes of memory to 0xFF, clearing */
                                                                                                                /*   common memory first, so we can check for overlap later        */
                        *(comm + x) = 0x00;
                        *(attr + x) = 0xAA;

                        if (*(attr + x) == 0xAA) memsetcnt++;
                        }

                    if (memsetcnt == CHECKATTRCNT) attrmemtype = ATTR_ISMEM;                                    /* assume that the write succeeded, even though it may have infact just */
                                                                                                                /*   written a pattern which was already there - check again later.     */
                    else {                                                                                      /* retry using a delay for the Fujitsu SRAM card which appears not to   */
                                                                                                                /*   like being read back too soon after writes.                        */
                        for(x=0,memsetcnt=0; x<(CHECKATTRCNT*2); x+=2) {

                            *(attr + x) = 0xAA;

                            FujiDelay((attr + x),0xAA);

                            if (*(attr + x) != 0xAA) break;
                            memsetcnt++;
                            }

                        if (memsetcnt == CHECKATTRCNT) {

                            attrmemtype = ATTR_ISMEM;
                            isFUJI = TRUE;
                            }
                        }
        
                    for(x=0,memsetcnt=0; x<(CHECKATTRCNT*2); x+=2) if (comm[x] == 0xAA) memsetcnt++;            /* check to see if this overlapped into common memory, which was set to 0's above */

                    if (memsetcnt == CHECKATTRCNT) attrmemtype = ATTR_OVERLAPPED;                               /* well, now we know we actually have some RAM here!!! its overlapped, but its RAM! */

                    if (attrmemtype == ATTR_ISMEM) {                                                            /* Check again - Make sure this is really memory, and enough memory */

                        for(x=0,memsetcnt=0; x<(CHECKATTRCNT*2); x+=2) {

                            *(attr + x) = CISTPL_NO_LINK;
                            if (isFUJI) FujiDelay((attr + x),CISTPL_NO_LINK);

                            if (*(attr + x) == CISTPL_NO_LINK) memsetcnt++;

                            *(attr + x) = CISTPL_END;
                            if (isFUJI) FujiDelay((attr + x),CISTPL_END);
                            } 

                        if (!memsetcnt) attrmemtype = ATTR_NONE;                                                /* nothing really wrote, so conclude its not RAM at all, but NO MEM */

                        else if (memsetcnt < CHECKATTRCNT) attrmemtype = ATTR_SMALLMEM;                         /* something wrote, but it wasn't enough to be usable for the basic tuples which need to */
                        }                                                                                       /*   go into ATTR mem (if at all possible)                                               */

                    error = PREP_ERROR_NOCARD;                                                                  /* Figure out what we would like to write for our CISTPL_DEVICE tuple */

                    if (CardStatus.CardIn && !CardStatus.WasRemoved) {                                       

                        error = WriteCISTPL_DEVICE(&size,attrmemtype,isADEVICE,&dt,&targetat);                  /* write CISTPL_VER_2 tuple for compliance */

                        if (!error) {
                            comm += (targetat + 5);

                            error = PREP_ERROR_NOTWRITABLE;

                            if (WriteTuple(comm,1,VER2_SIZE, &CISVER2_tup[0])) {
                                error = PREP_ERROR_NOERROR;

                                comm += (VER2_SIZE - 2);                                                        /* write CISTPL_FORMAT, and CISTPL_GEOMETRY if disklike */

                                if (mark) error = WriteCISTPL_FORMAT(size,comm);
                                }
                            }
                        }
                    }
                }
            }

        EndCardAccess(&CHandle);
        }
 
    failat = RETURN_OK;
    if (error) failat = RETURN_ERROR;

    if (failat == PREP_ERROR_NOERROR && mark) SetupCardmark(size, CardMemMap->cmm_CommonMemory);

    return(failat);
    }


/**
 ** Write a CISTPL_FORMAT tuple
 **/

LONG WriteCISTPL_FORMAT(ULONG size, UBYTE *mem) {

register struct TP_Format *tpf;
UBYTE tuples[26];

ULONG defsize;
LONG error;
LONG i;

    for (i = 0; i != 26; tuples[i++] = 0);                                                      /* zero out tuple memory */
    tpf = (struct TP_Format *)&tuples[0];

    tpf->TPL_CODE = CISTPL_FORMAT;
    tpf->TPL_LINK = 20;

    tpf->TPLFMT_TYPE[0] = TPLFMTTYPE_CARDMARK;

    tpf->TPLFMT_OFFSET[0] = 0x00;
    tpf->TPLFMT_OFFSET[1] = 0x02;
    tpf->TPLFMT_OFFSET[2] = 0x0;
    tpf->TPLFMT_OFFSET[3] = 0x0;

    defsize = size - 512;

    tpf->TPLFMT_NBYTE[0] = (defsize) & 0xFF;
    tpf->TPLFMT_NBYTE[1] = (defsize >> 8) & 0xFF;
    tpf->TPLFMT_NBYTE[2] = (defsize >> 16) & 0xFF;
    tpf->TPLFMT_NBYTE[3] = (defsize >> 24) & 0xFF;

    tuples[22] = CISTPL_END;
    tuples[23] = CISTPL_END;

    error = PREP_ERROR_NOTWRITABLE;

    if (WriteTuple(mem,1,24,(UBYTE *)&tpf[0])) error = PREP_ERROR_NOERROR;

    if (error == PREP_ERROR_NOERROR) {

        if (!CardStatus.CardIn || CardStatus.WasRemoved) error = PREP_ERROR_NOTWRITABLE;
        }

    return(error);
    }





/**
 ** Write a CISTPL_DEVICE tuple
 **/

LONG WriteCISTPL_DEVICE(ULONG *msize, UWORD attrmemtype, BOOL isADEVICE, struct DeviceTData *dt, ULONG *targetat) {

UBYTE deftype,defspeed;
UBYTE units,usize;
UBYTE *volatile attr;
UBYTE *volatile comm;

int x, y;
int lasty,lastx;
int besty,bestx;
BOOL match;
ULONG size,bestsize;
LONG error;
UBYTE CISDEV_tup[8];
UBYTE CISLINK_tup[8];
BOOL scanrom;

    error = PREP_ERROR_TOOSMALL;

    deftype = 6<<4;         /* SRAM   */
    defspeed = 1;           /* 250 ns */

/* calc default units, and unit size - best fit requires least # of units, but if we can't
 * find an exact match, take the best fit we can get
 */

    size = *msize;

    size &= ~(511L);
    match = FALSE;
    lasty = 0;
    lastx = 1;

    for(x=1; x<33; x++) {

        for(y=0; y<8; y++) {

            if ((usizes[y] * x) <= size) {

                besty = y;
                bestx = x;
                bestsize = (usizes[y] * x);

                if ((usizes[y] * x) == size) {

                    if (y >= lasty) {

                        lasty = y;
                        lastx = x;
                        match = TRUE;
                        }
                    }
                }
            }
        }       

    if (!match) {

        lastx = bestx;
        lasty = besty;

/* It turns out we might have come up with an actual size that cannot be
 * expressed using unit size, and # of units.  An example would be 17K
 * actual memory size.  The best we could get would be 16K using 512 byte
 * unit sizes, or 18K using 2K size units.  While this is unlikely to be
 * the case, its caught by the double loop above, and size is adjusted
 * if needed to be the best possible size which can be expressed.
 *
 * In the case above we'd come up with 16K actual size expressed as 2
 * 8K units.
 *
 */

        size = bestsize;
        }


    units = lastx;
    usize = lasty;


/* make sure advanced requested units, and unit size is no larger than
 * the actual size we calculated.
 */

    if ((ULONG)(units * usizes[usize]) <= size) {

        size = (ULONG)(units * usizes[usize]);

        units--;
        units <<= 3;


        CISDEV_tup[0] = CISTPL_DEVICE;          /* code */
        CISDEV_tup[1] = 4;                      /* link */
        CISDEV_tup[2] = (deftype|defspeed);     /* type, WPS == 0, speed */
        CISDEV_tup[3] = (usize|units);          /* units * usize */
        CISDEV_tup[4] = 0;                      /* reserve for future use */
        CISDEV_tup[5] = 0;                      /* reserve for future use */

        CISLINK_tup[0] = CISTPL_LONGLINK_C;     /* code */
        CISLINK_tup[1] = 4;                     /* link */
        CISLINK_tup[2] = 0;                     /* default is start of common mmem */
        CISLINK_tup[3] = 0;
        CISLINK_tup[4] = 0;
        CISLINK_tup[5] = 0;

        CardMemMap = GetCardMap();
        comm = CardMemMap->cmm_CommonMemory;
        attr = CardMemMap->cmm_AttributeMemory;

/*
 * Overlapped requires adjusting the long link to skip over the CISTPL_DEVICE tuple
 */
        error = PREP_ERROR_NOTWRITABLE;

        if (attrmemtype == ATTR_OVERLAPPED || attrmemtype == ATTR_ISMEM) {

            if (WriteTuple(attr,2,6,&CISDEV_tup[0])) {

                if (attrmemtype == ATTR_OVERLAPPED) {

                    CISLINK_tup[2] = 0x1C;                          
                    *targetat = 0x1CL;
                    }

                attr += (6*2);

                if (WriteTuple(attr,2,6,&CISLINK_tup[0])) {
                    attr += (6*2);

                    if (WriteTuple(attr,2,2,&CISEND_tup[0])) {

                        if (attrmemtype == ATTR_OVERLAPPED) comm = attr + (2*2);

                        if (WriteTuple(comm,1,5,&CISTAR_tup[0])) {

                            comm+=5;
                            if (WriteTuple(comm,1,2,&CISEND_tup[0])) error = PREP_ERROR_NOERROR;
                            }
                        }
                    }
                }
            }

/*
 * If mem is too small, make sure we have CISTPL_END in attr mem, and
 * continue as if its no mem.
 */

        if (attrmemtype == ATTR_SMALLMEM) {

            *attr = CISTPL_END;
            *(attr + 2) = CISTPL_END;
            attrmemtype = ATTR_NONE;
            }

        if (attrmemtype == ATTR_NONE) {

            if (WriteTuple(comm,1,5,&CISTAR_tup[0])) {

                comm += 5;
                if (WriteTuple(comm,1,6,&CISDEV_tup[0])) {

                    comm+=6;
                    if (WriteTuple(comm,1,2,&CISEND_tup[0])) {

                        *targetat += 6;
                        error = PREP_ERROR_NOERROR;
                        }
                    }
                }
            }

/* Worst case is ROM!!!
 *
 * If there is already a CISTPL_DEVICE tuple here, see if we can use
 * it.  Note that if there is not a 01 at the beg of attribute memory,
 * we have no problem since whatever there is invalid, but...
 * 
 * IF it is a 0x1, then we have to assume its a CISTPL_DEVICE tuple,
 * and in that case we have to decide if -
 *
 * IF its a valid CISTPL_DEVICE tuple, we might be able to use
 * it as is (e.g., it describes a card of the same type, speed, and a
 * size which as at least as large as we need.  Actually speed, and
 * type (if it says SRAM or DRAM) can likely be ignored even.
 *
 * IF its not a valid CISTPL_DEVICE tuple, we cannot change this
 * card, or use it.
 *
 * Finally if its usable, we need to check it again later for other
 * tuples in non-writeable attribute memory (e.g., CISTPL_FORMAT)
 * which we might want to remove, or change.
 */
        if (attrmemtype == ATTR_ISROM) {

            if (isADEVICE) {

                if (dt->dtd_DTsize >= size) {

                    if (dt->dtd_DTtype == DTYPE_SRAM || dt->dtd_DTsize == DTYPE_DRAM) {

                        if (dt->dtd_DTspeed <= 250L) {

/* well, we can use this (good enough), but can we still write other important info
 * to common memory?   Scan for info past the CISTPL_DEVICE tuple in ROM
 */

                            size = dt->dtd_DTsize;

                            scanrom = TRUE;

                            while(scanrom) {

/* check for an CISTPL_END, or a LONGLINK_C, in which case we are in good shape */
/* an alternative is a LINK of CISTPL_END, which means the same thing           */

                                if (attr[2] == CISTPL_END) {
                                    scanrom = FALSE;
                                    error = PREP_ERROR_NOERROR;
                                    }
/* skip over CISTPL_NULL */
                                if (*attr == CISTPL_NULL) attr += 2;
                                else {
/* next tuple at body*2 + 2 bytes for code + 2 bytes for link - initially we
 * skip past the first tuple which we already know is a CISTPL_DEVICE tuple
 */
                                    attr += ((attr[2] * 2)+4);

                                    if (scanrom) {

                                        if (*attr == CISTPL_END|| *attr == CISTPL_LONGLINK_C) {

/* pick up where the LINK_TARGET needs to be */
/* I really should look for a LONGLINK_A tuple here too, but forget it.
 * Its just so darn unlikely, and a worthless thing to do, not this trip!
 */
                                            if (*attr == CISTPL_LONGLINK_C)
                                                *targetat = attr[4] + attr[6] << 8 + attr[8] << 16 + attr[10] << 24;
                                            scanrom = FALSE;
                                            error = PREP_ERROR_NOERROR;
                                            }

/* check for other kinds of tuples which we might want to rewrite */

                                        if (*attr == CISTPL_FORMAT || *attr == CISTPL_GEOMETRY) scanrom = FALSE;


/*
 * If so, stop, and we will return that we cannot rewrite this info.
 * We would like to be able to rewrite the VERS2 tuple too, but we can live
 * with not being able to do so
 */
                                        }

                                    }
/* do not scan past end of common memory */

                                if (attr >= (UBYTE *volatile)0xA20000) {

                                    scanrom = FALSE;
                                    error = PREP_ERROR_NOTWRITABLE;
                                    }
                                }

/* If we are able to use this info in ROM, then add the LINK_TARGET, and we
 * are done.
 */

                            if (!error) {
                                error = PREP_ERROR_NOTWRITABLE;

/* make sure ROM LONKLINK is within 4 megs, plus some reasonable buffer */

                                if (*targetat <= (4*1024*1024 - 10)) {
                                    comm += *targetat;
                                    if (WriteTuple(comm,1,5,&CISTAR_tup[0])) {
                                        comm+=5;
                                        if (WriteTuple(comm,1,2,&CISEND_tup[0])) error = PREP_ERROR_NOERROR;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }


/* Replace actual size, since it might have been modified by our code above
 * which took actual size, and came up with the best possible units,
 * unit size, and actual size.
 */

    *msize = size;

    return(error);
    }

/**
 ** Code to copy a tuple to attribute, or common memory.
 **/

BOOL WriteTuple(UBYTE *volatile mem, UBYTE skip, UBYTE len, UBYTE *data) {

int  x;
BOOL wrote;

    wrote = TRUE;

    for(x = 0; x < len; x++) {

        *mem = *data;

        if (skip == 2) FujiDelay(mem, *data);

        if (*mem != *data) {
            wrote = FALSE;
            break;
            }

        mem += skip;
        data++;
        }

    return(wrote);
    }

/* delay during attr mem writes for FUJI type SRAM cards
 * Fujitsu SRAM cards appear to require roughly 5 milliseconds after
 * a write to attribute mem for the value to read back properly.
 * 
 * I'll do a poll loop, which should fall through quickly for most
 * cards, and has enough slop to be long enough for the Fujitsu cards.
 */

#define FUJIDELAY               500
#define FUJIRETRY               50

UBYTE FujiDelay(UBYTE *volatile mem, UBYTE val) {

UBYTE *volatile ciapra;
UBYTE  cia;
int    retry,delay;

    ciapra = (UBYTE *volatile)0xbfe001;

    for(retry = 0; retry < FUJIRETRY; retry++) {

        for(delay = 0; delay < FUJIDELAY; delay++) cia = *ciapra;
        if (*mem == val) break;
        }

    return(cia);    
    }





void docaution() {

WORD event;

    while (1) {

        event = GetEvent (&input_data);

        if ((event == (IECODE_LBUTTON|IECODE_UP_PREFIX)) || 
            (event == (RAWKEY_RETURN | IECODE_UP_PREFIX))) break;
        }

    if (CardStatus.CardIn) {                                                    /* If no card, then reset the button and return */

        SetCautionMode(1);
        MasterF.gstate[PEGAD_CAUTION] |= CAU_CAUTION;
        MasterF.gstate[MasterF.BoxNo] = 0;
        MasterF.BoxNo = PEGAD_PRPCARD;
        UpdateDisplay();
        return;
        }

    MasterF.gstate[PEGAD_CAUTION] &= ~CAU_CAUTION;
    SetCautionMode(0);
    MasterF.gstate[MasterF.BoxNo] = 0;
    UpdateDisplay();
    }





/* Card Rendering functions */

static void SetCard(struct RastPort *rp, struct BitMap *bbm, WORD on) {

    if (on) BltBitMapRastPort(bbm, 192, 190, rp, 280, 152, 23, 29, 0xc0);
    else    BltBitMapRastPort(bm,  280, 152, rp, 280, 152, 23, 29, 0xc0);
    }

static void SetBatt(struct RastPort *rp, struct BitMap *bbm, WORD on) {

    if (on) BltBitMapRastPort(bbm, 232, 125, rp, 280, 183, 23, 5, 0xc0);
    else    BltBitMapRastPort(bm,  280, 183, rp, 280, 183, 23, 5, 0xc0);
    }

static void SetProt(struct RastPort *rp, struct BitMap *bbm, WORD on) {

    if (on) BltBitMapRastPort(bbm, 225, 194, rp, 289, 156, 5, 5, 0xc0);
    else    BltBitMapRastPort(bm,  289, 156, rp, 289, 156, 5, 5, 0xc0);
    }

static void SetStop(struct RastPort *rp, struct BitMap *bbm, WORD on) {

    if (on) BltBitMapRastPort(bbm, 76,  69, rp, 293, 200, 38, 36, 0xc0);
    else    BltBitMapRastPort(bbm, 170, 69, rp, 293, 200, 38, 36, 0xc0);
    }

static void SetGo(struct RastPort *rp, struct BitMap *bbm, WORD on) {

    if (on) BltBitMapRastPort(bbm, 223, 25, rp, 257, 200, 36, 36, 0xc0);
    else    BltBitMapRastPort(bbm, 133, 69, rp, 257, 200, 36, 36, 0xc0);
    }

static void SetCDTV(struct RastPort *rp, struct BitMap *bbm, WORD on) {

    if (on) BltBitMapRastPort(bbm, 0,   69,  rp, 293, 200, 38, 36, 0xc0);
    else    BltBitMapRastPort(bm,  293, 200, rp, 293, 200, 38, 36, 0xc0);
    }

static void SetRAM(struct RastPort *rp, struct BitMap *bbm, WORD on) {

    if (on) BltBitMapRastPort(bbm, 39,  69,  rp, 257, 200, 36, 36, 0xc0);
    else    BltBitMapRastPort(bm,  257, 200, rp, 257, 200, 36, 36, 0xc0);
    }




#define CR_L 259
#define CR_T 147
#define CR_W 65
#define CR_H 46




GADCardRen(struct RastPort *rp, struct BitMap *bbm, struct GadDir *g, UWORD old, UWORD new) {

    if (new & 8) {

        BltBitMapRastPort(bbm, 193, 223, rp, CR_L, CR_T, CR_W, CR_H, 0xc0 );
        SetStop(rp, bbm, new & 16);
        SetGo(rp, bbm, new & 32);
        }

    else {

        BltBitMapRastPort(bm, CR_L, CR_T, rp, CR_L, CR_T, CR_W, CR_H, 0xc0 );
        SetCard(rp, bbm, new & 1);
        SetCDTV(rp, bbm, new & 16);
        SetRAM(rp, bbm, new & 32);

        if (new & 1) {                                                                     /* Do the card only if card is in AND we're in card mode */

            SetBatt(rp, bbm, new & 2);
            SetProt(rp, bbm, new & 4);
            }

        else {

            SetBatt(rp, bbm, 0);
            SetProt(rp, bbm, 0);
            }
        }

    if ((MasterF.BoxNo == PEGAD_PRPRAM) || (MasterF.BoxNo == PEGAD_PRPCARD)) {

        g = &MasterF.GadList[ MasterF.BoxNo ];
        g->BoxFunc(WorkF->rp, g, 1, MasterF.gstate[MasterF.BoxNo] );
        }

    return (0x80);
    }





void docard(UWORD which) {

UWORD event;

    while (1) {

        event = GetEvent (&input_data);
        if ((event == (IECODE_LBUTTON|IECODE_UP_PREFIX)) || 
            (event == (RAWKEY_RETURN | IECODE_UP_PREFIX)))
            break;
        }
    
    if (MasterF.BoxNo != PEGAD_PRPCARD) {

        CardStatus.WasRemoved = 0;

        if (which == PEGAD_PRPRAM) PrepCard(0);
        else                       PrepCard(1);
        }

    SetCautionMode(0);
    MasterF.gstate[MasterF.BoxNo] = 0;                                                  /* We're done - reset and return */
    MasterF.gstate[PEGAD_CAUTION] = 0;
    UpdateDisplay();
    UpdateCardStatus();
    }


