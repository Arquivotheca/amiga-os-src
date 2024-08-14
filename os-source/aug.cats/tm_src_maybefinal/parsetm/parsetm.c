
#include <exec/types.h>

#include "ParseTM.h"

#include <libraries/iffparse.h>
#include <dos/dos.h>
#include <string.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/iffparse_protos.h>

void main(int, char **);
void ParseTMFile(char *);
void StripNewLine(char *);
BOOL GetBytes(void *, int, int, struct IFFHandle *, BOOL);
BOOL GetString(char *, int, struct IFFHandle *, BOOL);
void PutIFFError(LONG);

extern struct Library *DOSBase;
extern struct Library *SysBase;

struct Library *IFFParseBase;

#ifdef LATTICE
int CXBRK(void) { return(0); }
int chkabort(void) { return(0); }
#endif

/****************************************************************************/

void main(int argc, char **argv)
  {
  if(argc != 2)
    {
    printf("Usage: %s <projectfile>\n", argv[0]);
    }
  else
    {
    if(IFFParseBase = OpenLibrary("iffparse.library", 37))
      {
      ParseTMFile(argv[1]);
      CloseLibrary(IFFParseBase);
      }
    }
  }

/****************************************************************************/

void ParseTMFile(char *filename)
  {
  char   dummystring[STRINGSIZE];
  char   dummybyte;
  ULONG  dummylong;
  WORD   dummyword;
  WORD   datacount;
  LONG   i;
  LONG   ifferror;
  BOOL   st=TRUE;
  UBYTE  error=TMFILEERR_NONE;
  struct TextAttr      dummytextattr;
  struct Rectangle     dummyrectangle;
  struct NewGadget     dummynewgadget;
  struct IFFHandle     *iff;
  struct ContextNode   *chunk;

  if(iff = AllocIFF())
    {
    if(iff->iff_Stream = Open(filename, MODE_OLDFILE))
      {
      InitIFFasDOS(iff);

      if(OpenIFF(iff, IFFF_READ) == 0)
        {
        if(StopChunk(iff, ID_TMUI, ID_FORM) == 0)
          {
          if(ifferror = ParseIFF(iff, IFFPARSE_SCAN) == 0)
            {
            while((((ifferror = ParseIFF(iff, IFFPARSE_RAWSTEP)) == 0) || (ifferror == IFFERR_EOC)) && (ifferror != IFFERR_EOF) && st && !(SetSignal(0,0) & SIGBREAKF_CTRL_C))
              {
              if((chunk = CurrentChunk(iff)) && (ifferror != IFFERR_EOC))
                {
                printf("--------------------------\n");

                switch(chunk->cn_ID)
                  {
                  /**************************************************************/

                  case ID_SCOM:
                    printf("Chunk: Code Comments (SCOM)\n");

                    st = GetBytes(&dummybyte, sizeof(UBYTE), 1, iff, st);
                    printf(" Flag = %s\n", dummybyte ? "TRUE" : "FALSE");

                    break;

                  /**************************************************************/

                  case ID_SATB:
                    printf("Chunk: Auto Top Border (SATB)\n");

                    st = GetBytes(&dummybyte, sizeof(UBYTE), 1, iff, st);
                    printf(" Flag = %s\n", dummybyte ? "TRUE" : "FALSE");

                    break;

                  /**************************************************************/

                  case ID_SPRA:
                    printf("Chunk: Use Pragmas (SPRA)\n");

                    st = GetBytes(&dummybyte, sizeof(UBYTE), 1, iff, st);
                    printf(" Flag = %s\n", dummybyte ? "TRUE" : "FALSE");

                    break;

                  /**************************************************************/

                  case ID_SGSZ:
                    printf("Chunk: Grid Size (SGSZ)\n");

                    st = GetBytes(&dummybyte, sizeof(UBYTE), 1, iff, st);
                    printf(" Size = %s\n", dummybyte ? "TRUE" : "FALSE");

                    st = GetBytes(&dummybyte, sizeof(UBYTE), 1, iff, st);
                    printf(" <reserved BYTE>\n");

                    break;

                  /**************************************************************/

                  case ID_SICN:
                    printf("Chunk: Create Icons (SICN)\n");

                    st = GetBytes(&dummybyte,   sizeof(UBYTE), 1, iff, st);
                    printf(" Flag = %s\n", dummybyte ? "TRUE" : "FALSE");

                    break;

                  /**************************************************************/

                  case ID_SUSG:
                    printf("Chunk: User Signal (SUSG)\n");

                    st = GetBytes(&dummybyte, sizeof(UBYTE), 1, iff, st);
                    printf(" Flag = %s\n", dummybyte ? "TRUE" : "FALSE");

                    break;

                  /**************************************************************/

                  case ID_SARX:
                    printf("Chunk: Use SimpleRexx (SARX)\n");

                    st = GetBytes(&dummybyte,  sizeof(UBYTE), 1, iff, st);
                    printf(" Flag = %s\n", dummybyte ? "TRUE" : "FALSE");

                    break;

                  /**************************************************************/

                  case ID_SCHP:
                    printf("Chunk: Use __chip Keyword (SCHP)\n");

                    st = GetBytes(&dummybyte,  sizeof(UBYTE), 1, iff, st);
                    printf(" Flag = %s\n", dummybyte ? "TRUE" : "FALSE");

                    break;

                  /**************************************************************/

                  case ID_SUDA:
                    printf("Chunk: Use UserData Structures (SUDA)\n");

                    st = GetBytes(&dummybyte,  sizeof(UBYTE), 1, iff, st);
                    printf(" Flag = %s\n", dummybyte ? "TRUE" : "FALSE");

                    break;

                  /**************************************************************/

                  case ID_SFRQ:
                    printf("Chunk: Use ASL File Requester (SFRQ)\n");

                    st = GetBytes(&dummybyte,  sizeof(UBYTE), 1, iff, st);
                    printf(" Flag = %s\n", dummybyte ? "TRUE" : "FALSE");

                    break;

                  /**************************************************************/

                  case ID_TSDA:
                    printf("Chunk: Screen Data (TSDA)\n");

                    st = GetBytes(&dummylong, sizeof(ULONG), 1, iff, st);
                    printf(" DisplayID = 0x%x\n", dummylong);

                    st = GetBytes(&dummylong, sizeof(ULONG), 1, iff, st);
                    printf(" Width = %d\n", dummylong);

                    st = GetBytes(&dummylong, sizeof(ULONG), 1, iff, st);
                    printf(" Height = %d\n", dummylong);

                    st = GetBytes(&dummylong, sizeof(ULONG), 1, iff, st);
                    printf(" Depth = %d\n", dummylong);

                    st = GetBytes(&dummylong, sizeof(ULONG), 1, iff, st);
                    printf(" Overscan = 0x%x\n", dummylong);

                    st = GetBytes(&dummylong, sizeof(ULONG), 1, iff, st);
                    printf(" TMSFlags = 0x%x\n", dummylong);

                    st = GetBytes(&dummylong, sizeof(ULONG), 1, iff, st);
                    printf(" Mode = 0x%x\n", dummylong);

                    st = GetBytes(&dummyrectangle, sizeof(struct Rectangle), 1, iff, st);
                    printf(" <struct Rectangle unused>\n");

                    st = GetBytes(&dummylong, sizeof(ULONG), 1, iff, st);
                    printf(" <reserved LONG>\n");

                    st = GetBytes(&dummylong, sizeof(ULONG), 1, iff, st);
                    printf(" <reserved LONG>\n");

                    st = GetString(dummystring, STRINGSIZE, iff, st);
                    printf(" Title = '%s'\n", dummystring);

                    st = GetString(dummystring, STRINGSIZE, iff, st);
                    printf(" Source Label = '%s'\n", dummystring);

                    st = GetString(dummystring, STRINGSIZE, iff, st);
                    printf(" <reserved string>\n");

                    break;

                  /**************************************************************/

                  case ID_CMAP:
                    printf("Chunk: Color map (CMAP)\n");

                    datacount = MIN(32, chunk->cn_Size/3);
                    for(i=0; i<datacount; i++)
                      {
                      st = GetBytes(&dummybyte, sizeof(UBYTE), 1, iff, st);
                      printf(" Color %d\tR:0x%x\t", i, (UBYTE) dummybyte);

                      st = GetBytes(&dummybyte, sizeof(UBYTE), 1, iff, st);
                      printf("G:0x%x\t", (UBYTE) dummybyte);

                      st = GetBytes(&dummybyte, sizeof(UBYTE), 1, iff, st);
                      printf("B:0x%x\n", (UBYTE) dummybyte);
                      }

                    break;

                  /**************************************************************/

                  case ID_TWDA:
                    printf("Chunk: Window Data (TWDA)\n");

                    st = GetBytes(&dummylong, sizeof(ULONG), 1, iff, st);
                    printf(" IDCMP: 0x%x\n", dummylong);

                    st = GetBytes(&dummylong, sizeof(ULONG), 1, iff, st);
                    printf(" TMWFlags = 0x%x\n", dummylong);

                    st = GetBytes(&dummylong, sizeof(ULONG), 1, iff, st);
                    printf(" <reserved LONG>\n");

                    st = GetBytes(&dummylong, sizeof(ULONG), 1, iff, st);
                    printf(" <reserved LONG>\n");

                    st = GetString(dummystring, STRINGSIZE, iff, st);
                    printf(" Title = '%s'\n", dummystring);

                    st = GetString(dummystring, STRINGSIZE, iff, st);
                    printf(" Source Label = '%s'\n", dummystring);

                    st = GetString(dummystring, STRINGSIZE, iff, st);
                    printf(" <reserved string>\n");

                    break;

                  /**************************************************************/

                  case ID_TMDA:
                    printf("Chunk: Menu Data (TMDA)\n");

                    st = GetBytes(&dummylong, sizeof(ULONG), 1, iff, st);
                    printf(" Position = %d\n", dummylong);

                    st = GetBytes(&dummyword, sizeof(UWORD), 1, iff, st);
                    printf(" Flags = %d\n", dummyword);

                    st = GetBytes(&dummylong, sizeof(ULONG), 1, iff, st);
                    printf(" TMMFlags = %d\n", dummylong);

                    st = GetBytes(&dummylong, sizeof(ULONG), 1, iff, st);
                    printf(" <reserved LONG>\n");

                    st = GetBytes(&dummylong, sizeof(ULONG), 1, iff, st);
                    printf(" <reserved LONG>\n");

                    st = GetString(dummystring, STRINGSIZE, iff, st);
                    printf(" Menu Text = '%s'\n", dummystring);

                    st = GetString(dummystring, STRINGSIZE, iff, st);
                    printf(" Source Label = '%s'\n", dummystring);

                    st = GetString(dummystring, STRINGSIZE, iff, st);
                    printf(" <reserved string>\n");

                    break;

                  /**************************************************************/

                  case ID_TIDA:
                    printf("Chunk: Menu Item Data (TIDA)\n");

                    st = GetBytes(&dummylong, sizeof(ULONG), 1, iff, st);
                    printf(" Position = %d\n", dummylong);

                    st = GetBytes(&dummyword, sizeof(UWORD), 1, iff, st);
                    printf(" Flags = %d\n", dummyword);

                    st = GetBytes(&dummylong, sizeof(ULONG), 1, iff, st);
                    printf(" TMMFlags = %d\n", dummylong);

                    st = GetBytes(&dummylong, sizeof(ULONG), 1, iff, st);
                    printf(" <reserved LONG>\n");

                    st = GetBytes(&dummylong, sizeof(ULONG), 1, iff, st);
                    printf(" <reserved LONG>\n");

                    st = GetBytes(&dummylong, sizeof(ULONG), 1, iff, st);
                    printf(" <reserved LONG>\n");

                    st = GetString(dummystring, STRINGSIZE, iff, st);
                    printf(" Item Text = '%s'\n", dummystring);

                    st = GetString(dummystring, STRINGSIZE, iff, st);
                    printf(" Command Key = '%s'\n", dummystring);

                    st = GetString(dummystring, STRINGSIZE, iff, st);
                    printf(" Source Label = '%s'\n", dummystring);

                    st = GetString(dummystring, STRINGSIZE, iff, st);
                    printf(" <reserved string>\n");

                    break;

                  /**************************************************************/

                  case ID_TSID:
                    printf("Chunk: Menu Subitem (TSID)\n");

                    st = GetBytes(&dummylong, sizeof(ULONG), 1, iff, st);
                    printf(" Position = %d\n", dummylong);

                    st = GetBytes(&dummyword, sizeof(UWORD), 1, iff, st);
                    printf(" Flags = %d\n", dummyword);

                    st = GetBytes(&dummylong, sizeof(ULONG), 1, iff, st);
                    printf(" TMMFlags = %d\n", dummylong);

                    st = GetBytes(&dummylong, sizeof(ULONG), 1, iff, st);
                    printf(" <reserved LONG>\n");

                    st = GetBytes(&dummylong, sizeof(ULONG), 1, iff, st);
                    printf(" <reserved LONG>\n");

                    st = GetBytes(&dummylong, sizeof(ULONG), 1, iff, st);
                    printf(" <reserved LONG>\n");

                    st = GetString(dummystring, STRINGSIZE, iff, st);
                    printf(" Subitem Text = '%s'\n", dummystring);

                    st = GetString(dummystring, STRINGSIZE, iff, st);
                    printf(" Command Key = '%s'\n", dummystring);

                    st = GetString(dummystring, STRINGSIZE, iff, st);
                    printf(" Source Label = '%s'\n", dummystring);

                    st = GetString(dummystring, STRINGSIZE, iff, st);
                    printf(" <reserved string>\n");

                    break;

                  /**************************************************************/

                  case ID_TGDA:
                    printf("Chunk: Gadget Data (TGDA)\n");

                    st = GetBytes(&dummylong, sizeof(ULONG), 1, iff, st);
                    printf(" Position = %d\n", dummylong);

                    st = GetBytes(&dummylong, sizeof(ULONG), 1, iff, st);
                    printf(" Kind = %d\n", dummylong);

                    st = GetBytes(&dummylong, sizeof(ULONG), 1, iff, st);
                    printf(" TMGFlags = 0x%x\n", dummylong);

                    st = GetBytes(&dummynewgadget, sizeof(struct NewGadget), 1, iff, st);
                    printf(" NewGadget.ng_LeftEdge   = %d\n", dummynewgadget.ng_LeftEdge);
                    printf(" NewGadget.ng_TopEdge    = %d\n", dummynewgadget.ng_TopEdge);
                    printf(" NewGadget.ng_Width      = %d\n", dummynewgadget.ng_Width);
                    printf(" NewGadget.ng_Height     = %d\n", dummynewgadget.ng_Height);
                    printf(" NewGadget.ng_GadgetText = <unused>\n");
                    printf(" NewGadget.ng_TextAttr   = <unused>\n");
                    printf(" NewGadget.ng_GadgetID   = <unused>\n");
                    printf(" NewGadget.ng_Flags      = <unused>\n");
                    printf(" NewGadget.ng_VisualInfo = <unused>\n");
                    printf(" NewGadget.ng_UserData   = <unused>\n");

                    st = GetBytes(&dummylong, sizeof(ULONG), 1, iff, st);
                    printf(" <reserved LONG>\n");

                    st = GetBytes(&dummylong, sizeof(ULONG), 1, iff, st);
                    printf(" <reserved LONG>\n");

                    st = GetString(dummystring, STRINGSIZE, iff, st);
                    printf(" Gadget Text = '%s'\n", dummystring);

                    st = GetString(dummystring, STRINGSIZE, iff, st);
                    printf(" Source Label = '%s'\n", dummystring);

                    st = GetString(dummystring, STRINGSIZE, iff, st);
                    printf(" <reserved string>\n");

                    break;

                  /**************************************************************/

                  case ID_TTAT:
                    printf("Chunk: Font (TTAT)\n");

                    st = GetBytes(&dummytextattr, sizeof(struct TextAttr), 1, iff, st);
                    printf(" TextAttr.ta_Name  = <unnused>\n");
                    printf(" TextAttr.ta_YSize = %d\n", dummytextattr.ta_YSize);
                    printf(" TextAttr.ta_Style = 0x%x\n", dummytextattr.ta_Style);
                    printf(" TextAttr.ta_Flags = 0x%x\n", dummytextattr.ta_Flags);

                    st = GetString(dummystring, STRINGSIZE, iff, st);
                    printf(" Font Name = '%s'\n", dummystring);

                    break;

                  /**************************************************************/

                  case ID_TCHA:
                    printf("Chunk: Character Tag (TCHA)\n");

                    st = GetBytes(&dummylong, sizeof(ULONG), 1, iff, st);
                    printf(" Tag = 0x%x\n", dummylong);

                    st = GetBytes(&dummylong, sizeof(ULONG), 1, iff, st);
                    printf(" Character = '%c'\n", dummylong);

                    st = GetBytes(&dummylong, sizeof(ULONG), 1, iff, st);
                    printf(" <reserved LONG>\n");

                    break;

                  /**************************************************************/

                  case ID_TINT:
                    printf("Chunk: Integer Tag (TINT)\n");

                    st = GetBytes(&dummylong, sizeof(ULONG), 1, iff, st);
                    printf(" Tag = 0x%x\n", dummylong);

                    st = GetBytes(&dummylong, sizeof(ULONG), 1, iff, st);
                    printf(" Integer = %d\n", dummylong);

                    st = GetBytes(&dummylong, sizeof(ULONG), 1, iff, st);
                    printf(" <reserved LONG>\n");

                    st = GetString(dummystring, STRINGSIZE, iff, st);
                    printf(" Value = '%s'\n", dummystring);

                    break;

                  /**************************************************************/

                  case ID_TNIN:
                    printf("Chunk: Non-interactive Tag (TNIN)\n");

                    st = GetBytes(&dummylong, sizeof(ULONG), 1, iff, st);
                    printf(" Tag = 0x%x\n", dummylong);

                    st = GetBytes(&dummylong, sizeof(ULONG), 1, iff, st);
                    printf(" <reserved LONG>\n");

                    st = GetString(dummystring, STRINGSIZE, iff, st);
                    printf(" Value = '%s'\n", dummystring);

                    break;

                  /**************************************************************/

                  case ID_TSTR:
                    printf("Chunk: String Tag (TSTR)\n");

                    st = GetBytes(&dummylong, sizeof(ULONG), 1, iff, st);
                    printf(" Tag = 0x%x\n", dummylong);

                    st = GetBytes(&dummylong, sizeof(ULONG), 1, iff, st);
                    printf(" <reserved LONG>\n");

                    st = GetBytes(&dummylong, sizeof(ULONG), 1, iff, st);
                    printf(" <reserved LONG>\n");

                    st = GetString(dummystring, STRINGSIZE, iff, st);
                    printf(" String = '%s'\n", dummystring);

                    break;

                  /**************************************************************/

                  case ID_TSLS:
                    printf("Chunk: String List Tag (TSLS)\n");

                    st = GetBytes(&dummyword, sizeof(UWORD), 1, iff, st);
                    printf(" Data Count = %d\n", dummyword);

                    st = GetBytes(&dummylong, sizeof(ULONG), 1, iff, st);
                    printf(" Tag = 0x%x\n", dummylong);

                    datacount = dummyword;

                    for(i=0; i<datacount; i++)
                      {
                      st = GetBytes(&dummylong, sizeof(ULONG), 1, iff, st);
                      printf(" <reserved LONG>\n");
                      }

                    for(i=0; i<datacount; i++)
                      {
                      st = GetBytes(&dummylong, sizeof(ULONG), 1, iff, st);
                      printf(" <reserved LONG>\n");
                      }

                    for(i=0; i<datacount; i++)
                      {
                      st = GetString(dummystring, STRINGSIZE, iff, st);
                      printf(" String = '%s'\n", dummystring);
                      }

                    for(i=0; i<datacount; i++)
                      {
                      st = GetString(dummystring, STRINGSIZE, iff, st);
                      printf(" Source Label = '%s'\n", dummystring);
                      }

                    break;

                  /**************************************************************/

                  case ID_TLLS:
                    printf("Chunk: Linked List Tag (TLLS)\n");

                    st = GetBytes(&dummyword, sizeof(UWORD), 1, iff, st);
                    printf(" Data Count = %d\n", dummyword);

                    st = GetBytes(&dummylong, sizeof(ULONG), 1, iff, st);
                    printf(" Tag = 0x%x\n", dummylong);

                    datacount = dummyword;

                    for(i=0; i<datacount; i++)
                      {
                      st = GetBytes(&dummylong, sizeof(ULONG), 1, iff, st);
                      printf(" <reserved LONG>\n");
                      }

                    for(i=0; i<datacount; i++)
                      {
                      st = GetBytes(&dummylong, sizeof(ULONG), 1, iff, st);
                      printf(" <reserved LONG>\n");
                      }

                    for(i=0; i<datacount; i++)
                      {
                      st = GetString(dummystring, STRINGSIZE, iff, st);
                      printf(" String = '%s'\n", dummystring);
                      }

                    for(i=0; i<datacount; i++)
                      {
                      st = GetString(dummystring, STRINGSIZE, iff, st);
                      printf(" Source Label = '%s'\n", dummystring);
                      }

                    break;

                  /**************************************************************/

                  case ID_TWLS:
                    printf("Chunk: WORD List Tag (TWLS)\n");

                    st = GetBytes(&dummyword, sizeof(UWORD), 1, iff, st);
                    printf(" Data Count = %d\n", dummyword);

                    st = GetBytes(&dummylong, sizeof(ULONG), 1, iff, st);
                    printf(" Tag = 0x%x\n", dummylong);

                    st = GetBytes(&dummylong, sizeof(ULONG), 1, iff, st);
                    printf(" <reserved LONG>\n");

                    datacount = dummyword;

                    for(i=0; i<datacount; i++)
                      {
                      st = GetBytes(&dummyword, sizeof(WORD), 1, iff, st);
                      printf(" Word %d = %d\n", i, dummyword);
                      }

                    break;

                  /**************************************************************/
                  }
                }
              }
            }
          else
            {
            error = TMFILEERR_WRONGTYPE;
            }
          }
        else
          {
          error = TMFILEERR_OUTOFMEM;
          }

        PutIFFError(ifferror);

        CloseIFF(iff);
        }
      else
        {
        error = TMFILEERR_OPEN;
        }

      Close(iff->iff_Stream);
      }
    else
      {
      error = TMFILEERR_OPEN;
      }

    FreeIFF(iff);
    }
  else
    {
    error = TMFILEERR_OUTOFMEM;
    }

  if(st==FALSE) error = TMFILEERR_READ;

  if(error)
    {
    switch(error)
      {
      case TMFILEERR_OPEN:
        printf("Error opening project file\n");
        break;

      case TMFILEERR_WRONGTYPE:
        printf("Improper project file format\n");
        break;

      case TMFILEERR_OUTOFMEM:
        printf("Out of memory\n");
        break;

      case TMFILEERR_READ:
        printf("Error reading project file\n");
        break;
      }
    }
  }

