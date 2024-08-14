/*
 * This file is for the functions for the database of CDPlay to use.
 *
 *  functions include:
 *
 *  matchRecord()
 *  findRecord()
 *  readRecord()
 *  writeRecord()
 *  createIndex()
 *
 * Written by Greg Givler © March 1992
 *
 */

#include "cdplay.h"

struct cdinfo *matchRecord(int newrecord[SEARCH_FIELDS], int oldrecord[SEARCH_FIELDS] )
{
    /* this function will match records up */

}

int findRecord(struct index *idx,int searchfield)
{
    /* This function will find a records starting position
        in the index */

}

struct cdinfo *readRecord(int start, char *file)
{
    /* This function will read the record from the disk file and will
        fill in the cdinfo structure. */
}

bool writeRecord(int start, char *file)
{
    /* This function will write the record to the disk file from the
        cdinfo structure. Success indicates that record was written. */
}

bool createIndex(char *file)
{
    /* This will go thru the save file and create an index that will
        be placed in memory for findRecord() to use. */

}
