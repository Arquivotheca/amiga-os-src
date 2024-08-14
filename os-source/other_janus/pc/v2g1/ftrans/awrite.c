/************************************************************************
 *
 *   AWRITE.C   -   Program to send a file to the Amiga from the PC.
 *
 * /b option  -    Overrides conversion of CR_LF.
 *
 * /nc option -    Overrides character translation.
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
   char  buf[1024];

   if (argc<3)         
   { 
      puts("Usage: AWRITE PCfilename   Amigafilename  [/B] [/NC] [/CR]");
      puts("       AWRITE PC-wild-card AmigaDirectory [/B] [/NC] [/CR]");
         exit(10);
   }

   mode     = JFT_CR_LF;                  /* Default is text mode       */
   convert  = JFT_CONVERT;

   if (argc > 3)                          /* Possibly /B option given   */
   { 
      for(x=argc-1;;x--)
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
                  break;
          argc--;
      }
   }
  
   if(argc==3)    /* single file transfer so 2nd arg is dest filename   */
   {
      error=j_file_transfer(argv[1],argv[2],JFT_PC_AMIGA,mode,NULL,convert);
   }
   else
   {
      x=strlen(argv[argc-1]);
      if(argv[argc-1][x-1] != ':' && argv[argc-1][x-1] != '/')
      {
         printf("Output filespec must be a directory!");
         exit(10);
      }
      for(x=1;x<argc-1;x++)
      {
         buf[0]='\000';
         strcat(buf,argv[argc-1]);
         strcat(buf,argv[x]);
         printf("Writing %s...",argv[x]);
         error=j_file_transfer(argv[x],buf,JFT_PC_AMIGA,mode,NULL,convert);
         if(error) break;
         puts("copied");
      }
   }
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