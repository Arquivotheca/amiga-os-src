/*
lc -d -j73 -o/obj/Eval.o -i/include -v -csf Eval
blink /obj/Eval.o to /bin/Eval sc sd nd
protect /bin/Eval +p
quit
*/
  
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* |_o_o|\\ Copyright (c) 1989 The Software Distillery.  All Rights Reserved */
/* |. o.| || This program may not be distributed without the permission of   */
/* | .  | || the authors:                             BBS: (919) 382-8265    */
/* | o  | ||   Dave Baker      Alan Beale         Jim Cooper                 */
/* |  . |//    Jay Denebeim    Bruce Drake        Gordon Keener              */
/* ======      John Mainwaring Andy Mercier       Jack Rouse                 */
/*             John Toebes     Mary Ellen Toebes  Doug Walker   Mike Witcher */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**	(C) Copyright 1989 Commodore-Amiga, Inc.
 **	    All Rights Reserved  
**/ 
  
/*------------------------------------------------------------------------*/
/* Command: Eval                                                          */
/* Author:  Mary Ellen Toebes                                             */
/* Change History:                                                        */
/*  Date    Person        Action                                          */
/* -------  ------------- -----------------                               */
/* 19MAR89  John Toebes   Initial Creation                                */
/* 30Nov89  Mary Ellen Toebes added % for mod function                    */
/* 13Apr90  Mary Ellen Toebes Changed to parse and evaluate L to R        */
/* 27dec90  Mary Ellen Toebes at JAT request support lformats of %N%N%N%N */
/*          usefull for printing out number in hex, octal, decimal, char  */
/* 03mar91  Mary Ellen Toebes Do not stop evaluating because of missing   */
/*          paren                                                         */
/* 03mar91  Mary Ellen Toebes Truncate string if invalid operator found   */
/*          But do not print warning and do print questionable result     */
/*------------------------------------------------------------------------*/

#include "internal/commands.h"
#include "eval_rev.h"

#define MSG_BADFILE "Cannot open output file %s\n"
#define MSG_BADPREN "Mismatched parenthesis\n"
#define MSG_FORMAT  "%n\n"
#define MSG_BADMEM  "Out of Memory\n"
#define MSG_BADLFORMAT "Invalid LFORMAT string\n"


#define TEMPLATE    "VALUE1/A,OP,VALUE2/M,TO/K,LFORMAT/K" CMDREV
#define OPT_VAL1    0
#define OPT_OP      1
#define OPT_VAL2    2
#define OPT_TO      3
#define OPT_FORM    4
#define OPT_COUNT   5

/* return codes from parse_num */
#define VALID_NUM     0x0000
#define WARN_UNMPAREN 0x0001

/* return codes from parse_op */
#define VALID_OP      0x0000
#define MISSING_OP    0x0010
#define END_OF_EXP    0x0100 

int  eval     ( char **, long *, struct DOSLibrary *, struct Library *);
int  parse_num( char **, long *, struct DosLibrary *, struct Library *);
int  parse_op ( char **, char *);

