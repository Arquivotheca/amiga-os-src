/* -----------------------------------------------------------------------
 * kermit.c		handshake_src
 *
 * $Locker:  $
 *
 * $Id: kermit.c,v 1.1 91/05/09 16:28:53 bj Exp $
 *
 * $Revision: 1.1 $
 *
 * $Header: HOG:Other/inet/src/c/hs/RCS/kermit.c,v 1.1 91/05/09 16:28:53 bj Exp $
 *
 * $Log:	kermit.c,v $
 * Revision 1.1  91/05/09  16:28:53  bj
 * Initial revision
 * 
 *
 *------------------------------------------------------------------------
 */

/*
 *      K e r m i t  File Transfer Utility
 *
 *      UNIX Kermit, Columbia University, 1981, 1982, 1983
 *      Bill Catchings, Bob Cattani, Chris Maio, Frank da Cruz
 *
 *      Fixed up again for Unix, Jim Guyton 7/13/83 (Rand Corp)
 */

/*
 *      Hacked up for the Amiga, Raymond S. Brand 19851130->
 *      Hacked up for HandShake by Eric Haberfellner.
 */

#include "termall.h"
#undef DEL
#include "libraries/dosextens.h"

/* Conditional Compilation: 0 means don't compile it, nonzero means do */

/* Symbol Definitions */

#define BLK_SZ          1536
#define MAXPACK         94      /* Maximum packet size */
#define SP              32      /* ASCII space */
#define CR              13      /* ASCII Carriage Return */
#define DEL             127     /* Delete (rubout) */
#define CTRLD           4
#define BRKCHR          CTRLD   /* Default escape character for CONNECT */

#define MAXTRY          5       /* Times to retry a packet */
#define MYQUOTE         '#'     /* Quote character I will use */
#define MYQBIN          '&'     /* Binary Quote character */
#define MYPAD           0       /* Number of padding characters I will need */
#define MYPCHAR         0       /* Padding character I need */
#define MYEOL           '\r'    /* End-Of-Line character I need */
#define MYTIME          10      /* Seconds after which I should be timed out */
#define MAXTIM          20      /* Maximum timeout interval */
#define MINTIM          2       /* Minumum timeout interval */



/* Global Variables */

static unsigned long int File_size,
                         Byte_count;
static int     size,                   /* Size of present data */
        n,                      /* Message number */
        rpsiz,                  /* Maximum receive packet size */
        spsiz,                  /* Maximum send packet size */
        pad,                    /* How much padding to send */
        timint,                 /* Timeout for foreign host on sends */
        numtry,                 /* Times this packet retried */
        oldtry,                 /* Times previous packet retried */
        image,                  /* -1 means 8-bit mode */
        transfer_complete,      /* Completion flag */
        transfer_aborted,       /* Success flag */
        files_idx,
        debug = 0;              /* -1 means debugging */

static char    state,                  /* Present state of the automaton */
        padchar = ' ',          /* Padding character to send */
        eol,                    /* End-Of-Line character to send */
        escchr,                 /* Connect command escape character */
        quote,                  /* Quote character in incoming data */
        qbin,                   /* Binary quote character in incoming data */
        bin7bit,                /* 7 bit binary indicator */
        *filnam,                /* File Name */
        recpkt[MAXPACK],        /* Receive packet buffer */
        packet[MAXPACK];        /* Packet buffer */
        
static  unsigned char **files;


/* Program code */

unsigned short int __stdargs BeginKermitReceive ( unsigned int image_val,
                                        unsigned char *path )
  {
    if ( ftcb.trans_act             &&
         ftcb.protocol  != XON_XOFF &&
         nvmodes.direction != INCOMING )
         return ( 0 );

    if ( image_val == 1 )
        SetXmodemParameters ();
    else
        SetKermitParameters ();
    
    if ( path )
        strcpy ( nvmodes.dest_path, path );
    else if ( !FileReq ( "Destination Path", nvmodes.dest_path ) )
      {
        if ( image_val == 1 )
            RestoreParameters ();
        else
            RestoreKermitParameters ();
        return ( 0 );
      }
    if ( nvmodes.dest_path [ strlen ( nvmodes.dest_path ) - 1 ] != '/' &&
         nvmodes.dest_path [ strlen ( nvmodes.dest_path ) - 1 ] != ':' )
        strcat ( nvmodes.dest_path, "/" );

    Binit ();
    nvmodes.direction = INCOMING;
    ftcb.trans_act    = 1;

/*-----*/

    if ( !OpenTransferWindow ( "KERMIT Receive", 500, 55 ) )
            goto kermit_receive_aborted;

    SetFile ( "" );
    SetMessage ( "" );
    
    transfer_aborted  = 0;
    transfer_complete = 0;
    
    Byte_count = 0L;
    File_size  = 0L;
    pad     = MYPAD;        /* How much padding to send */
    timint  = MYTIME;       /* Timeout for foreign host on sends */
    image   = image_val;    /* Image mode off */
    if ( image == 2 )       /* Set transfer mode */
      {
        bin7bit = 1;
        qbin = MYQBIN;      /* Enable binary quoting if necessary */
      }
    else
      {
        bin7bit = 0;
        qbin = 0;
      }
    padchar = MYPCHAR;      /* Padding character to send */
    eol     = MYEOL;        /* End-Of-Line character to send */
    
    ftcb.blockno = 0;
    ftcb.total_errors = 0;
        
    Recsw ();
    
kermit_receive_aborted:;

    if ( image_val == 1 )
        RestoreParameters ();
    else
        RestoreKermitParameters ();

    CloseTransferWindow ();

    if ( transfer_complete )
      {
        TransferMessage ( "\r\n\nReceive Successful\n\n\r" );
        CompleteBeep ();
      }
    else if ( ftcb.retries == MAX_RETRIES )
      {
        TransferMessage ( "\r\n\nToo Many retries\n\n\r" );
        FailedBeep ();
      }
    else
      {
        TransferMessage ( "\r\n\nTransmission aborted\n\n\r" );
        /*
        SendCANs ();
        */
        FailedBeep ();
      }


/*-----*/

    ftcb.trans_act = 0;

    return ( (unsigned short int)transfer_complete );
  }

