/*
	HP_PaintJet data.c file.
	Originally written by Nick V. Flor of Hewlett Packard.
	Modified by David Berezowski, Commodore-Amiga

	Functions imnplemented:

	Special functions implemented:
*/

extern char InitSeq[];

char ReverseLF[] = "\033&a-120V";
char PartialUp[] = "\033&a-60V";

char *CommandTable[] = {
    InitSeq,			/* 00 -- reset */
    "\377",             /* 01 -- initialize */
    "\012",             /* 02 -- <LF> */
    "\015\012",         /* 03 -- <CR><LF> */
    ReverseLF,          /* 04 -- reverse <LF> */

    "\033&d@\033(s0B",  /* 05 -- normal character set */
    "\377",             /* 06 -- italics ON */
    "\377",             /* 07 -- italics OFF */
    "\033&dD",          /* 08 -- underline ON */
    "\033&d@",          /* 09 -- underline OFF */
    "\033(s1B",         /* 10 -- boldface ON */
    "\033(s0B",         /* 11 -- boldface OFF */
    "\377",             /* 12 -- set foreground color */
    "\377",             /* 13 -- set background color */

    "\033&k0S",         /* 14 -- normal pitch */
    "\033&k4S",         /* 15 -- elite ON */
    "\033&k0S",         /* 16 -- elite OFF */
    "\033&k2S",         /* 17 -- condensed fine ON */
    "\033&k0S",         /* 18 -- condensed fine OFF */
    "\377",             /* 19 -- enlarged ON */
    "\377",             /* 20 -- enlarged OFF */
    "\377",             /* 21 -- shadow print ON */
    "\377",             /* 22 -- shadow print OFF */
    "\377",             /* 23 -- double strike ON */
    "\377",             /* 24 -- double strike OFF */
    "\377",             /* 25 -- NLQ ON */
    "\377",             /* 26 -- NLQ OFF */
    "\377",             /* 27 -- superscript ON */
    "\377",             /* 28 -- superscript OFF */
    "\377",             /* 29 -- subscript ON */
    "\377",             /* 30 -- subscript OFF */
    "\377",             /* 31 -- normalize */
    PartialUp,          /* 32 -- partial line up */
    "\033=",            /* 33 -- partial line down */
    "\033(0U\033)0U\033(s0B\033)s1B",   /* 34 -- US char set */
    "\033(1F\033)1F\033(s0B\033)s1B",   /* 35 -- French char set */
    "\033(1G\033)1G\033(s0B\033)s1B",   /* 36 -- German char set */
    "\033(1E\033)1E\033(s0B\033)s1B",   /* 37 -- UK char set */
    "\033(0D\033)0D\033(s0B\033)s1B",   /* 38 -- Danish 1 char set */
    "\033(0S\033)0S\033(s0B\033)s1B",   /* 39 -- Swedish char set */
    "\033(0I\033)0I\033(s0B\033)s1B",   /* 40 -- Italian char set */
    "\033(2S\033)2S\033(s0B\033)s1B",   /* 41 -- Spanish char set */
    "\033(0U\033)0U\033(s0B\033)s1B",   /* 42 -- Japanese char set */
    "\033(0D\033)0D\033(s0B\033)s1B",   /* 43 -- Norwegian char set */
    "\033(0D\033)0D\033(s0B\033)s1B",   /* 44 -- Danish 2 char set */
    "\377",             /* 45 -- proportional ON */
    "\377",             /* 46 -- proportional OFF */
    "\377",             /* 47 -- proportional clear */
    "\377",             /* 48 -- set proportional offset */
    "\377",             /* 49 -- auto left justify */
    "\377",             /* 50 -- auto right justify */
    "\377",             /* 51 -- auto full justify */
    "\377",             /* 52 -- auto justify off */
    "\377",             /* 53 -- letter space (justify) */
    "\377",             /* 54 -- word fill (auto center) */
    "\033&l8D",         /* 55 -- 1/8" line spacing */
    "\033&l6D",         /* 56 -- 1/6" line spacing */
    "\377",             /* 57 -- set form length n */
    "\033&l1L",         /* 58 -- perf skip ON */
    "\033&l0L",         /* 59 -- perf skip OFF */
    "\377",             /* 60 -- left margin set */
    "\377",             /* 61 -- right margin set */
    "\377",             /* 62 -- top margine set */
    "\377",             /* 63 -- bottom margin set */
    "\377",             /* 64 -- T&B margins */
    "\377",             /* 65 -- L&R margins */
    "\377",             /* 66 -- clear margins */
    "\377",             /* 67 -- set horiz tab */
    "\377",             /* 68 -- set vertical tabs */
    "\377",             /* 69 -- clr horiz tab */
    "\377",             /* 70 -- clear all h tab */
    "\377",             /* 71 -- clr vertical tabs */
    "\377",             /* 72 -- clr all v tabs */
    "\377",             /* 73 -- clr all h & v tabs */
    "\377",             /* 74 -- set default tabs */
    "\377",             /* 75 -- extended commands */
	"\377",				/* 76 -- raw write (placeholder) */
};

