/*
 * This is a program to test the difference in time getting data from a
 * CD in both doublespeed and normal speed mode.
 *
 * The idea is to check the system time then get a certain amount of
 * data from the CD-ROM then check the system time. This will be done in
 * both normal speed and double speed mode.
 *
 * The planned parameters for the program will be:
 *
 * DataTime NORMAL/S DOUBLE/S SIZE/K/N ITERATIONS/K/N FILE/A/K
 *
 * The default will be:
 *
 * NORMAL SIZE= (of data in bytes which is to be decided.) ITERATIONS=1
 *
 * Written by Gregory P. Givler
 * Copyright © 06 May 1993 CISCO
 *
 *  To Do List:
 *
 *  [x]     ReadArgs Stuff
 *  [x]     Timer device Stuff
 *  [x]     CD Device Stuff
 *  [x]     CD Read Stuff
 *  [x]     Clean Up Routine
 *  [x]     Allocate memory for timeval structs
 *  [x]     Enable the Iteration Keyword stuff
 *  [x]     Get the default size of the read
 *  [x]     Write a function do the time math, or get Lockhart's stuff
 *  [x]     Write a funtion for the Memory CleanUp
 *  [x]     Add facility to use CD_READ
 *
 */


#include "datatime.h"

/*
 *
 * Clean Up Routine
 *
 */

void CleanUp(WORD ReturnCode)
{
    if (memoryptr) {
        FreeMem(memoryptr, readsize);
    }

    if (argsptr) {
        FreeArgs(argsptr);
    }

    if (start) {
        FreeVec(start);
    }

    if (stop) {
        FreeVec(stop);
    }

    if (CDOpen) {
        CloseDevice(IOStdReq);
    }

    if (IOStdReq) {
        DeleteStdIO(IOStdReq);
    }

    if (MsgPort) {
        DeletePort(MsgPort);
    }

    if (TimerIO) {
        if (!(CheckIO(TimerIO))) {
            AbortIO(TimerIO);
        }
/* DEBUG BEGIN:
        WaitIO(TimerIO);
END DEBUG */
        if (TimerOpen) {
            CloseDevice(TimerIO);
        }

        if (TimerIO) {
            DeleteExtIO(TimerIO);
        }

        if (TimerMP) {
            DeletePort(TimerMP);
        }
    }
    exit(ReturnCode);
}

/*
 *
 * Main Program
 *
 */