unsigned short int __stdargs BeginKermitSend ( unsigned int image_val,
                                                  unsigned char *file )
  {
    unsigned char  **file_list ( unsigned char *);
    
    if ( ftcb.trans_act                &&
         ftcb.protocol     != XON_XOFF &&
         nvmodes.direction != INCOMING )
        return ( 0 );
    
    if ( image_val == 1 )
        SetXmodemParameters ();
    else
        SetKermitParameters ();
    
    if ( file )
        strcpy ( nvmodes.pc_file, file );
    else if ( !FileReq ( "File(s)", nvmodes.pc_file ) )
      {
        if ( image_val == 1 )
            RestoreParameters ();
        else
            RestoreKermitParameters ();
        return ( 0 );
      }
    
    if ( !(files = FileList ( nvmodes.pc_file ) ) )
      {
        Error ( "Could not build file list" );
        if ( image_val == 1 )
            RestoreParameters ();
        else
            RestoreKermitParameters ();
        return ( 0 );
      }
      
    files_idx = 0;
    
    Binit ();
    ftcb.protocol  = nvmodes.protocol;
    nvmodes.direction = OUTGOING;
    ftcb.trans_act    = 1;

/*-----*/

    if ( !OpenTransferWindow ( "KERMIT Transmit", 500, 65 ) )
        goto kermit_transfer_aborted;
    
    if ( !GNxtFl () )
      goto kermit_transfer_aborted;
      
    SetMessage ( "" );
    
    transfer_aborted  = 0;
    transfer_complete = 0;
    
    Byte_count = 0L;
    File_size  = 0L;
    pad     = MYPAD;        /* How much padding to send */
    timint  = MYTIME;       /* Timeout for foreign host on sends */
    image   = image_val;    /* Image mode off */
    if ( image == 2 )
      {
        bin7bit = 1;
        qbin = MYQBIN;       /* Eanble binary quoting if necessary */
      }
    else
      {
        bin7bit = 0;
        qbin = 0;
      }
    padchar = MYPCHAR;      /* Padding character to send */
    eol     = MYEOL;        /* End-Of-Line character to send */
    
    ftcb.blockno = 0;
    ftcb.total_errors = 0;
        
    Sendsw ();

/*-----*/


kermit_transfer_aborted:;

    if ( image_val == 1 )
        RestoreParameters ();
    else
        RestoreKermitParameters ();

    CloseTransferWindow ();

    if ( transfer_complete )
      {
        TransferMessage ( "\r\n\nTransmit Successful\n\n\r" );
        CompleteBeep ();
      }
    else if ( ftcb.retries == MAX_RETRIES )
      {
        TransferMessage ( "\r\n\nToo Many retries\n\n\r" );
        FailedBeep ();
      }
    else
      {
        TransferMessage ( "\r\n\nTransmission aborted\n\n\r" );
        /*
        SendCANs ();
        */
        FailedBeep ();
      }

    ftcb.trans_act = 0;

    return ( (unsigned short int) transfer_complete );

  }

/*
 *      s e n d s w
 *
 *      Sendsw is the state table switcher for sending
 *      files.  It loops until either it finishes, or
 *      an error is encountered.  The routines called by
 *      sendsw are responsible for changing the state.
 *
 */

void Sendsw ()
  {
    char Sinit (),sfile(),seof(),sdata(),sbreak();

    state = 'S';                           /* Send initiate is the start state */
    n = 0;                                 /* Initialize message number */
    numtry = 0;                            /* Say no tries yet */
    while(TRUE)                            /* Do this as long as necessary */
      {
        SetBytes ( Byte_count );
        SetErrorCounts ( numtry, ftcb.total_errors );
        if ( debug ) printf ("state %c\n",state);
        switch(state)
          {
            case 'D':  state = Sdata ();  break; /* Data-Send state */
            case 'F':  state = Sfile ();  break; /* File-Send */
            case 'Z':  state = SEOF ();   break; /* End-of-File */
            case 'S':  state = Sinit ();  break; /* Send-Init */
            case 'B':  state = Sbreak (); break; /* Break-Send */
            case 'C':
                transfer_complete = 1;
                return;           /* Complete */
            case 'A':
            default :
            strcpy ( packet, "Aborting Transfer" );
            Spack ( 'E',n,17,packet );
            transfer_aborted = 1;
            if ( ftcb.fh )
              {
                _dclose ( ftcb.fh );
                ftcb.fh = NULL;
              }
            return;          /* "Abort" */
          }
        if (transfer_aborted )
          {
            strcpy ( packet, "Aborting Transfer" );
            Spack ( 'E',n,17,packet );
            if ( ftcb.fh )
              {
                _dclose ( ftcb.fh );
                ftcb.fh = NULL;
              }
            return;
          }
      }
    return;
  }

