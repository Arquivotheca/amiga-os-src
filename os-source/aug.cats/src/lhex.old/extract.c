/***********************************************************
	extract.c -- extract file from archive
***********************************************************/
#include "lharc.h"
#include "intrface.h"
extern int verify_mode;

int decode_lzhuf (BPTR infp, BPTR outfp, long original_size, 
		long packed_size, char *name, int method)
{
  interface.method = method;
  interface.dicbit = 13; /* method + 8; */
  interface.infile = infp;
  interface.outfile = outfp;
  interface.original = original_size;
  interface.packed = packed_size;

  switch (method) {
  case 0:
    start_indicator (name, original_size,
	verify_mode ? " Testing:    " : " Extracting: ",2048, TRUE);
  case 8:
    start_indicator (name, original_size,
	verify_mode ? " Testing:    " : " Extracting: ",2048, FALSE);
    copyfile(infp, (verify_mode ? NULL : outfp), original_size, 2);
    break;
  case 6:	/* -lzs- */
    interface.dicbit = 11;
    start_indicator (name, original_size,
	verify_mode ? " Testing:    " : " Extracting: ",
		     1<<interface.dicbit, FALSE);
    decode(&interface);
    break;
  case 1: /* -lh1- */
  case 4: /* -lh4- */
  case 7: /* -lz5- */
    interface.dicbit = 12;
  default:
    start_indicator (name, original_size,
	verify_mode ? " Testing:    " : " Extracting: ",
		     1<<interface.dicbit, FALSE);
    decode(&interface);
  }
  finish_indicator (name, verify_mode ? " Tested:     " : " Extracted:  ");
  return crc;
}