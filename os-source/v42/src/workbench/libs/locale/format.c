
#include <exec/types.h>
#include <dos/dos.h>
#include <utility/hooks.h>
#include <utility/date.h>
#include <string.h>

#include <clib/utility_protos.h>
#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/locale_protos.h>

#include <pragmas/utility_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/locale_pragmas.h>

#include "localebase.h"
#include "format.h"
#include "driverface.h"
#include "numconvert.h"


/*****************************************************************************/


VOID LongToStr(ULONG num, STRPTR buffer, WORD digits)
{
    buffer += digits;
    *buffer = '\0';
    while (--digits >= 0)
    {
        *(--buffer) = num % 10 + '0';
        num /= 10;
    }
}



#define PutCh(ch) (CallHookPkt(putCharFunc,locale,ch))

VOID ASM FormatDateLVO(REG(a0) struct ExtLocale *locale,
                       REG(a1) STRPTR template,
                       REG(a2) struct DateStamp *date,
                       REG(a3) struct Hook *putCharFunc,
                       REG(a6) struct LocaleLib *LocaleBase)
{
char             c;
char             str[100];
STRPTR 		 ptr;
struct ClockData cd;
UWORD		 num;
UWORD		 yday;

    if (template)
    {
        Amiga2Date(date->ds_Days*24*60*60+date->ds_Minute*60+date->ds_Tick/50,&cd);

        yday  = date->ds_Days;
        yday -= cd.year * 365;
        yday -= cd.year / 4;
        yday += cd.year / 100;
        yday -= cd.year / 400;

        while (c = *template++)
        {
            if (c != '%')
            {
                PutCh((APTR)c);
            }
            else
            {
                ptr = str;
                switch (*template++)
                {
                    case 'a': ptr = GetLocaleStr(locale,cd.wday+ABDAY_1);
                              break;

                    case 'A': ptr = GetLocaleStr(locale,cd.wday+DAY_1);
                              break;

                    case 'b':
                    case 'h': ptr = GetLocaleStr(locale,cd.month-1+ABMON_1);
                              break;

                    case 'B': ptr = GetLocaleStr(locale,cd.month-1+MON_1);
                              break;

                    case 'c': ptr = NULL;
                              FormatDateLVO(locale,"%a %b %d %H:%M:%S %Y",date,putCharFunc,LocaleBase);
                              break;

                    case 'C': ptr = NULL;
                              FormatDateLVO(locale,"%a %b %e %T %Z %Y",date,putCharFunc,LocaleBase);
                              break;

                    case 'd': LongToStr(cd.mday,str,2);
                              break;

                    case 'D': ptr = NULL;
                              FormatDateLVO(locale,"%m/%d/%y",date,putCharFunc,LocaleBase);
                              break;

                    case 'e': LongToStr(cd.mday,str,2);
                              if (cd.mday < 10)
                                  str[0] = ' ';
                              break;

                    case 'H': LongToStr(cd.hour,str,2);
                              break;

                    case 'I': if ((num = cd.hour % 12) == 0)
                                  num = 12;
                              LongToStr(num,str,2);
                              break;

                    case 'q': LongToStr(cd.hour,str,2);
                              if (cd.hour < 10)
                              {
                                  str[0] = str[1];
                                  str[1] = 0;
                              }
                              break;

                    case 'Q': if ((num = cd.hour % 12) == 0)
                                  num = 12;
                              LongToStr(num,str,2);
                              if (num < 10)
                              {
                                  str[0] = str[1];
                                  str[1] = 0;
                              }
                              break;

                    case 'j': LongToStr(yday+1,str,3);
                              break;

                    case 'm': LongToStr(cd.month,str,2);
                              break;

                    case 'M': LongToStr(cd.min,str,2);
                              break;

                    case 'n': ptr = "\n";
                              break;

                    case 'p': if (cd.hour >= 12)
                                  ptr = GetLocaleStr(locale,PM_STR);
                              else
                                  ptr = GetLocaleStr(locale,AM_STR);
                              break;

                    case 'r': FormatDateLVO(locale,"%I:%M:%S ",date,putCharFunc,LocaleBase);
                              if (cd.hour >= 12)
                                  ptr = "PM";
                              else
                                  ptr = "AM";
                              break;

                    case 'R': ptr = NULL;
                              FormatDateLVO(locale,"%H:%M",date,putCharFunc,LocaleBase);
                              break;

                    case 'S': LongToStr(cd.sec,str,2);
                              break;

                    case 't': ptr = "\t";
                              break;

                    case 'T': ptr = NULL;
                              FormatDateLVO(locale,"%H:%M:%S",date,putCharFunc,LocaleBase);
                              break;

                    case 'U': LongToStr((yday - cd.wday + 7)/7,str,2);
                              break;

                    case 'w': LongToStr(cd.wday,str,1);
                              break;

                    case 'W': if ((num = 8 - cd.wday) == 8)
                                  num = 1;
                              LongToStr((yday + num)/7,str,2);
                              break;

                    case 'x': ptr = NULL;
                              FormatDateLVO(locale,"%m/%d/%y",date,putCharFunc,LocaleBase);
                              break;

                    case 'X': ptr = NULL;
                              FormatDateLVO(locale,"%H:%M:%S",date,putCharFunc,LocaleBase);
                              break;

                    case 'y': LongToStr(cd.year-1900,str,2);
                              break;

                    case 'Y': LongToStr(cd.year,str,4);
                              break;

    /* time zone, not supported yet...
                    case 'Z': ptr = GetLocaleStr(locale,locale->loc_TimeZone+TZ_1);
                              break;
    */

                    case 0  : PutCh(NULL);
                              return;

                    default : str[0] = c;
                              str[1] = 0;
                              break;
                }

                if (ptr)
                    while (*ptr)
                        PutCh((APTR)*ptr++);
            }
        }
    }

    PutCh(NULL);
}