/*
 *      s i n i t
 *
 *      Send Initiate: Send my parameters, get other side's back.
 */

char Sinit ()
  {
    int num, len;                             /* Packet number, length */

    if (debug) printf("sinit\n");
    if (numtry++ > MAXTRY) return('A');       /* If too many tries, give up */
    Spar (packet);                             /* Fill up with init info */
    if (debug) printf("n = %d\n",n);

    Spack ('S',n,bin7bit?7:6,packet);          /* Send an S packet */
    switch(Rpack(&len,&num,recpkt))           /* What was the reply? */
      {
        case 'N':
            SetMessage ( "NAK received to Initialization Packet" );
            ftcb.total_errors++;
            return(state);                    /* NAK */

        case 'Y':                             /* ACK */
            if (n != num) return(state);      /* If wrong ACK, stay in S state */
            Rpar (recpkt);                     /* Get other side's init info */
            if (eol   == 0) eol   = MYEOL;    /* Check and set defaults */
            if (quote == 0) quote = MYQUOTE;  /* Control-prefix quote */
            numtry = 0;                       /* Reset try counter */
            n = (n+1)%64;                     /* Bump packet count */
            ftcb.blockno++;
            if (debug) printf("Opening %s\n",filnam);

            Binit ();
            ftcb.fh = _dopen(filnam,MODE_OLDFILE);/* Open the file to be sent */
            ShortName ( filnam );
            if (ftcb.fh < 0) return('A');     /* if bad file descriptor, give up */
            if ( debug ) printf("Sending %s\n",filnam);
            File_size = _dseek ( ftcb.fh, 0L, 2 );
            _dseek ( ftcb.fh, 0L, 0 );
            SetSize ( File_size );
      
            return('F');                      /* OK, switch state to F */

        case FALSE:
            Spack ('N',num,0,0);
            return(state);                    /* Receive failure, stay in
                                                 S state */
        
        default:
            return('A');                      /* Anythig else, just "abort" */
      }
    return ( 0 );
  }


/*
 *      s f i l e
 *
 *      Send File Header.
 */

char Sfile ()
  {
    int num, len;                            /* Packet number, length */

    if (debug) printf("sfile\n");

    if (numtry++ > MAXTRY) return('A');      /* If too many tries, give up */
    for (len=0; filnam[len] != '\0'; len++); /* Add up the length */

    Spack ('F',n,len,filnam);                 /* Send an F packet */
    switch(Rpack(&len,&num,recpkt))          /* What was the reply? */
      {                                    
        case 'N':                            /* NAK, just stay in this state, */
            if (n != (num=(--num<0)?63:num)) /*  unless NAK for next packet, */
              {
                SetMessage ( "NAK Reveiced to File Header" );
                ftcb.total_errors++;
                return(state);               /*  which is just like an ACK */
              }
                                             /*  for this packet, fall thru to... */
        case 'Y':                            /* ACK */
            if (n != num) return(state);     /* If wrong ACK, stay in F state */
            numtry = 0;                      /* Reset try counter */
            n = (n+1)%64;                    /* Bump packet count */
            ftcb.blockno++;
            size = Bufill (packet);           /* Get first data from file */
            return('D');                     /* Switch state to D */

        case FALSE:
            Spack ('N',num,0,0);
            return(state);                   /* Receive failure, stay in F state */
        default:
            return('A');                     /* Something esle, just "abort" */
      }
    return ( 0 ); 
  }


/*
 *      s d a t a
 *
 *      Send File Data
 */

char Sdata ()
  {
    int num, len;                         /* Packet number, length */

    if (numtry++ > MAXTRY) return('A');   /* If too many tries, give up */
    Spack ('D',n,size,packet);             /* Send a D packet */

    switch(Rpack(&len,&num,recpkt))       /* What was the reply? */
      {                            
        case 'N':                         /* NAK, just stay in this state, */
        if (n != (num=(--num<0)?63:num))  /*  unless NAK for next packet, */
          {
            SetMessage ("NAK received to Data Packet" );
            ftcb.total_errors++;
            return(state);                /*  which is just like an ACK */
          }
                                          /*  for this packet, fall thru to... */
        case 'Y':                         /* ACK */
            if (n != num) return(state);  /* If wrong ACK, fail */
            numtry = 0;                   /* Reset try counter */
            n = (n+1)%64;                 /* Bump packet count */
            ftcb.blockno++;
            if ((size = Bufill (packet)) == EOF) /* Get data from file */
                return('Z');              /* If EOF set state to that */
            return('D');                  /* Got data, stay in state D */

        case FALSE:
            Spack ('N',num,0,0);
            return(state);                /* Receive failure, stay in D */
        
        default:
            return('A');                  /* Anything else, "abort" */
      }
    return ( 0 );
  }


/*
 *      s e o f
 *
 *      Send End-Of-File.
 */

