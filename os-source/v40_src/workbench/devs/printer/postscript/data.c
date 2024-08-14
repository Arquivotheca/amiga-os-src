
/* Character data table for the DoSpecial() function */

#include <exec/types.h>


/*****************************************************************************/


STRPTR CommandTable[] =
{
    "\377",         /* 00 Reset */
    "\377",         /* 01 initialize */
    "\n",           /* 02 LF */
    "\r\n",         /* 03 Return, LF */
    "\377",         /* 04 Reverse Line Feed */

    "\377",         /* 05 Normal character set */
    "\377",         /* 06 Italics on */
    "\377",         /* 07 Italics off */
    "\377",         /* 08 Underline on */
    "\377",         /* 09 Underline off */
    "\377",         /* 10 Boldface on */
    "\377",         /* 11 Boldface off */
    "\377",         /* 12 Set foreground color */
    "\377",         /* 13 Set background color */
    "\377",         /* 14 Normal pitch */
    "\377",         /* 15 Elite on */
    "\377",         /* 16 Elite off */
    "\377",         /* 17 Condensed on */
    "\377",         /* 18 Condensed off */
    "\377",         /* 19 Enlarged on */
    "\377",         /* 20 Enlarged off */
    "\377",         /* 21 Shadow print on */
    "\377",         /* 22 Shadow print off */
    "\377",         /* 23 Double strike on */
    "\377",         /* 24 Double strike off */
    "\377",         /* 25 NLQ on */
    "\377",         /* 26 NLQ off */

    "\377",         /* 27 superscript on */
    "\377",         /* 28 superscript off */
    "\377",         /* 29 subscript on */
    "\377",         /* 30 subscript off */
    "\377",         /* 31 normalize */
    "\377",         /* 32 partial line up */
    "\377",         /* 33 partial line down */

    "\377",         /* 34 US character set */
    "\377",         /* 35 French char set */
    "\377",         /* 36 German char set */
    "\377",         /* 37 UK char set */
    "\377",         /* 38 Danish I char set */
    "\377",         /* 39 Sweden char set */
    "\377",         /* 40 Italian char set */
    "\377",         /* 41 Spanish char set */
    "\377",         /* 42 Japanese char set */
    "\377",         /* 43 Norweign char set */
    "\377",         /* 44 Danish II char set */

    "\377",         /* 45 Proportional on */
    "\377",         /* 46 Proportional off */
    "\377",         /* 47 Proportional clear */
    "\377",         /* 48 Set prop offset */
    "\377",         /* 49 Auto left justify */
    "\377",         /* 50 Auto right justify */
    "\377",         /* 51 Auto full justify */
    "\377",         /* 52 Auto jusity off */
    "\377",         /* 53 letter space */
    "\377",         /* 54 Auto center on */

    "\377",         /* 55 1/8" line space */
    "\377",         /* 56 1/6" line space */
    "\377",         /* 57 set form length */
    "\377",         /* 58 perf skip n */
    "\377",         /* 59 perf skip off */

    "\377",         /* 60 Left margin set */
    "\377",         /* 61 Right margin set */
    "\377",         /* 62 Top margin set */
    "\377",         /* 63 Bottom margin set */
    "\377",         /* 64 T&B margin set */
    "\377",         /* 65 L&R margin set */
    "\377",         /* 66 Clear margins */

    "\377",         /* 67 Set horiz. tab */
    "\377",         /* 68 Set vertical tab */
    "\377",         /* 69 Clr horiz tab */
    "\377",         /* 70 Clear all h tabs */
    "\377",         /* 71 Clr vertical tab */
    "\377",         /* 72 Clr all v tabs */
    "\377",         /* 73 Clr all h & v tabs */
    "\377",         /* 74 Set default tabs */

    "\377",         /* 75 Extended command */
    "\377",         /* 76 Raw command */
};


/*****************************************************************************/


STRPTR ExtendedCharTable[] =
{
    "\240", "\241", "\242", "\243", "\244", "\245", "\246", "\247",
    "\250", "\251", "\252", "\253", "\254", "\255", "\256", "\257",
    "\260", "\261", "\262", "\263", "\264", "\265", "\266", "\267",
    "\270", "\271", "\272", "\273", "\274", "\275", "\276", "\277",
    "\300", "\301", "\302", "\303", "\304", "\305", "\306", "\307",
    "\310", "\311", "\312", "\313", "\314", "\315", "\316", "\317",
    "\320", "\321", "\322", "\323", "\324", "\325", "\326", "\327",
    "\330", "\331", "\332", "\333", "\334", "\335", "\336", "\337",
    "\340", "\341", "\342", "\343", "\344", "\345", "\346", "\347",
    "\350", "\351", "\352", "\353", "\354", "\355", "\356", "\357",
    "\360", "\361", "\362", "\363", "\364", "\365", "\366", "\367",
    "\370", "\371", "\372", "\373", "\374", "\375", "\376", "\377",
};