/****************************************************************************/

BOOL GetBytes(void *address, int size, int count, struct IFFHandle *iff, BOOL st)
  {
  char *firstchar;
  LONG totalsize;

  if(st)
    {
    totalsize = size*count;
    if(ReadChunkBytes(iff, address, totalsize) != totalsize)
      {
      firstchar = address;
      *firstchar = 0;
      st = FALSE;
      }
    }

  return(st);
  }

/****************************************************************************/

BOOL GetString(char *string, int size, struct IFFHandle *iff, BOOL st)
  {
  UBYTE *chptr;
  UBYTE ch;

  if(st)
    {
    chptr = string;

    while(((st = ReadChunkBytes(iff, &ch, 1)) == 1) && (ch != '\n') && st)
      {
      *chptr++ = ch;
      }
    *chptr = '\0';
    }

  return(st);
  }

/****************************************************************************/

VOID PutIFFError(LONG ifferror)
  {
  switch(ifferror)
    {
    case IFFERR_NOSCOPE:
      printf("No valid scope for property");
      break;

    case IFFERR_NOMEM:
      printf("Internal memory alloc failed");
      break;

    case IFFERR_READ:
      printf("Stream read error");
      break;

    case IFFERR_WRITE:
      printf("Stream write error");
      break;

    case IFFERR_SEEK:
      printf("Stream seek error");
      break;

    case IFFERR_MANGLED:
      printf("Data in file is corrupt");
      break;

    case IFFERR_SYNTAX:
      printf("IFF syntax error");
      break;

    case IFFERR_NOTIFF:
      printf("Not an IFF file");
      break;

    case IFFERR_NOHOOK:
      printf("No call-back hook provided");
      break;
    }
  }