char SEOF ()
  {
    int num, len;                         /* Packet number, length */
    if (debug) printf("seof\n");
    if (numtry++ > MAXTRY)
        return('A');                      /* If too many tries, "abort" */
    Spack ('Z',n,0,packet);                /* Send a 'Z' packet */
    if (debug) printf("seof1 ");
    switch(Rpack(&len,&num,recpkt))       /* What was the reply? */
      {
        case 'N':                         /* NAK, fail */
        if (n != (num=(--num<0)?63:num))  /* ...unless for previous packet, */
          {
            SetMessage ( "NAK received to End of File" );
            ftcb.total_errors++;
            return(state);                /*  in which case, fall thru to ... */
          }
        case 'Y':                         /* ACK */
            if (debug) printf("seof2 ");
            if (n != num) return(state);
            numtry = 0;                   /* Reset try counter */
            n = (n+1)%64;                 /* and bump packet count */
            ftcb.blockno++;
            if (debug) printf("closing %s, ",filnam);
            Bflush ();
            _dclose(ftcb.fh);
            ftcb.fh = NULL;               /* Close the input file */
            if (debug) printf("ok, getting next file\n");

            if (GNxtFl() == FALSE)        /* No more files go? */
                return('B');              /* if not, break, EOT, all done */
            if (debug) printf("new file is %s\n",filnam);
            Binit ();
            ftcb.fh = _dopen(filnam,MODE_OLDFILE);/* Open next file to send */
            ShortName ( filnam );
            if (ftcb.fh<0) return ('B');  /* if bad file descriptor, give up*/
                                          /* but close file just sent!! */
            if ( debug ) printf("Sending %s\n",filnam);
            File_size = _dseek ( ftcb.fh, 0L, 2 );
            _dseek ( ftcb.fh, 0L, 0 );
            SetSize ( File_size );

            return('F');                  /* More files, switch state to F */

        case FALSE:
            Spack ('N',num,0,0);
            return(state);                /* Receive failure, stay in state Z */
        default:
            return('A');                  /* Something else, "abort" */
      }
    return(0);
  }


/*
 *      s b r e a k
 *
 *      Send Break (EOT)
 */

char Sbreak ()
  {
    int num, len;                             /* Packet number, length */
    if (debug) printf("sbreak\n");
    if (numtry++ > MAXTRY) return('A');       /* If too many tries "abort" */

    Spack ('B',n,0,packet);                    /* Send a B packet */
    switch (Rpack(&len,&num,recpkt))          /* What was the reply? */
      {
        case 'N':                             /* NAK, fail */
            if (n != (num=(--num<0)?63:num))  /* ...unless for previous packet, */
              {
                SetMessage ( "NAK received to Break Packet" );
                ftcb.total_errors++;
                return(state);                /*  in which case, fall thru to ... */
              }

        case 'Y':                             /* ACK */
            if (n != num) return(state);      /* If wrong ACK, fail */

            numtry = 0;                       /* Reset try counter */
            n = (n+1)%64;                     /* and bump packet count */
            ftcb.blockno++;
            return('C');                      /* switch state to Complete */

        case FALSE:
            Spack ('N',num,0,0);
            return(state);                    /* Receive failure, stay in state B */
        
        default:
            return ('A');                     /* Other, "abort" */
      }
    return ( 0 );
  }


/*
 *      r e c s w
 *
 *      This is the state table switcher for receiving files.
 */

void Recsw ()
  {
    char Rinit (),rdata(),rfile();         /* Use these procedures */
  
    state = 'R';                          /* Receive is the start state */
    n = 0;                                /* Initialize message number */
    numtry = 0;                           /* Say no tries yet */

    while(TRUE)
      {
        SetBytes ( Byte_count );
        SetErrorCounts ( numtry, ftcb.total_errors );
        if (debug) printf( "state %c\n",state);
        switch(state)             /* Do until done */
          {
            case 'D':   state = Rdata (); break; /* Data receive state */
            case 'F':   state = Rfile (); break; /* File receive state */
            case 'R':   state = Rinit (); break; /* Send initiate state */
            case 'C':
                transfer_complete = 1;
                return;             /* Complete state */
            case 'A':
            default:
                strcpy ( packet, "Aborting Transfer" );
                Spack ( 'E',n,17,packet );
                transfer_aborted = 1;
                if ( ftcb.fh )
                  {
                    Bflush ();
                    _dclose ( ftcb.fh );
                    ftcb.fh = NULL;
                  }
                return;            /* "Abort" */
          }
        if (transfer_aborted )
          {
            strcpy ( packet, "Aborting Transfer" );
            Spack ( 'E',n,17,packet );
            if ( ftcb.fh )
              {
                Bflush ();
                _dclose ( ftcb.fh );
                ftcb.fh = NULL;
              }
            return;
          }
      }
    return;
  }
    
/*
 *      r i n i t
 *
 *      Receive Initialization
 */
  
char Rinit ()
  {
    int len, num;                         /* Packet length, number */

    if (debug) printf( "rinit\n");
    if (numtry++ > MAXTRY) return('A');   /* If too many tries, "abort" */
    switch(Rpack(&len,&num,packet))       /* Get a packet */
      {
        case 'S':                         /* Send-Init */
            Rpar (packet);                 /* Get the other side's init data */
            Spar (packet);                 /* Fill up packet with my init info */
            Spack ('Y',n,bin7bit?7:6,packet); /* ACK with my parameters */
            oldtry = numtry;              /* Save old try count */
            numtry = 0;                   /* Start a new counter */
            n = (n+1)%64;                 /* Bump packet number, mod 64 */
            ftcb.blockno++;
            return('F');                  /* Enter File-Send state */

        case FALSE:
            Spack ('N',num,0,0);
            return (state);               /* Didn't get a packet, keep waiting */
        default:
            return('A');                  /* Some other packet type, "abort" */
      }
    return (0);
  }


/*
 *      r f i l e
 *
 *      Receive File Header
 */

