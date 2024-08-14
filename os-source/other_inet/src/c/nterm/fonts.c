/* All Disk Font Stuff is done here */

#include "exec/types.h"
#include "libraries/diskfont.h"
#include "graphics/text.h"

/*#define DEBUG /* */

ULONG DiskfontBase; /* library ptr */
struct TextFont *tf_gfx = 0, *tf_spcgfx = 0, *tf_vt100_80 = 0;
struct TextFont *tf_gfx_128 = 0, *tf_spcgfx_128 = 0, *tf_vt100_128 = 0;

InitDiskFonts() /* initialize the disk fonts */
{
struct TextAttr ta;

	if ((DiskfontBase = OpenLibrary("diskfont.library",0))==0) {
		return(FALSE); /* couldn't open library */
	}
#ifdef DEBUG
printf("diskfont.library status = %ld\n",DiskfontBase);
#endif

	ta.ta_YSize = 8; /* set size */
	ta.ta_Style = FS_NORMAL; /* set style */
	ta.ta_Flags = FPF_ROMFONT | FPF_DISKFONT | FPF_DESIGNED; /* set flags */

	ta.ta_Name = "vt100.font"; /* set name */
	tf_vt100_80 = (struct TextFont *)OpenDiskFont(&ta);
#ifdef DEBUG
printf("vt100_80 disk font status = %ld\n",tf_vt100_80);
#endif

	ta.ta_Name = "vt128.font"; /* set name */
	tf_vt100_128 = (struct TextFont *)OpenDiskFont(&ta);
#ifdef DEBUG
printf("vt128 disk font status = %ld\n",tf_vt100_128);
#endif

	ta.ta_Name = "gfx.font"; /* set name */
	tf_gfx = (struct TextFont *)OpenDiskFont(&ta);
#ifdef DEBUG
printf("gfx disk font status = %ld\n",tf_gfx);
#endif

	ta.ta_Name = "gfx128.font"; /* set name */
	tf_gfx_128 = (struct TextFont *)OpenDiskFont(&ta);
#ifdef DEBUG
printf("gfx128 disk font status = %ld\n",tf_gfx_128);
#endif

	ta.ta_Name = "spcgfx.font"; /* set name */
	tf_spcgfx = (struct TextFont *)OpenDiskFont(&ta);
#ifdef DEBUG
printf("spcgfx disk font status = %ld\n",tf_spcgfx);
#endif

	ta.ta_Name = "spcgfx128.font"; /* set name */
	tf_spcgfx_128 = (struct TextFont *)OpenDiskFont(&ta);
#ifdef DEBUG
printf("spcgfx128 disk font status = %ld\n",tf_spcgfx_128);
#endif

}

CloseDiskFonts() /* close all disk fonts */
{
	if (tf_spcgfx_128) CloseFont(tf_spcgfx_128); /* close if it was opened */
	if (tf_spcgfx) CloseFont(tf_spcgfx); /* close if it was opened */
	if (tf_gfx_128) CloseFont(tf_gfx_128); /* close if it was opened */
	if (tf_gfx) CloseFont(tf_gfx); /* close if it was opened */
	if (tf_vt100_128) CloseFont(tf_vt100_128); /* close if it was opened */
	if (tf_vt100_80) CloseFont(tf_vt100_80); /* close if it was opened */
	if (DiskfontBase) CloseLibrary(DiskfontBase); /* close if it was opened */
}
