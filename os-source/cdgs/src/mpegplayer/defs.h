#ifndef DEFS_H
#define DEFS_H


/*****************************************************************************/


#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <devices/mpeg.h>


/*****************************************************************************/


#define SECTORS_PER_SECOND 75
#define SECTORS_PER_MINUTE (SECTORS_PER_SECOND*60)
#define SECTORS_PER_INTRO  (SECTORS_PER_SECOND*10)
#define MPEG_SECTOR_SIZE   2328

#define SLOT_HEIGHT         27
#define SLOT_BASELINE       21
#define TEXT_OFFSET         32
#define VISIBLE_SLOTS       8
#define VISIBLE_AUDIO_SLOTS 9

#define PAGE_LEFT      118
#define PAGE_TOP       135
#define PAGE_RIGHT     582
#define PAGE_BOTTOM    382
#define PAGE_WIDTH     (PAGE_RIGHT-PAGE_LEFT+1)
#define PAGE_HEIGHT    (PAGE_BOTTOM-PAGE_TOP+1)

#define CREDIT_LEFT    (PAGE_LEFT + 200)
#define CREDIT_WIDTH   (PAGE_WIDTH - 200)

#define TITLE_LEFT     118
#define TITLE_TOP      71
#define TITLE_RIGHT    582
#define TITLE_BOTTOM   112
#define TITLE_WIDTH    (TITLE_RIGHT-TITLE_LEFT+1)
#define TITLE_HEIGHT   (TITLE_BOTTOM-TITLE_TOP+1)

#define TIME_LEFT      486
#define TIME_TOP       405
#define TIME_RIGHT     582
#define TIME_BOTTOM    446
#define TIME_WIDTH     (TIME_RIGHT-TIME_LEFT+1)
#define TIME_HEIGHT    (TIME_BOTTOM-TIME_TOP+1)

#define ICONS_LEFT     118
#define ICONS_TOP      405
#define ICONS_RIGHT    407
#define ICONS_BOTTOM   446
#define ICONS_WIDTH    (ICONS_RIGHT-ICONS_LEFT+1)
#define ICONS_HEIGHT   (ICONS_BOTTOM-ICONS_TOP+1)

#define PREV_LEFT    90
#define PREV_TOP     360
#define PREV_RIGHT   104
#define PREV_BOTTOM  374
#define PREV_WIDTH   (PREV_RIGHT-PREV_LEFT+1)
#define PREV_HEIGHT  (PREV_BOTTOM-PREV_TOP+1)

#define NEXT_LEFT    593
#define NEXT_TOP     360
#define NEXT_RIGHT   607
#define NEXT_BOTTOM  374
#define NEXT_WIDTH   (NEXT_RIGHT-NEXT_LEFT+1)
#define NEXT_HEIGHT  (NEXT_BOTTOM-NEXT_TOP+1)


/*****************************************************************************/


// these are order-dependant, watch out...
enum PageTypes
{
    PAGETYPE_NODISK,
    PAGETYPE_ALBUMNOTES,
    PAGETYPE_TRACKS,
    PAGETYPE_TRACKLYRICS,
    PAGETYPE_TRACKNOTES,
    PAGETYPE_TRACKCREDITS,
    PAGETYPE_PLAYERCREDITS
};


/*****************************************************************************/


enum BoolOptions
{
    OPTB_RANDOM,
    OPTB_INTRO,
    OPTB_LOOP,
    OPTB_MUTE,
    OPTB_TIMEDEFAULT,
    OPTB_TIMEOVERLAY,
    OPTB_SHOWCDG
};

#define OPTF_RANDOM      (1<<OPTB_RANDOM)
#define OPTF_INTRO       (1<<OPTB_INTRO)
#define OPTF_LOOP        (1<<OPTB_LOOP)
#define OPTF_MUTE        (1<<OPTB_MUTE)
#define OPTF_TIMEDEFAULT (1<<OPTB_TIMEDEFAULT)
#define OPTF_TIMEOVERLAY (1<<OPTB_TIMEOVERLAY)
#define OPTF_SHOWCDG     (1<<OPTB_SHOWCDG)


/*****************************************************************************/


enum Icons
{
    // for selection screen
    INTRO_ICON,
    RANDOM_ICON,
    LOOP_ICON,
    CHECKMARK_ICON,

    // for overlay on play screen
    MUTE_ICON,
    CHANNEL1_ICON,
    CHANNEL2_ICON,
    INTRO2_ICON,
    RANDOM2_ICON,
    LOOP2_ICON,
    PLAY_ICON,
    PAUSE_ICON,
    FFWD_ICON,
    FRWD_ICON,
    NEXTTRACK_ICON,
    PREVTRACK_ICON,
    TIME_ICON,
    INTRO3_ICON,
    RANDOM3_ICON,
    LOOP3_ICON,

    NOP_ICON  /* nothing.... */
};

extern const struct Rectangle __far iconsPos[];


/*****************************************************************************/


enum MPEGChannels
{
    MC_ONE,
    MC_TWO,
    MC_NONE
};


/*****************************************************************************/


/* The values form a progression, with each state implying the previous
 * (except CDG_UNAVAILABLE, of course).
 *
 * CDG_UNAVAIABLE means CDGBegin() failed, and we can't do any CDG...() calls.
 * CDG_AVAILABLE means CDGBegin() succeeded, but we have not turned
 *	on subcode processing (by calling CDGPlay()), or that we
 *	have turned it off (by calling CDGStop()).
 * CDG_SUBCODES means that CDGPlay() has been called, and therefore
 *	subcodes are being processed by cdg.library. This state
 *	indicates that we don't have (or don't yet know that we have)
 *	CD+G packets in those subcodes.
 * CDG_HAVEGRAPHICS means we know that CD+G packets are coming off this disk.
 * CDG_SHOWGRAPHICS means that CD+G packets are coming in, and we
 *	have the CD+G screen frontmost.
 */

enum CDGStates
{
    CDG_UNAVAILABLE,
    CDG_AVAILABLE,
    CDG_SUBCODES,
    CDG_HAVEGRAPHICS,
    CDG_SHOWGRAPHICS
};


/*****************************************************************************/


#endif /* DEFS_H */