char Rfile ()
  {
    int num, len;                       /* Packet number, length */

    if (debug) printf( "rfile\n");
    if (numtry++ > MAXTRY) return('A'); /* "abort" if too many tries */
    switch(Rpack(&len,&num,packet))     /* Get a packet */
      {
        case 'S':                       /* Send-Init, maybe our ACK lost */
            if (oldtry++ > MAXTRY) return('A'); /* If too many tries, "abort" */
            if (num == ((n==0)?63:n-1)) /* Previous packet, mod 64? */
              {                         /* Yes, ACK it again */
                SetMessage ( "re-ACKING Init Packet" );
                ftcb.total_errors++;
                Spar (packet);           /* with our Send-Init parameters */
                Spack ('Y',num,bin7bit?7:6,packet);/*  ... */
                numtry = 0;             /* Reset try counter */
                return(state);          /* Stay in this state */
              }
            else return('A');           /* Not previous packet, "abort" */

        case 'Z':                       /* End-Of-File */
            if (oldtry++ > MAXTRY) return('A');
            if (num == ((n==0)?63:n-1)) /* Previous packet, mod 64? */
              {                         /* Yes, ACK it again. */
                SetMessage ( "re-ACKING End 0f File Packet" );
                ftcb.total_errors++;
                Spack ('Y',num,0,0);
                numtry = 0;
                return(state);          /* Stay in this state */
              }
            else return('A');           /* Not previous packet, "abort" */

        case 'F':                       /* File Header, */
            if (num != n) return('A');  /* which is what we really want */
                                        /* The packet number must be right */
            if (!GetFil())              /* Try to open a new file */
              {
                SetMessage ("Could not create file\n"); /* Give up if can't */
                ftcb.total_errors++;
                return('A');
              }
            Spack ('Y',n,0,0);            /* Acknowledge the file header */
            oldtry = numtry;             /* Reset try counters */
            numtry = 0;                  /* ... */
            n = (n+1)%64;                /* Bump packet number, mod 64 */
            ftcb.blockno++;
            return('D');                 /* Switch to Data state */

        case 'B':                        /* Break transmission (EOT) */
            if (num != n) return ('A');  /* Need right packet number here */
            Spack ('Y',n,0,0);            /* Say OK */
            return('C');                 /* Go to complete state */

        case FALSE:
            Spack ('N',num,0,0);
            return(state);       /* Couldn't get packet, keep trying */
        default:
            return ('A');       /* Some other packet, "abort" */
      }
    return (0);
  }


/*
 *      r d a t a
 *
 *      Receive Data
 */

char Rdata ()
  {
    int num, len;                               /* Packet number, length */

    if (debug) printf( "rdata\n");
    if (numtry++ > MAXTRY) return('A');         /* "abort" if too many tries */
    switch(Rpack(&len,&num,packet))             /* Get packet */
      {
        case 'D':                               /* Got Data packet */
            if (num != n)                       /* Right packet? */
              {                                 /* No */
                if (oldtry++ > MAXTRY) return('A'); /* If too many tries, give up */
                if (num == ((n==0)?63:n-1))     /* Else check packet number */
                  {                             /* Previous packet again? */
                    SetMessage ( "re-ACKING Data Packet" );
                    ftcb.total_errors++;
                    Spack ('Y',num,6,packet);    /* Yes, re-ACK it */
                    numtry = 0;                 /* Reset try counter */
                    return(state);              /* Stay in D, don't write out data! */
                  }
                else return('A');               /* sorry wrong number */
              }
                                                /* Got data with right packet number */
            Bufemp (packet,len);                 /* Write the data to the file */
            Spack ('Y',n,0,0);                   /* Acknowledge the packet */
            oldtry = numtry;                    /* Reset the try counters */
            numtry = 0;                         /* ... */
            n = (n+1)%64;                       /* Bump packet number, mod 64 */
            ftcb.blockno++;
            return('D');                        /* Remain in data state */

        case 'F':                               /* Got a File Header */
            if (oldtry++ > MAXTRY) return('A'); /* If too many tries, "abort" */
            if (num == ((n==0)?63:n-1))         /* Else check packet number */
              {                                 /* It was the previous one */
                SetMessage ( "re-ACKING File Header" );
                ftcb.total_errors++;
                Spack ('Y',num,0,0);             /* ACK it again */
                numtry = 0;                     /* Reset try counter */
                return(state);                  /* Stay in Data state */
              }
            else return('A');                   /* Not previous packet, "abort" */

        case 'Z':                               /* End-Of-File */
            if (num != n) return('A');          /* Must have right packet number */
            Spack ('Y',n,0,0);                   /* OK, ACK it. */
            Bflush ();
            _dclose(ftcb.fh);                   /* Close the file */
            ftcb.fh = NULL;     
             n = (n+1)%64;                      /* Bump packet number */
            ftcb.blockno++;
            return('F');                        /* Go back to Receive File state */

        case FALSE:
            Spack ('N',num,0,0);
            return(state);                      /* No packet came, keep waiting */
        default:
            return('A');               /* Some other packet, "abort" */
      }
    return (0);
  }

/*
 *      c o n n e c t
 *
 *      Establish a virtual terminal connection with the remote host.
 *
 */

void Connect ()
{
}

/*
 *      KERMIT utilities.
 */

/* tochar converts a control character to a printable one by adding a space */

char ToChar (ch)
char ch;
  {
    return((char)(ch + ' '));     /* make sure not a control char */
  }


