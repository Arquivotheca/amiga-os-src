/************************************************************************
 * pcddata.h - Private include file for pcdisk private information
 ************************************************************************/

#define MAXFILES  8                    /* Maximum number of open files  */

struct OpenFile {
   struct FileHandle *fh;
   int cur_pos;
   };

struct FileTable {
   struct OpenFile File[MAXFILES];
   };

void dprintf(char *format,...);