char *ExtendedCharTable[] = {
/*
    " ", "!", "c", "L", "o", "Y", "|", "S",

    "\"", "c", "a", "<", "~", "-", "r", "-",

    "*", "+", "2", "3", "'", "u", "P", ".",

    ",", "1", "o", ">", "/", "/", "/", "?",

    "A", "A", "A", "A", "A", "A", "A", "C",

    "E", "E", "E", "E", "I", "I", "I", "I",

    "D", "N", "O", "O", "O", "O", "O", "x",

    "O", "U", "U", "U", "U", "Y", "P", "B",

    "a", "a", "a", "a", "a", "a", "a", "c",

    "e", "e", "e", "e", "i", "i", "i", "i",

    "d", "n", "o", "o", "o", "o", "o", "/",

    "o", "u", "u", "u", "u", "y", "p", "y"
*/

	"\033(0N\240", "\033(0N\241", "\033(0N\242", "\033(0N\243",
	"\033(0N\244", "\033(0N\245", "\033(0N\246", "\033(0N\247",

	"\033(0N\250", "\033(0N\251", "\033(0N\252", "\033(0N\253",
	"\033(0N\254", "\033(0N\255", "\033(0N\256", "\033(0N\257",

	"\033(0N\260", "\033(0N\261", "\033(0N\262", "\033(0N\263",
	"\033(0N\264", "\033(0N\265", "\033(0N\266", "\033(0N\267",

	"\033(0N\270", "\033(0N\271", "\033(0N\272", "\033(0N\273",
	"\033(0N\274", "\033(0N\275", "\033(0N\276", "\033(0N\277",

	"\033(0N\300", "\033(0N\301", "\033(0N\302", "\033(0N\303",
	"\033(0N\304", "\033(0N\305", "\033(0N\306", "\033(0N\307",

	"\033(0N\310", "\033(0N\311", "\033(0N\312", "\033(0N\313",
	"\033(0N\314", "\033(0N\315", "\033(0N\316", "\033(0N\317",

	"\033(0N\320", "\033(0N\321", "\033(0N\322", "\033(0N\323",
	"\033(0N\324", "\033(0N\325", "\033(0N\326", "\033(0N\327",

	"\033(0N\330", "\033(0N\331", "\033(0N\332", "\033(0N\333",
	"\033(0N\334", "\033(0N\335", "\033(0N\336", "\033(0N\337",

	"\033(0N\340", "\033(0N\341", "\033(0N\342", "\033(0N\343",
	"\033(0N\344", "\033(0N\345", "\033(0N\346", "\033(0N\347",

	"\033(0N\350", "\033(0N\351", "\033(0N\352", "\033(0N\353",
	"\033(0N\354", "\033(0N\355", "\033(0N\356", "\033(0N\357",

	"\033(0N\360", "\033(0N\361", "\033(0N\362", "\033(0N\363",
	"\033(0N\364", "\033(0N\365", "\033(0N\366", "\033(0N\367",

	"\033(0N\370", "\033(0N\371", "\033(0N\372", "\033(0N\373",
	"\033(0N\374", "\033(0N\375", "\033(0N\376", "\033(0N\377",
};