/* unchar undoes tochar */

char UnChar (ch)
char ch;
  {
    return((char)(ch - ' '));     /* restore char */
  }


/*
 * ctl turns a control character into a printable character by toggling the
 * control bit (ie. ^A becomes A and A becomes ^A).
 */

char Ctl (ch)
char ch;
  {
    return((char)(ch ^ 64));       /* toggle the control bit */
  }


/*
 *      s p a c k
 *
 *      Send a Packet
 */

void Spack (type,num,len,data)
char type, *data;
int num, len;
  {
    int i;                                /* Character loop counter */
    unsigned char chksum, buffer[100];    /* Checksum, packet buffer */
    register unsigned char *bufp;         /* Buffer pointer */
  
    if (debug) printf( "spack\n");
    bufp = buffer;                        /* Set up buffer pointer */
    for (i=1; i<=pad; i++)
        WriteSer (&padchar,1);             /* Issue any padding */

    *bufp++ = SOH;                        /* Packet marker, ASCII 1 (SOH) */
    chksum  = ToChar ((char) (len+3));     /* Initialize the checksum */
    *bufp++ = ToChar ((char) (len+3));     /* Send the character count */
    chksum += ToChar ((char) num);         /* Init checksum */
    *bufp++ = ToChar ((char) num);         /* Packet number */
    chksum += type;                       /* Accumulate checksum */
    *bufp++ = type;                       /* Packet type */

    for (i=0; i<len; i++)                 /* Loop for all data characters */
      {
        *bufp++ = data[i];                /* Get a character */
        chksum = chksum+data[i];          /* Accumulate checksum */
      }
    chksum = (chksum + (chksum & 192) / 64) & 63; /* Compute final checksum */
    *bufp++ = ToChar (chksum);             /* Put it in the packet */
    *bufp = eol;                          /* Extra-packet line terminator */

    WriteSer (buffer,(int)(bufp-buffer+1));/* Send the packet */

    if (debug) putchar((int)'.');
  }

/*
 *  r p a c k
 *
 *  Read a Packet
 *
 */

int Rpack (len,num,data)
int *len, *num;                         /* Packet length, number */
char *data;                             /* Packet data */
  {
    int i, done;                        /* Data character number, loop exit */
    char chksum, t, type;               /* Checksum, current char, pkt type */
 
    if (debug) printf( "Rpack\n");
    t = '\0';

    while (t != SOH)                    /* Wait for packet header */
      {

        if ( !ReadSer(&t,1) )
            return (FALSE);

        if (!image || bin7bit)
            t &= 0x7f;                  /* Handle parity */
      }

    done = FALSE;                       /* Got SOH, init loop */
    while (!done)                       /* Loop to get a packet */
      {

        if ( !ReadSer(&t,1) )           /* Get character */
        return (FALSE);

        if (!image || bin7bit )
            t &= 0x7f;                  /* Handle parity */
        if (t == SOH) continue;         /* Resynchronize if SOH */

        chksum = t;                     /* Start the checksum */
        *len = UnChar (t)-3;             /* Character count */

        if ( !ReadSer(&t,1) )           /* Get character */
            return (FALSE);

        if (!image || bin7bit )
            t &= 0x7f;                  /* Handle parity */
        if (t == SOH) continue;         /* Resynchronize if SOH */
        chksum = chksum + t;            /* Accumulate checksum */
        *num = UnChar (t);               /* Packet number */

        if ( !ReadSer(&t,1) )           /* Get character */
            return (FALSE);

        if (!image || bin7bit )
            t &= 0x7f;                  /* Handle parity */
        if (t == SOH) continue;         /* Resynchronize if SOH */
        chksum = chksum + t;            /* Accumulate checksum */
        type = t;                       /* Packet type */

        for (i=0; i<*len; i++)          /* The data itself, if any */
          {                             /* Loop for character count */
            if ( !ReadSer(&t,1) )       /* Get character */
                return (FALSE);

            if (!image || bin7bit )
                t &= 0x7f;              /* Handle parity */
            if (t == SOH)
                continue;               /* Resynch if SOH */
            chksum = chksum + t;        /* Accumulate checksum */
            data[i] = t;                /* Put it in the data buffer */
          }
        data[*len] = 0;                 /* Mark the end of the data */

        if ( !ReadSer(&t,1) )           /* Get character */
            return (FALSE);

        if (!image || bin7bit )
            t &= 0x7f;                  /* Handle parity */
        if (t == SOH) continue;         /* Resynchronize if SOH */
        done = TRUE;                    /* Got checksum, done */
      }

    chksum = (chksum + (chksum & 192)
              / 64) & 63;               /* Fold bits 7,8 into chksum */
              
    if (chksum != UnChar (t))
              return(FALSE);            /* Check the checksum, fail if bad */

    if ( debug ) putchar( '\n' );
    return((int)(type));                /* All OK, return packet type */
  }



/*
 *      b u f i l l
 *
 *      Get a bufferful of data from the file that's being sent.
 *      Repeat count prefixes are not handled.
 */

