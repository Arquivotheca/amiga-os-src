/*************************************
* aspe.h local include file for aspe *
* (c)1990 CBM Inc.                   *
* written by Darius Taghavy (dart)   *
*************************************/

/* maximum number of test iterations */
#define ASPE_MAX_TESTS 25L

/* baud rate */
#define ASPE_MINBAUDRATE 300
#define ASPE_BAUDRATE 31250
#define ASPE_MAXBAUDRATE 19200*3

/* total transmitted bytes per test iteration */
#define ASPE_MINTOTALBYTES 1
#define ASPE_TOTALBYTES 3000
#define ASPE_MAXTOTALBYTES 32000

/* bytes per transmit interrupt */
#define ASPE_MINPACKETSIZE 1
#define ASPE_PACKETSIZE 3
#define ASPE_MAXPACKETSIZE ASPE_MAXTOTALBYTES

/* eof */