int cmd_eval(void)
{
   struct Library *SysBase = (*((struct Library **) 4));
   struct DosLibrary *DOSBase;
   struct Library    *UtilityBase;
   struct RDargs     *rdargs;
   long   opts[OPT_COUNT];
   int    rc;
   long   length = 0L;
   long   result = 0L;
   long   result_array[4];
   BPTR   fh;
   char   *error_msg;
   char   *expression = NULL;
   char   *from, *to, **token;

   rc  = RETURN_FAIL;
   error_msg = NULL;
   
   if ((DOSBase = (struct DosLibrary *)OpenLibrary(DOSLIB, DOSVER)) &&
       (UtilityBase = OpenLibrary("utility.library",0)))
      {
      memset((char *)opts, 0, sizeof(opts));

      rdargs = ReadArgs(TEMPLATE, opts, NULL);

      if (rdargs == NULL)
      {
         PrintFault(IoErr(), NULL);
      }
      else
      {
         /*--------------------------------------------------------------*/
         /* Open output file                                             */
         /*--------------------------------------------------------------*/
         if (opts[OPT_TO])
         {
            if (!(fh = Open((char *)opts[OPT_TO], MODE_NEWFILE)))
            {
               error_msg = MSG_BADFILE;
               goto ERROR;
            }
         }
         else
         {
            fh = Output();
         }
         rc = RETURN_OK;

         /*--------------------------------------------------------------*/
         /* Allocate memory to hold concatenated expression              */
         /*--------------------------------------------------------------*/
         from     = (char *)opts[OPT_VAL1]; length = strlen(from)+1;
         if (from = (char *)opts[OPT_OP])   length += strlen(from)+1;
         if (token = (char **)opts[OPT_VAL2])
            while(from = *token++)
               length += strlen(from)+1;

         if (!(expression = AllocMem(length,  0)))
         {
            error_msg = MSG_BADMEM;
            goto ERROR;
         }
         
         /*--------------------------------------------------------------*/
         /* Concatinate value1, op, value2 together and parse R to L     */
         /*--------------------------------------------------------------*/
         to = expression;
         from     = (char *)opts[OPT_VAL1]; while (*from) *to++ = *from++;
         if (from = (char *)opts[OPT_OP])   while (*from) *to++ = *from++;
         if (opts[OPT_VAL2])
         if (token = (char **)opts[OPT_VAL2])
            for(; *token; token++)
            {
               int len;
               len = strlen(*token);
               memcpy( to, (*token), len);
               to += len;
               *to++ = ' ';
            }
         *to = '\0';            /* sentinal at end of string */

         /*--------------------------------------------------------------*/
         /* Evaluate expression                                          */
         /*--------------------------------------------------------------*/
         to = expression;
         eval(&to, &result, DOSBase, UtilityBase);

         /*--------------------------------------------------------------*/
         /* Validate Lformat if one given                                */
         /*--------------------------------------------------------------*/
         if (opts[OPT_FORM])
         {
            int special = 0;
            for( from=(char *)opts[OPT_FORM]; *from; from++)
               if (*from == '%')
               {
                  switch(*++from)
                  {
                     case 'n': case 'N':
                     case 'o': case 'O':
                     case 'c': case 'C':
                     case 'x': case 'X':
                               if (special++ < 4) break;
                     default:  error_msg = MSG_BADLFORMAT;
                               goto ERROR;
                  }
               }
         }
         else 
            opts[OPT_FORM] = (long)MSG_FORMAT;
         
         /*--------------------------------------------------------------*/
         /* Print result - if no format specified use "%n\n"             */
         /*--------------------------------------------------------------*/
         result_array[0] = result_array[1] = 
         result_array[2] = result_array[3] = result;
         VFWritef(fh, (char *)opts[OPT_FORM], result_array);

         if (opts[OPT_TO])
            Close(fh);
ERROR:
         FreeArgs(rdargs);
      }

      if (error_msg)
         VPrintf( error_msg, &opts[OPT_TO]);

      if (expression)
         FreeMem(expression, length);
         
      CloseLibrary(UtilityBase);
      CloseLibrary((struct Library *)DOSBase);
   }
   else
   {
      /* We couldn't open DOS.library so give them an alert */
      OPENFAIL;
   }

return(rc);
}

/*---------------------------------------------------------------------*/
/* Name: Eval                                                          */
/* Pupose: Evaluate an expression                                      */
/* Returns: 1- mismatched parenthesis                                  */
/*          0- End of expression                                       */
/* Note: recursive function. This function call parse_num which may    */
/*       call eval.                                                    */
/*---------------------------------------------------------------------*/
int eval(str, result, DOSBase, UtilityBase)
char **str;
long *result;
struct DosLibrary *DOSBase;
struct Library *UtilityBase;
{
   int  rc;
   long rh;
   char op;

  /*--------------------------------------------------------------------*/
  /* Get left hand side of expression.                                  */
  /*--------------------------------------------------------------------*/
   rc = parse_num(str, result, DOSBase, UtilityBase);

   while(rc == VALID_NUM)
   {
     /*------------------------------------------------------------------*/
     /* Get Operator                                                     */
     /*------------------------------------------------------------------*/
      if (rc = parse_op(str, &op)) break;
      
     /*------------------------------------------------------------------*/
     /* Get right hand side of expression                                */
     /*------------------------------------------------------------------*/
      if (rc = parse_num(str, &rh, DOSBase, UtilityBase)) break;
            
     /*------------------------------------------------------------------*/
     /* Evaluate expression                                              */
     /*------------------------------------------------------------------*/
      switch (op)
      {
         case '*': *result = SMult32(*result, rh);             break;
         case '/': *result = (rh==0)? 0: SDiv32(*result, rh);  break;
         case '+': *result = *result + rh;                     break;
         case '-': *result = *result - rh;                     break;
         case '%':
         case 'M':
         case 'm': *result = (rh==0)? 0: SMod32(*result, rh);  break;
         case '&': *result =  *result & rh;                    break;
         case '|': *result =  *result | rh;                    break;
         case '~': *result = ~rh;                              break;
         case 'r':
         case 'R': *result =  *result >> rh;                   break;
         case 'l':
         case 'L': *result =  *result << rh;                   break;
         case 'X':
         case 'x': *result =  (*result ^ rh);                  break;
         case 'E':
         case 'e': *result = ~(*result ^ rh);                  break;
         default : rc = END_OF_EXP;                            break;
      }
   }