int Bufill (buffer)
char buffer[];                          /* Buffer */
  {
    int i;                              /* Loop index */
    char t,t7;
  
  
    if (debug) printf( "bufil\n");
    i = 0;                              /* Init data buffer pointer */
    while(_dread(ftcb.fh,&t,1) > 0)     /* Get the next character */
      {
        Byte_count++;
        if (image && !bin7bit )         /* In 8-bit mode? */
          {
            t7 = t & 0x7f;              /* Yes, look at low-order 7 bits */
            if (t7 <  SP  ||
                t7 == DEL ||
                t7 == quote )           /* Control character? */
              {                         /*    Yes, */
                buffer[i++] = quote;    /*    quote this character */
                if (t7 != quote) t = Ctl (t); /* and uncontrollify */
              }
          }
        
        else if ( image && bin7bit )    /* In 7-bit binary ? */
          {
            if ( t & 0x80 )             /* 8 bit character ? */
              {
                buffer[i++] = qbin;     /* Yes, binary quote it */
                t = t & 0x7f;
              }
            t7 = t & 0x7f;              /* Yes, look at low-order 7 bits */
            if (t7 <  SP    ||
                t7 == DEL   ||
                t7 == quote ||
                t7 == qbin)             /* Control character? */
              {                         /*    Yes, */
                buffer[i++] = quote;    /*    quote this character */
                if (t7 != quote && 
                    t7 != qbin )
                    t = Ctl (t);         /* and uncontrollify */
              }
          }
        
        else                            /* Else, ASCII text mode */
          {
            t &= 0x7f;                  /* Strip off the parity bit */
            if (t < SP || t == DEL || t == quote) /* Control character? */
              {                         /* Yes */
                if (t=='\n')            /* If newline, squeeze CR in first */
                  {
                    buffer[i++] = quote;
                    buffer[i++] = Ctl ('\r');
                  }
                buffer[i++] = quote;    /* Insert quote */
                if (t != quote) t=Ctl(t);/* Uncontrollified the character */
              }
          }
        
        buffer[i++] = t;                /* Deposit the character itself */
        if (i >= spsiz - 8)
            return(i);                  /* Check length */
      }
    
    if (i == 0)
        return(EOF);                    /* Wind up here only on EOF */
   
    return(i);                          /* Handle partial buffer */
  }

/*
 *      b u f e m p
 *
 *      Get data from an incoming packet into a file.
 */

void Bufemp (buffer,len)
char buffer[];                          /* Buffer */
int len;                                /* length */
  {
    int i;                              /* Counter */
    char t;                             /* Character holder */

    for (i=0; i<len; i++)               /* Loop thru the data field */
      {
        t = buffer[i];                  /* Get character */
        if ( t == MYQBIN &&
             bin7bit     &&
             image )
          {
            t = buffer[++i] & 0x7f;     /* Get binary quoted character */
            if ( t == MYQUOTE )         /* Is it a QUOTE character? */
              {
                t = buffer[++i] & 0x7f; /* Get the next character. */
                if ( t != MYQUOTE &&
                     t != MYQBIN )      /* Is it a quote character ? */
                    t = Ctl (t);         /* No, convert to control character. */
                t |= 0x80;              /* Turn on high bit. */
              }
            else if ( t  != MYQBIN )    /* Is it binary quote character ? */
                t |= 0x80;              /* No, OR in the high bit. */
          }
        
        else if (t == MYQUOTE)          /* Control quote? */
          {                             /* Yes */
            t = buffer[++i];            /* Get the quoted character */
            if ( (t & 0x7f) != MYQUOTE &&
                 t != MYQBIN )          /* Low order bits match quote char? */
                 t  = Ctl (t);           /* No, uncontrollify it */
          }
        
        if (image || (t != CR))         /* Don't pass CR in text mode */
          {
            Bwrite (&t,1);               /* Put the char in the file */
            Byte_count++;
          }
      }
  }


/*
 *      g e t f i l
 *
 *      Open a new file
 */

int GetFil ()
  {
    static unsigned char file_name [ 129 ];
  
    strcpy ( file_name, nvmodes.dest_path );
    stcgfn ( file_name + strlen ( file_name ), packet );
    filnam = file_name;
    SetFile ( file_name );
    Byte_count = 0L;
    SetReceived ( Byte_count );
    Binit ();
    ftcb.fh = _dcreat(filnam,0);
    return (ftcb.fh > 0);               /* Return false if file won't open */
  }

/*
 *      g n x t f l
 *
 *      Get next file in a file group
 */

int GNxtFl ()
  {
    if (debug) printf( "GNxtFl\n");
    if (files[files_idx])
      {
        filnam = files[ files_idx++];
        SetFile ( filnam );
        Byte_count = 0L;
      }
    else
        return ( FALSE );                /* If no more, fail */
    return ( TRUE );
  }

void ShortName ( name )
unsigned char *name;
  {
    static unsigned char file_name[ 65 ];
  
    stcgfn ( file_name, name);
    filnam = file_name;
  }

/*
 *      s p a r
 *
 *      Fill the data array with my send-init parameters
 *
 */

void Spar (data)
char data[];
  {
    data[0] = ToChar (MAXPACK);            /* Biggest packet I can receive */
    data[1] = ToChar (MYTIME);             /* When I want to be timed out */
    data[2] = ToChar (MYPAD);              /* How much padding I need */
    data[3] = Ctl (MYPCHAR);               /* Padding character I want */
    data[4] = ToChar (MYEOL);              /* End-Of-Line character I want */
    data[5] = MYQUOTE;                    /* Control-Quote character I send */
    data[6] = MYQBIN;                     /* Binary-Quote character I send */
  }


/*      r p a r
 *
 *      Get the other host's send-init parameters
 *
 */

