/************************************************************************
 *
 * LockTest.h - LockTest specific data structures
 *
 * This file is intended for use on both the Amiga and PC side. Therefore,
 * it should contain only information important to both sides, and should
 * not contain any information used exclusively by one side.
 *
 * 10-09-90  -  Bill Koester - Created this file
 ************************************************************************/

#define LOCKTEST_APPLICATION_ID 1L
#define LOCKTEST_LOCAL_ID 2

/* Shared structure for Lock testing */

struct LockServReq {
   unsigned char  lsr_b1;
   unsigned char  lsr_b2;
};
