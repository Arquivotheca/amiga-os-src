/************************************************************************
 *
 * AREAD.C     -  Program to read a file from the Amiga into a PC file.
 *
 * /b  option  -  Overrides conversion of CR_LF.
 *
 * /nc option  -  Overrides character translation.
 *
 * Written for MICROSOFT-C 5.1 01-25-88 by Bill Koester (CATS) 
 * 9-19-88 - Added error return codes for script file support.
 * Copyright 1988 Commodore International Ltd.
 *
 ************************************************************************/
                                                                                
#define LINT_ARGS                         /* Enable type checking       */
#include <stdio.h>
#include <janus/jftrans.h>
 
                                               
main(argc, argv)
int argc;
char *argv[];
{
   int   mode;
   int   error;
   int   convert;
   int   x;
 
   if (argc!=3 && argc!=4 && argc!=5)         
   { 
      puts("Usage: AREAD Amigafilename PCfilename [/B] [/NC] [/CR]");
         exit(10);
   }

   mode     = JFT_CR_LF;                  /* Default is text mode       */
   convert  = JFT_CONVERT;

   if (argc > 3)                          /* Possibly /B option given   */
   { 
      for(x=3;x<argc;x++)
      {
         if (strcmpi(argv[x],"/cr")==0)
            mode=JFT_BINARY;
         else 
            if(strcmpi(argv[x],"/nc")==0)
               convert=JFT_NO_CONVERT;
            else 
					if(strcmpi(argv[x],"/b")==0)
					{
                  mode=JFT_BINARY;
                  convert=JFT_NO_CONVERT;
					} else
                  {
                     puts("Invalid option, [/B] [/NC] [/CR] expected");
                     exit(10);
                  }
      }
   }
  
   error=j_file_transfer(argv[1],argv[2],JFT_AMIGA_PC,mode,NULL,convert);

   if(error)
      switch(error)
      {
         case  JFT_ERR_INVALID_MODE:
               puts("Error: invalid transfer mode.");
               break;
         case  JFT_ERR_INVALID_DIRECTION:
               puts("Error: invalid direction specified.");
               break;
         case  JFT_ERR_NO_SERVER:
               puts("Error: server not running. Run PCDisk on Amiga");
               break;
         case  JFT_ERR_PC_OPEN:
               puts("Error opening PC file.");
               break;
         case  JFT_ERR_AMIGA_OPEN:
               puts("Error opening Amiga file.");
               break;
         case  JFT_ERR_AMIGA_READ:
               puts("Amiga read error.");
               break;
         case  JFT_ERR_AMIGA_WRITE:
               puts("Amiga write error.");
               break;
         case  JFT_ERR_INVALID_CONVERSION_MODE:
               puts("Invalid conversion mode specified.");
               break;
         default:
               puts("Unknown error.");
               break;
      }

	return(error);
}                                         /* End main                   */
