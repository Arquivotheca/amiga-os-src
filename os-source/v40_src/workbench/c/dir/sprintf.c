/***
*
* This is a tiny implementation of sprintf for use within the editor
*  It should elminate most needs to format numbers
*  Note that it assumes that the buffer is large enough to hold any
*  formatted string
*
***/
#pragma syscall RawDoFmt 20a ba9804
extern void __builtin_emit(UWORD);

static void __regargs prbuf(char c);
void RawDoFmt(char *, long[], void(*)(), char *);

int vsprintf(buf, ctl, args)
char *buf;
char *ctl;
long args[];
{
   RawDoFmt(ctl, args, prbuf, buf);
   return(strlen(buf));
}

/***
*
* The following stub routine is called from RawDoFmt for each
* character in the string.  At invocation, we have:
*    D0 - next character to be formatted
*    A3 - pointer to data buffer
*
***/
static void __regargs prbuf(char c)
{
__builtin_emit(0x16c0);   /* move.b D0,(A3)+ */
}
