

#include "internal/commands.h"
#include "avail_rev.h"

/*                  ssss   dddddddd  dddddddd  dddddddd  dddddddd    */
#define MSG_STATS  "Type  Available    In-Use   Maximum   Largest\n" \
                   "chip%11ld %9ld %9ld %9ld\n"                      \
                   "fast%11ld %9ld %9ld %9ld\n"                      \
                   "total%10ld %9ld %9ld %9ld\n"
#define MSG_NOTBOTH "only one of CHIP, FAST, or TOTAL allowed\n"


#define TEMPLATE  "CHIP/S,FAST/S,TOTAL/S,FLUSH/S" CMDREV
#define OPT_CHIP  0
#define OPT_FAST  1
#define OPT_TOTAL 2
#define OPT_FLUSH 3
#define OPT_COUNT 4

int cmd_avail(void)
{
   struct Library *SysBase = (*((struct Library **) 4));
   struct DosLibrary *DOSBase;
   ULONG              max_chip, max_fast;
   ULONG              avail_chip, avail_fast;
   ULONG              large_chip, large_fast;
   long               parray[13];
   long               opts[OPT_COUNT];
   struct RDargs     *rdargs;
   char              *txt;
   int                rc;
   UWORD              i;

   rc = RETURN_FAIL;
   if (DOSBase = (struct DosLibrary *)OpenLibrary(DOSLIB,DOSVER))
   {
      memset((char *)opts, 0, sizeof(opts));
      rdargs = ReadArgs(TEMPLATE, opts, NULL);

      if (rdargs == NULL)
      {
         PrintFault(IoErr(), NULL);
      }
      else
      {

        rc = RETURN_OK;
	if(opts[OPT_FLUSH])	/* they want expunge first */
	{
	    i = 0;
	    while (i < 10)
	    {
                FreeVec(AllocVec(0x7ffffff0,MEMF_PUBLIC));
                i++;
            }
	}
         /*-------------------------------------------------------------*/
         /* Ask the system for the available numbers.  We do this under */
         /* forbid to ensure consistent numbers.                        */
         /*-------------------------------------------------------------*/
	 Forbid();
	 max_chip   = AvailMem (MEMF_CHIP | MEMF_TOTAL);
         avail_chip = AvailMem (MEMF_CHIP);
         large_chip = AvailMem (MEMF_CHIP | MEMF_LARGEST);

	 max_fast   = AvailMem (MEMF_FAST | MEMF_TOTAL);
         avail_fast = AvailMem (MEMF_FAST);
         large_fast = AvailMem (MEMF_FAST | MEMF_LARGEST);
         Permit ();


         /*-------------------------------------------------------------*/
         /* Assume that they are only going to ask for one thing        */
         /* default to giving them the amount of CHIP until we learn    */
         /* better from looking at the options.                         */
         /*-------------------------------------------------------------*/
         txt = "%ld\n";
         parray[0] = avail_chip;

         if (opts[OPT_TOTAL])
         {
	    if (opts[OPT_FAST] || opts[OPT_CHIP])
	    {
               txt = MSG_NOTBOTH;
	       rc = RETURN_WARN;
	    }
	    else
	    {
               /*----------------------------------------------------------*/
               /* They only want the total amount of memory.  Add up the   */
               /* amount of fast memory to the alredy totaled chip         */
	       /*----------------------------------------------------------*/
               parray[0] += avail_fast;
	    }
         }

         else if (opts[OPT_FAST])
         {
            if (opts[OPT_CHIP])
            {
               /* They asked for both chip and fast, give them an error */
               txt = MSG_NOTBOTH;
               rc = RETURN_WARN;
            }
            else
            {
               /*-------------------------------------------------------*/
               /* They want the amount of fast memory                   */
               /*-------------------------------------------------------*/
               parray[0] = avail_fast;
            }
         }

         /*-------------------------------------------------------------*/
         /* If they ask for the amount of chip ram, it is already set   */
         /* up, so we can just fall through to print it.                */
         /*-------------------------------------------------------------*/
         else if (!opts[OPT_CHIP])
         {
            /*----------------------------------------------------------*/
            /* They asked for everything, calculate the values to be    */
            /* substituted into the message stats                       */
            /*----------------------------------------------------------*/
            txt =  MSG_STATS;
            parray[ 1] = max_chip-avail_chip;   /* Chip memory in use   */
            parray[ 2] = max_chip;              /* total chip memory    */
            parray[ 3] = large_chip;            /* Largest chip piece   */

            parray[ 4] = avail_fast;            /* available fast memory*/
            parray[ 5] = max_fast-avail_fast;   /* fast memory in use   */
            parray[ 6] = max_fast;              /* total fast memory    */
            parray[ 7] = large_fast;            /* largest fast piece   */

            parray[ 8] = avail_chip+avail_fast; /* all memory in use    */
            parray[ 9] = parray[1]+parray[5];
            parray[10] = max_chip+max_fast;     /* total all memory     */
            parray[11] = large_chip;            /* largest piece        */
            if (large_fast > large_chip) parray[11] = large_fast;
         }

         /*-------------------------------------------------------------*/
         /* The messge and substitutions are in place so print them     */
         /*-------------------------------------------------------------*/
         VPrintf(txt, parray);

         /*-------------------------------------------------------------*/
         /* Clean up and exit                                           */
         /*-------------------------------------------------------------*/
         FreeArgs(rdargs);
      }

      CloseLibrary((struct Library *)DOSBase);
   }
   else
   {
      OPENFAIL;

   }
   return(rc);
}