/*****************************************************************************/


#define GetCh() (CallHookPkt(getCharFunc,locale,NULL))
#define GetTCh() (template[tempCnt++])


BOOL ASM ParseDateLVO(REG(a0) struct ExtLocale *locale,
                      REG(a1) struct DateStamp *date,
                      REG(a2) STRPTR template,
                      REG(a3) struct Hook *getCharFunc,
                      REG(a6) struct LocaleLib *LocaleBase)
{
char             ch, tch;
UWORD            i;
char             token[120];
char             tempTok[120];
UWORD            tempCnt;
struct ClockData cd;
LONG             cnt;
LONG             value;
STRPTR           str;
UWORD            tokIndex;
ULONG            seconds;

    cd.year  = 1978;
    cd.month = 1;
    cd.mday  = 1;
    cd.hour  = 0;
    cd.min   = 0;
    cd.sec   = 0;

    tempCnt = 0;
    ch      = GetCh();
    tch     = ' ';
    while (ch)
    {
        while ((ch == ' ') || (ch == '\t'))
            ch = GetCh();

        i = 0;
        while (ch && (ch != ' ') && (ch != '\t') && (i < sizeof(token) - 1))
        {
            token[i++] = ch;
            ch = GetCh();
        }
        token[i] = 0;
        tokIndex = 0;

        while (token[tokIndex])
        {
            while ((tch == ' ') || (tch == '\t'))
                tch = GetTCh();

            while (tch == '%')
            {
                switch (tch = GetTCh())
                {
                    case 'a':
                    case 'A': for (i = DAY_1; i <= ABDAY_7; i++)
                              {
                                  str = GetLocaleStr(locale,i);
                                  if (Strnicmp(str,&token[tokIndex],strlen(str)) == 0)
                                  {
                                      tokIndex += strlen(str);
                                      break;
                                  }
                              }
                              break;

                    case 'h':
                    case 'b':
                    case 'B':
                    case 'm': for (i = MON_1; i <= ABMON_12; i++)
                              {
                                  str = GetLocaleStr(locale,i);
                                  if (Strnicmp(str,&token[tokIndex],strlen(str)) == 0)
                                  {
                                      cd.month = ((i - MON_1) % 12) + 1;
                                      tokIndex += strlen(str);
                                      break;
                                  }
                              }

                              if (i > ABMON_12)
                              {
                                  cnt = StrToLong(&token[tokIndex],&value);
                                  if ((cnt <= 0) || (value < 1) || (value > 12))
                                      return(FALSE);

                                  cd.month = value;
                                  tokIndex += cnt;
                              }
                              break;


                    case 'd':
                    case 'e': cnt = StrToLong(&token[tokIndex],&value);
                              if ((cnt <= 0) || (value < 1) || (value > 31))
                                  return(FALSE);

                              cd.mday = value;
                              tokIndex += cnt;
                              break;

                    case 'H':
                    case 'I': cnt = StrToLong(&token[tokIndex],&value);
                              if ((cnt <= 0) || (value < 0) || (value > 23))
                                  return(FALSE);

                              cd.hour = value;
                              tokIndex += cnt;
                              break;

                    case 'p': str = GetLocaleStr(locale,AM_STR);
                              if (Strnicmp(str,&token[tokIndex],strlen(str)) == 0)
                              {
                                  cd.hour = cd.hour % 12;
                              }
                              else
                              {
                                  str = GetLocaleStr(locale,PM_STR);
                                  if (Strnicmp(str,&token[tokIndex],strlen(str)) == 0)
                                      cd.hour = (cd.hour % 12) + 12;
                                  else
                                      return(FALSE);
                              }
                              break;

                    case 'M': cnt = StrToLong(&token[tokIndex],&value);
                              if ((cnt <= 0) || (value < 0) || (value > 59))
                                  return(FALSE);

                              cd.min = value;
                              tokIndex += cnt;
                              break;

                    case 'S': cnt = StrToLong(&token[tokIndex],&value);
                              if ((cnt <= 0) || (value < 0) || (value > 59))
                                  return(FALSE);

                              cd.sec = value;
                              tokIndex += cnt;
                              break;

                    case 'y': cnt = StrToLong(&token[tokIndex],&value);
                              if ((cnt <= 0) || (value > 99))
                                  return(FALSE);

                              if (value < 78)
                                  value += 100;

                              cd.year = value;

                              if (value < 1900)
                                  cd.year += 1900;

                              tokIndex += cnt;
                              break;

                    case 'Y': cnt = StrToLong(&token[tokIndex],&value);
                              if ((cnt <= 0) || (value < 1978) || (value > 9999))
                                  return(FALSE);

                              cd.year = value;
                              tokIndex += cnt;
                              break;

                    default : break;
                }

                if (tch != 0)
                    tch = GetTCh();
            }

            i = 0;
            while (tch && (tch != ' ') && (tch != '\t') && (tch != '%'))
            {
                tempTok[i++] = tch;
                tch = GetTCh();
            }
            tempTok[i] = 0;

            if (Strnicmp(tempTok,&token[tokIndex],i) != 0)
                return(FALSE);

            if (!tch && !ch)
            {
                seconds = CheckDate(&cd);
                date->ds_Days   = seconds / (60*60*24);
                date->ds_Minute = (seconds % (60*60*24)) / 60;
                date->ds_Tick   = (seconds % 60) * 50;
                return((BOOL)((seconds != 0)
                       || ((cd.year  == 1978)
                        && (cd.month == 1)
                        && (cd.mday == 1)
                        && (cd.hour == 0)
                        && (cd.min == 0)
                        && (cd.sec == 0))));
            }

            tokIndex += i;
        }
    }

    return(FALSE);
}