void main(int argc, char *argv[])
{
    BYTE error;
    LONG Opts[OPT_COUNT], actualread, totits, k;
    ULONG realTics;
    BPTR file;
    STRPTR elapsedtime, totelapsedtime;
    UBYTE *optspeed=NULL;
    unsigned char **endofstring=NULL;
    float realelapsedtime=0, newelapsedtime=0;
    BOOL closed;
    struct TagItem NormalList[] = {
        TAGCD_READSPEED, 75,
        TAG_END, 0
    };
    struct TagItem DoubleList[] = {
        TAGCD_READSPEED, 150,
        TAG_END, 0
    };
    float sumElapsed=0, minElapsed=1000*1000*1000, maxElapsed=0;

    /* Read Args stuff goes here */
    /* Grab memory for ReadArgs */
    memset(Opts, 0, sizeof(Opts));

    /* Now to check the command line */
    argsptr = ReadArgs(TEMPLATE, Opts, NULL);
    if (argsptr == NULL) {
        PrintFault(IoErr(),"ReadArgs");
        CleanUp(RETURN_FAIL);
    }


    /* Create a Standard Message Port so that I can talk to cd.device */
    MsgPort=CreatePort(NULL,0L);
    /* If I fail exit the program */
    if (!MsgPort) {
        printf("Couldn't create message port\n");
        CleanUp(RETURN_FAIL);
    }

    /* Create the Standard IO Request */
    IOStdReq=CreateStdIO(MsgPort);
    /* If I fail exit the program */
    if (!IOStdReq) {
        printf("Couldn't create standard IO request\n");
        CleanUp(RETURN_FAIL);
    }

    /* Open cd.device */

    error=OpenDevice("cd.device", CD_UNIT, IOStdReq, CD_FLAGS);
    CDOpen=error?FALSE:TRUE;

    /* Okay first thing to do is setup timer.device. */
    /* First create the Timer message port */
    TimerMP=CreatePort(NULL,0L);
    if (!TimerMP) {
        printf("Can't create Timer device message port\n");
        CleanUp(RETURN_FAIL);
    }

    /* Then create the TimeRequest */
    TimerIO=(struct timerequest *)CreateExtIO(TimerMP, sizeof(struct timerequest));
    if (!TimerIO) {
        printf("Unable to create Timer IO Request\n");
        CleanUp(RETURN_FAIL);
    }

    /* Now to Open the Device */
    error = OpenDevice("timer.device",UNIT_ECLOCK,TimerIO,0);
    if (error) {
        printf("Error: Can't Open timer.device. Error = %d\n",error);
        CleanUp(RETURN_FAIL);
    }

    TimerOpen = TRUE;

    TimerBase = (struct Library *)TimerIO->tr_node.io_Device;


    /* Set CD-ROM drive to Normal or Double Speed depending on the command line */
    if (Opts[OPT_NORMAL] != NULL) {
        /* Set the drive to Normal Speed */
        if (CDOpen) {
            optspeed = "NORMAL";
            IOStdReq->io_Command = CD_CONFIG;
            IOStdReq->io_Data = (APTR)&NormalList;
            IOStdReq->io_Length = 0;
            DoIO(IOStdReq);
            if (IOStdReq->io_Error) {
                printf("Could not Configure drive to Normal Speed\n");
                CleanUp(RETURN_FAIL);
            }
        } else {
            printf("cd.device not found -- ignoring NORMAL switch\n");
        }
    }

    if (Opts[OPT_DOUBLE] != NULL) {
        /* Set the drive to Double Speed */
        if (CDOpen) {
            optspeed = "DOUBLE";
            IOStdReq->io_Command = CD_CONFIG;
            IOStdReq->io_Data = (APTR)&DoubleList;
            IOStdReq->io_Length = 0;
            DoIO(IOStdReq);
            if (IOStdReq->io_Error) {
                printf("Could not Configure drive to Double Speed\n");
                CleanUp(RETURN_FAIL);
            }
        } else {
            printf("cd.device not found -- ignoring DOUBLE switch\n");
        }
    }

    /* If neither the NORMAL or DOUBLE switch is set then default to NORMAL */
    if (Opts[OPT_NORMAL] == NULL && Opts[OPT_DOUBLE] == NULL) {
        if (CDOpen) {
            optspeed = "NORMAL";
            IOStdReq->io_Command = CD_CONFIG;
            IOStdReq->io_Data = (APTR)&NormalList;
            IOStdReq->io_Length = 0;
            DoIO(IOStdReq);
            if (IOStdReq->io_Error) {
                printf("Could not Configure drive to Normal Speed\n");
                CleanUp(RETURN_FAIL);
            }
        } else {
            printf("cd.device not found -- ignoring NORMAL switch\n");
        }
    }
    /* Get the number of iterations by checking the command line */
    if (Opts[OPT_ITERATIONS] != NULL) {
        totits = *(LONG *)Opts[OPT_ITERATIONS];
    }
    else {
        totits = 1;
    }

    /* Allocate start and stop values */
    start=AllocVec(sizeof(struct EClockVal)*totits,NULL);
    if (!start) {
        printf("Error allocating start value array\n");
        CleanUp(RETURN_FAIL);
    }
    stop=AllocVec(sizeof(struct EClockVal)*totits,NULL);
    if (!stop) {
        printf("Error allocating stop value array\n");
        CleanUp(RETURN_FAIL);
    }

    /* Get the size of the Read */
    if (Opts[OPT_SIZE] != NULL ) {
        /* Get the size of the read to OPT_SIZE */
        readsize = *(LONG *)Opts[OPT_SIZE];
    }
    else {
        readsize = DEFAULT_SIZE;
    }

    /* Alloc the memory for the buffer for reading the file */
    memoryptr = AllocMem(readsize,MEMF_PUBLIC | MEMF_CLEAR);
    if (!memoryptr) {
        printf("Could not allocate memory for read buffer %d\n");
        CleanUp(RETURN_FAIL);
    }

    /* Now that timer.device is open I want to get the EClock Time */
    for(k=0;k < totits;k++) {

        /* first open the file */
        if (Opts[OPT_FILE] != NULL) {
            /* This should open the file */
            /* I will use the AmigaDOS call as it will save space and is easier to use */
            file = Open((STRPTR)Opts[OPT_FILE], MODE_OLDFILE);
            if (!file) {
                PrintFault(IoErr(),"Open");
                printf("Could not open File: %s\n",(STRPTR)Opts[OPT_FILE]);
                CleanUp(RETURN_FAIL);
            }
        }
        else {
            /* This will use the CD_READ function of cd.device this should eliminate the overhead of DOS */
            if (Opts[OPT_CDREAD] != NULL && Opts[OPT_OFFSET] != NULL) {
                IOStdReq->io_Command = CD_READ;
                IOStdReq->io_Data = memoryptr;
                IOStdReq->io_Length = readsize;
                IOStdReq->io_Offset = *(LONG *)Opts[OPT_OFFSET];
            }
            else {
                printf("Offset not provided\n");
                CleanUp(RETURN_FAIL);
            }
        }


        /* Read the EClock */
        if (k == 0) {
            /* Dump the Command line to stdout. This is so when we do testing we will have a record
             * of the command line
             */
            if (optspeed == NULL) {
                optspeed = "NOT SET";
            }
            if (Opts[OPT_FILE] != NULL) {
                printf("\nCOMMAND LINE: DataTime FILE = %s, ITERATIONS = %d, SPEED = %s, SIZE = %d\n\n", (STRPTR)Opts[OPT_FILE], totits, optspeed, readsize);
            }
            else {
                printf("\nCOMMAND LINE: DataTime ITERATIONS = %d, SPEED = %s, SIZE = %d, OFFSET = %d\n\n", totits, optspeed, readsize, *(LONG *)Opts[OPT_OFFSET]);
            }
            realTics = ReadEClock(&start[k]);
        }
        else {
            ReadEClock(&start[k]);
        }

        /* Now read the stuff */
        if (Opts[OPT_FILE] != NULL) {
            actualread = Read(file, memoryptr, readsize);
        }
        else {
            DoIO(IOStdReq);
            if (IOStdReq->io_Error) {
                printf("CD_READ failed io_Error = %d\n",IOStdReq->io_Error);
                CleanUp(RETURN_FAIL);
            }
            else {
                actualread = IOStdReq->io_Actual;
            }
        }

        /* Read the EClock again */
        ReadEClock(&stop[k]);

        /* Now to test the read */
        if (actualread != readsize) {
            printf("Read sizes are not equal.\n\treadsize = %lu bytes actually read = %ld\n", readsize, actualread);
        }

/* These printfs were for debugging. I am leaving them here but commented out in case they are needed
 *  in future revs
 */
        /* Now to print the values for the EClock */
/*        printf("EClock Start Value = %10lu Seconds, %10lu MicroSeconds\n",start[k].ev_hi, start[k].ev_lo); */
/*        printf("EClock Stop  Value = %10lu Seconds, %10lu MicroSeconds\n",stop[k].ev_hi, stop[k].ev_lo); */

        /* Now find out the elapsed time for the read */
        elapsedtime = timeCmp(&stop[k], &start[k], realTics);
        if (!elapsedtime) {
            printf("timeCmp() failed\n");
            CleanUp(RETURN_FAIL);
        }
        /* Now print the value returned by timeCmp() */
        printf("Elapsed Time for Iteration %d = %s seconds\n", k, elapsedtime);
        /* Change the string to a float */
        newelapsedtime = strtod(elapsedtime,endofstring);

        /* Now to add the elapsed time */
        realelapsedtime = realelapsedtime + newelapsedtime;

        /* Update statistics */
        sumElapsed+=newelapsedtime;
        minElapsed=min(minElapsed,newelapsedtime);
        maxElapsed=max(maxElapsed,newelapsedtime);

        /* Now free the pointer */
        FreeVec(elapsedtime);

        /* Close the File */
        if (Opts[OPT_FILE] != NULL) {
            closed = Close(file);
            if (!closed) {
                printf("File %s was not closed by Close()\n",(STRPTR)file);
                CleanUp(RETURN_FAIL);
            }
        }
    }

    /* Get the total time to do all the reads with overhead of the rest of the program */
    /* totits - 1 equals the last filled structure */
    totelapsedtime = timeCmp(&stop[totits-1], &start[0], realTics);
    if (!totelapsedtime) {
        printf("timeCmp() failed\n");
        CleanUp(RETURN_FAIL);
    }

    /* Print the total Elapsed time with overhead */
    printf("\nThe total Elapsed time with overhead = %s\n", totelapsedtime);
    printf("The total Elapsed time without overhead = %6.10lf\n\n", realelapsedtime);

    /* Output summary statistics */
    printf("\nMinimum elapsed time = %6.10lf seconds\n",minElapsed);
    printf("Maximum elapsed time = %6.10lf seconds\n",maxElapsed);
    printf("Average elapsed time = %6.10lf seconds\n",sumElapsed/totits);

    /* free the pointer */
    FreeVec(totelapsedtime);

    /* Program is done cleanup and exit */
    CleanUp(RETURN_OK);
}