   return(rc);
}

/*---------------------------------------------------------------------*/
/* Name: Parse_num                                                     */
/* Purpose: To parse a number of the form: 3, 0x3, 03, #x3, #3, 'a     */
/*---------------------------------------------------------------------*/

int parse_num(str, value, DOSBase, UtilityBase)
char **str;
long *value;
struct DosLibrary *DOSBase;
struct Library *UtilityBase;
{
   int  rc = VALID_NUM;   
   long v  = 0L;
   long nv;
   long base;
   long sign;
   char c;
   char *p = *str;

   /*--------------------------------------------------------------------*/
   /* Scan right to left skipping over blanks                            */
   /*--------------------------------------------------------------------*/
   while (*p == ' ') p++;

   /*--------------------------------------------------------------------*/
   /* Look for sign                                                      */
   /*--------------------------------------------------------------------*/
   if (*p == '-')
   {
      p++;
      sign = -1;
   }
   else if (*p == '+')
   {
      p++;
      sign = +1;
   }
   else
      sign = 0;
   
   /*--------------------------------------------------------------------*/
   /* Number is an expression call eval to evaluate it                   */
   /*--------------------------------------------------------------------*/
   if (*p == '(')
   {
      p++;                                    
      if (eval(&p, &v, DOSBase, UtilityBase) == WARN_UNMPAREN) 
         p++;
      else
         VPrintf( MSG_BADPREN, NULL );
   }

   /*--------------------------------------------------------------*/
   /* Number is a character constant of the form 'c                */
   /*--------------------------------------------------------------*/
   else if (*p == '\'') 
   {
      v = *(p+1);
      p += 2;
   }

   /*--------------------------------------------------------------*/
   /* Number is a base 8,10, or 16 number                          */
   /*--------------------------------------------------------------*/
   else
   {
      /*--------------------------------------------------------------*/
      /* Determine Base- 0Xdd is base 16, 0dd is base 8, dd is base 10*/
      /*                 #Xdd is base 16, #dd is base 8, dd is base 10*/
      /*--------------------------------------------------------------*/
      if ( (*p == '#') || (*p == '0') )
      {
         p++;
         if ( (*p == 'x') || (*p == 'X') )
         {
            p++;
            base = 16L;
         }
         else
            base = 8L;
      }
      else
         base = 10L;

      /*--------------------------------------------------------------*/
      /* Scan right to left accumulating number in v.                 */
      /*--------------------------------------------------------------*/
      while (c = *p)
      {
         nv = SMult32(v, base);
         if      ((c>='0') && (c<='7'))               v=nv+(c - '0');                     
         else if ((c>='8') && (c<='9') && (base!= 8)) v=nv+(c - '0');
         else if ((c>='a') && (c<='f') && (base==16)) v=nv+(c - 'a')+10;
         else if ((c>='A') && (c<='F') && (base==16)) v=nv+(c - 'A')+10;
         else break;
         p++;
      }
   }

   if (sign)
      v = SMult32(v, sign);
   *value = v;
   *str = p;
   return(rc);
}



int parse_op(str, op)
char **str;
char *op;
{
   char *p = *str;
   int  rc = VALID_OP;

   /* strip trailing blanks */
   while (*p == ' ') p++;

   *op = *p;

   if (*op == '\0')       rc = END_OF_EXP;
   else if (*op == '(')   rc = MISSING_OP;
   else if (*op == ')')   rc = WARN_UNMPAREN;

   /* found an operator */
   else if ((*p     == 'M' || *p     == 'm') && 
            (*(p+1) == 'O' || *(p+1) == 'o') && 
            (*(p+2) == 'D' || *(p+2) == 'd'))
            p += 3;
   else if ((*p     == 'X' || *p     == 'x') && 
            (*(p+1) == 'O' || *(p+1) == 'o') && 
            (*(p+2) == 'R' || *(p+2) == 'r'))
            p += 3;
   else if ((*p     == 'E' || *p     == 'e') && 
            (*(p+1) == 'Q' || *(p+1) == 'q') && 
            (*(p+2) == 'V' || *(p+2) == 'v'))
            p += 3;
   else if ((*p     == 'R' || *p     == 'r') && 
            (*(p+1) == 'S' || *(p+1) == 's') && 
            (*(p+2) == 'H' || *(p+2) == 'h'))
            p += 3;
   else if ((*p     == 'L' || *p     == 'l') && 
            (*(p+1) == 'S' || *(p+1) == 's') && 
            (*(p+2) == 'H' || *(p+2) == 'h'))
            p += 3;
   else 
            p++;

   *str = p;
   return(rc);
}