/*****************************************************************************/


APTR ASM FormatStringLVO(REG(a0) struct ExtLocale *locale,
                         REG(a1) STRPTR string,
                         REG(a2) UBYTE *dataStream,
                         REG(a3) struct Hook *putCharFunc,
                         REG(a6) struct LocaleLib *LocaleBase)
{
#define NUM_FMT_CMDS 128        /* WARNING! Anything larger will require
				 * the 'args' array to be declared as a UWORD
				 */

UWORD    i,j,k,l,arg,argNum;
UBYTE    args[NUM_FMT_CMDS];
STRPTR   str;
ULONG   *lptr;
UWORD   *sptr;
BOOLEAN leftJustify;
BOOLEAN zero;
BOOLEAN longValue;
BOOLEAN formatCmd;
BOOLEAN nextIsLimit;
BOOLEAN numeric;
char    padByte;
UWORD   width;
UWORD   limit;
ULONG   value;
UWORD   len;
UWORD   number;
char    buffer[15];
UWORD   start;
STRPTR  sep;
char    numFormatBuffer[200];

    if (!string)
        return(dataStream);

    i = NUM_FMT_CMDS;
    while (i--)
    {
        args[i] = NULL;
    }

    /* first loop determines the size of each argument */

    i   = 0;
    arg = 0;
    while (string[i])
    {
        if (string[i] == '%')
        {
            longValue = FALSE;
            argNum    = arg;
            number    = 0;
            formatCmd = TRUE;

            i++;
            while (formatCmd && string[i])
            {
                switch (string[i])
                {
                    case '0':
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                    case '8':
                    case '9': number = 0;
                              do
                              {
                                  number = number*2 + number*8 + (WORD)string[i] - (WORD)'0';
                                  i++;
                              } while ((string[i] >= '0') && (string[i] <= '9'));
                              i--;
                              break;

                    case '$': argNum = number-1;
                    case '-':
                    case '.': break;

                    case 'l': longValue = TRUE;
                              break;

                    case 's':
                    case 'b': longValue = TRUE;
                    case 'c':
                    case 'd':
                    case 'u':
                    case 'D':
                    case 'U':
                    case 'x':
		    case 'X': arg++;
                              if (argNum < NUM_FMT_CMDS)
                              {
                                  args[argNum] = 1;
                                  if (longValue)
                                  {
                                      args[argNum] = 2;
                                  }
                              }

                    default : formatCmd = FALSE;
                              break;
                }
                i++;
            }
            i--;
        }
        i++;
    }

    /* second loop converts the argument sizes into offsets within
     * 'dataStream' where to find each argument
     */

    i = 0;
    k = 0;
    while (i < NUM_FMT_CMDS)
    {
	j = args[i];
        args[i++] = k;
        k += j;
    }

    /* final loop where the actual output is done. This uses the
     * table of offsets generated in the two previous loops to
     * determine where in 'dataStream' to find the arguments it
     * needs
     */

    i   = 0;
    arg = 0;
    while (string[i])
    {
        if (string[i] == '%')
        {
            formatCmd   = TRUE;
            argNum      = arg;
            number      = 0;
            leftJustify = FALSE;
            width       = 0;
            limit       = 65535;
            longValue   = FALSE;
            nextIsLimit = FALSE;
            padByte     = ' ';
            start       = i;

            i++;
            while (formatCmd && string[i])
            {
                switch (string[i])
                {
                    case '0':
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                    case '8':
                    case '9': zero = (string[i] == '0');
                              number = 0;
                              do
                              {
                                  number = number*2 + number*8 + (WORD)string[i] - (WORD)'0';
                                  i++;
                              } while ((string[i] >= '0') && (string[i] <= '9'));

                              if (nextIsLimit)
                              {
                                  limit = number;
                              }
                              else if (string[i] != '$') /* a bit of lookahead never killed anyone... */
                              {
                                  width = number;
                                  if (zero)
                                      padByte = '0';
                              }
                              i--;
                              break;

                    case '$': argNum = number-1;
                              break;

                    case '-': leftJustify = TRUE;
                              break;

                    case '.': nextIsLimit = TRUE;
                              break;

                    case 'l': longValue = TRUE;
                              break;

                    case 's':
                    case 'b': longValue = TRUE;
                    case 'c':
                    case 'd':
                    case 'u':
                    case 'D':
                    case 'U':
                    case 'x':
		    case 'X': arg++;

                              formatCmd = FALSE;

                              if (argNum >= NUM_FMT_CMDS)
                              {
                                   CallHookPkt(putCharFunc,locale,(APTR)'%');
                                   i = start;
                              }
                              else
                              {
                                  lptr  = (ULONG *)&dataStream[2*(UWORD)args[argNum]];
                                  sptr  = (UWORD *)lptr;
                                  if (longValue)
                                  {
                                      value = (ULONG)*lptr;
                                  }
                                  else
                                  {
                                      if ((string[i] == 'd') || (string[i] == 'D'))
                                          value = (ULONG)((LONG)*(WORD *)sptr);
                                      else
                                          value = (ULONG)*sptr;
                                  }

                                  switch (string[i])
                                  {
                                      case 's': len = 0;
                                                if (str = (STRPTR)value)
                                                {
                                                    len = strlen(str);
                                                }
                                                numeric = FALSE;
                                                break;

                                      case 'b': len = 0;
                                                if (str = (STRPTR)(value * 4))
                                                {
                                                    len = (UWORD)(*str);
                                                    str++;
                                                }
                                                numeric = FALSE;
                                                break;

                                      case 'c': buffer[0] = (char)value;
                                                str       = buffer;
                                                len       = 1;
                                                numeric   = FALSE;
                                                break;

                                      case 'd': len     = ConvSigned((LONG)value,buffer);
                                                str     = buffer;
                                                numeric = TRUE;
                                                break;

                                      case 'u': len     = ConvUnsigned((ULONG)value,buffer);
                                                str     = buffer;
                                                numeric = FALSE;
                                                break;

                                      case 'D':
                                      case 'U': start = 0;
                                                if (string[i] == 'D')
                                                {
                                                    len = ConvSigned((LONG)value,buffer);
                                                    if ((LONG)value < 0)
                                                    {
                                                        numFormatBuffer[0] = '-';
                                                        start = 1;
                                                    }
                                                }
                                                else
                                                {
                                                    len = ConvUnsigned((ULONG)value,buffer);
                                                }

                                                str = buffer;
                                                if (sep = locale->el_Locale.loc_GroupSeparator)
                                                {
                                                    j   = start;
                                                    l   = len;
                                                    len = start;
                                                    str = numFormatBuffer;
                                                    while (buffer[j])
                                                    {
                                                        if ((locale->el_NumFormat[l-j]) && (j != start))
                                                        {
                                                            k = 0;
                                                            while (sep[k])
                                                                str[len++] = sep[k++];
                                                        }
                                                        str[len++] = buffer[j++];
                                                    }
                                                }
                                                numeric = TRUE;
                                                break;

                                      case 'X': len     = ConvHex((ULONG)value,buffer,longValue);
                                                str     = buffer;
                                                numeric = TRUE;
                                                break;

                                      case 'x': len     = ConvHexUpper((ULONG)value,buffer,longValue);
                                                str     = buffer;
                                                numeric = TRUE;
                                                break;
                                  }

                                  if (len > limit)
                                      len = limit;

                                  if (!leftJustify)
                                  {
                                      if ((padByte == '0')
                                      &&  (str[0] == '-')
                                      &&  (width > len)
                                      &&  (numeric))
                                      {
                                          CallHookPkt(putCharFunc,locale,(APTR)'-');
                                          str++;
                                      }

                                      while (width > len)
                                      {
                                          CallHookPkt(putCharFunc,locale,(APTR)padByte);
                                          width--;
                                      }
                                  }

                                  if (width > len)
                                      width -= len;
                                  else
                                      width = 0;

                                  while (len--)
                                  {
                                      CallHookPkt(putCharFunc,locale,(APTR)str[0]);
                                      str++;
                                  }

                                  while (width--)
                                  {
                                      CallHookPkt(putCharFunc,locale,(APTR)' ');
                                  }
                              }

                              break;

                    default : CallHookPkt(putCharFunc,locale,(APTR)string[i]);
                              formatCmd = FALSE;
                              break;

                }
                i++;
            }
        }
        else
        {
            CallHookPkt(putCharFunc,locale,(APTR)string[i]);
            i++;
        }
    }

    CallHookPkt(putCharFunc,locale,NULL);

    return (dataStream + 2*k);
}