void Rpar (data)
char data[];
  {
    spsiz = UnChar (data[0]);              /* Maximum send packet size */
    timint = UnChar (data[1]);             /* When I should time out */
    pad = UnChar (data[2]);                /* Number of pads to send */
    padchar = Ctl (data[3]);               /* Padding character to send */
    eol = UnChar (data[4]);                /* EOL character I must send */
    quote = data[5];                      /* Incoming data quote character */
    qbin  = data[6];                      /* Incoming binary quote charater */
    if (debug )
        printf ( "spsiz = %d, timint = %d, pad = %d, padchar = %d, eol = %d, quote = %d\n",
                spsiz, timint, pad, 
                padchar, eol, quote );   
  }


/*      R e a d S e r
 *
 *      Read characters from the serial.device
 *
 */

int ReadSer (data,length)
char *data;
int length;
  {

    unsigned short int  i;
    unsigned long int   mask,
                        io_mask,
                        tr_mask,
                        signals;

    tr_mask = 1L << timeout_req.tr_node.io_Message.mn_ReplyPort->mp_SigBit;
    io_mask = 1L <<       ser_req.IOSer.io_Message.mn_ReplyPort->mp_SigBit;
    mask    = tr_mask | io_mask;

    StartRead ( data, length );
    for ( i = 0; i < timint; i++ )
      {
        StartTimer ( &timeout_req, 1 );

        signals = Wait ( mask );

        if ( signals & io_mask )
          {
            GetMsg  ( ser_port );
            if ( !( signals & tr_mask ) )
              {
                AbortIO ( (struct IORequest *) &timeout_req );
                Wait    ( tr_mask );
              }
            GetMsg  ( timeout_port );
            if ( CheckForWindowAbort () )
              {
                transfer_aborted = 1;
                return ( 0 );
              }
            if ( debug ) DisplayPacket ( data, (unsigned short int) length );
            return ( 1 );
          }
        else
          {
            GetMsg ( timeout_port );
            if ( CheckForWindowAbort () )
              {
                transfer_aborted = 1;
                goto abort_serial_read;
              }
          }
      }
      
    SetMessage ( "Timeout waiting for packet" );
    ftcb.total_errors++;

abort_serial_read:;

    if ( !( signals & io_mask ) )
      {
        AbortIO ( (struct IORequest *) &ser_req );
        Wait    ( io_mask  );
      }
    GetMsg  ( ser_port );
    return  ( 0 );
  }

/*      W r i t e S e r
 *
 *      Write characters to the serial.device
 *
 */

int WriteSer (data,length)
char *data;
int length;
  {
    unsigned long int   signals,
                        timeoutmask,
                        serialmask;

    unsigned char       sent_ok;

    timeoutmask  = 1L << timeout_req.tr_node.io_Message.mn_ReplyPort->mp_SigBit;
    serialmask   = 1L << ser_req.IOSer.io_Message.mn_ReplyPort->mp_SigBit;

    ser_req.IOSer.io_Command = CMD_WRITE;
    ser_req.IOSer.io_Data    = (APTR) data;
    ser_req.IOSer.io_Flags  &= ~IOF_QUICK;
    ser_req.IOSer.io_Length  = length;
    
    if ( debug )
      {
        DisplayPacket (data,(unsigned short int) length );
        putchar ('\n');
      }
      
    SendIO ( (struct IORequest *) &ser_req );

    sent_ok = 0;
    do
      {
        StartTimer ( &timeout_req, MYTIME );

        signals = Wait ( timeoutmask | serialmask );

        if ( signals & serialmask )
         {
            AbortIO ( (struct IORequest *) &timeout_req );
            Wait    ( timeoutmask );
            GetMsg  ( timeout_port );
            WaitIO ( (struct IORequest *) &ser_req );
            sent_ok = 1;
            if ( CheckForWindowAbort () )
              {
                transfer_aborted = 1;
                return ( 0 );
              }
          }
        else if ( signals & timeoutmask )
          {
            WaitIO ( (struct IORequest *) &timeout_req );
            SetMessage ( "Timeout sending packet" );
            ftcb.total_errors++;
            if ( CheckForWindowAbort () )
              {
                transfer_aborted = 1;
                AbortIO ( (struct IORequest *) &ser_req );
                Wait ( serialmask );
                GetMsg ( ser_port );
                return ( 0 );
              }
            HandleLockedLine ( &temp_ser_req );
          }
      } while ( !sent_ok );
    return ( 1 );
  }

void SetKermitParameters ()
  {
    if ( nvmodes.direction ==  INCOMING &&
         ftcb.protocol     ==  XON_XOFF &&
         ftcb.trans_act                 &&
         ftcb.fh )
      {
        Bflush ();
        ftcb.sv_fh       = ftcb.fh;
      }
    else
        ftcb.sv_fh       = NULL;
  }

void RestoreKermitParameters ()
  {
    if ( ftcb.sv_fh )
      {
        ftcb.fh           = ftcb.sv_fh;
        ftcb.trans_act    = 1;
        ftcb.protocol     = XON_XOFF;
        ftcb.size         = BLK_SZ;
        nvmodes.direction = INCOMING;
        ftcb.sv_fh        = NULL;
      }
  }

void DisplayPacket ( data, length )
unsigned char *data;
unsigned short int length;
  {
    unsigned short int i;
    
    for ( i = 0; i < length; i++ )
      {
        if ( data[i] < '!' || data[i] > '~' )
           printf ( "x'%02x'", data[i] );
       else
           putchar ( data[i] );
      }
  }
