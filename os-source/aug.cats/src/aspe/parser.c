/* command line parser for A.S.P.E. */

#include <exec/types.h>
#include <stdio.h>
#include "aspe.h"

/* to turn this file into a standalone binary for development comment out
   the below define. To link with aspe leave as is */
#define FUNCTION

/* function prototypes */
void argparse(char *cp);
void help(void);

#ifdef FUNCTION
   extern UWORD baud_rate;    /* serial baud rate */
   extern LONG size;        /* number of bytes per interrupt */
   extern ULONG  num;     /* total number of bytes per test iteration */
   extern char which_timer;
   extern ULONG max_tests; /* maximum number of test iterations */
#endif

#ifndef FUNCTION
/* main program */
void main(int argc, char* argv[])
{
	register LONG i;

   /* if aspe was envoked with options do ... */
   if (argc >= 2)
      {
      /* Parse argument options (detected as those beginning with '-') */
      for (i = 1; i < argc && *argv[i] == '-'; ++i) argparse(argv[i]);
      if (*argv[i] == '?')help();
      }
   /* else use default values and continue with program here ... */
} /* end of main */
#endif

/* parser function */
void argparse(char *cp)
{
   extern int atoi();

   UWORD skipnum = 0;      /* parser flags */

#ifndef FUNCTION
   UWORD baud_rate = ASPE_BAUDRATE;    /* serial baud rate */
   LONG  size = (LONG) ASPE_PACKETSIZE;  /* number of bytes per interrupt */
   ULONG num =(LONG) ASPE_TOTALBYTES ; /* total # bytes per test iteration */
   char which_timer;
   ULONG max_tests; /* maximum number of test iterations */
#endif

   while (*cp != '\0')
      {
      if (skipnum) /* skip all digits of number */
         {
         if (!isdigit(*cp))
            {
            skipnum = 0;
            continue;
            }
         }
      else
         {
         switch(*cp)
            {
            /* skip arg introducer to allow multiple args in one string */
            case '-':   break;

            /* timer */
            case 't':   cp++;
                        if (*cp == 'a')
                           {
                           which_timer = 'a';
                           printf("\n Using cia timer a.");
                           }
                        else if (*cp == 'b')
                           {
                           printf("\n Using cia timer b.");
                           which_timer = 'b';
                           }
                        else
                           {
                           printf("\n Defaulting to timer b");
                           which_timer = 'b';
                           }
                        break;

            /* baudrate */
            case 'b':   baud_rate = atoi(++cp);
                        if (baud_rate <= ASPE_MINBAUDRATE)
                           baud_rate = ASPE_MINBAUDRATE;
                        else if (baud_rate >= ASPE_MAXBAUDRATE)
                           baud_rate = ASPE_MAXBAUDRATE;
#ifndef FUNCTION
                        printf("\n Baudrate = %ld",baud_rate);
#endif
                        skipnum = 1;
                        break;

            /* bytes per interrupt */
            case 'p':   size = (ULONG)atoi(++cp);
                        if (size <= ASPE_MINPACKETSIZE)
                           size =  ASPE_MINPACKETSIZE;
                        else if (size >= ASPE_MAXPACKETSIZE)
                           size =  ASPE_MAXPACKETSIZE;
#ifndef FUNCTION
                        printf("\n Packet size (bytes per transmit interrupt) = %ld",size);
#endif
                        skipnum = 1;
                        break;

            /* number of test iterations */
            case 'i':   max_tests = (ULONG)atoi(++cp);
                        if (max_tests <= 0) max_tests = 1L;
#ifndef FUNCTION
                        printf("\n Number of test iterations = %ld",max_tests);
#endif
                        skipnum = 1;
                        break;

            /* total number of bytes to send per iteration of test */
            case 'n':   num = atoi(++cp);
                        if (num <= ASPE_MINTOTALBYTES)
                           num = ASPE_MINTOTALBYTES;
                        else if(num >= ASPE_MAXTOTALBYTES)
                           num = ASPE_MAXTOTALBYTES;
#ifndef FUNCTION
                        printf("\n Total number of transmitted bytes per test iteration = %ld",num);
#endif
                        skipnum = 1;
                        break;

            case 'h':   help();
                        break;

            default:    printf("\n Unknown option: %c", *cp);
                        break;
            }
         }
      ++cp;
      }
}

void help(void)
{
   printf("\n\n Usage: aspe [options]");
   printf("\n\n options         comment\n");
   printf("  -b [n]         Set serial baudrate to n\n");
   printf("  -p [n]         Set packet size (bytes per transmit interrupt) to n\n");
   printf("  -i [n]         Set number of test iterations to n\n");
   printf("  -n [n]         Set total number of bytes to send per test iteration to n\n");
   printf("  -t [a or b]    Use ciaa (-ta) or ciab (-tb) timer  \n");
   printf("\n\n Multiple options can be invoked in any order, e.g.:");
   printf("\n \"aspe -b%ld -p%ld -n%ld\" which is equivalent to \"aspe -b%ldp%ldn%ld\"",
                ASPE_BAUDRATE,ASPE_PACKETSIZE,ASPE_TOTALBYTES,
                ASPE_BAUDRATE,ASPE_PACKETSIZE,ASPE_TOTALBYTES);
   printf("\n Missing options are assigned default values.\n\n");
   printf("\x9b\x20\x70");
   exit(0);
}

/* eof */