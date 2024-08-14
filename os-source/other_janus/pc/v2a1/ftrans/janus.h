 
/* Header file for some useful Janus structures */
 
typedef unsigned long  APTR;
typedef unsigned short UWORD;
typedef UWORD          RPTR;
 
struct JanusMemHead {
   char   jmh_Lock;
   char   jmh_pad0;
   APTR   jmh_68000Base;
   UWORD  jmh_8088Segment;
   UWORD  jmh_First;
   UWORD  jmh_Max;
   UWORD  jmh_Free;
   };
 
struct JanusBase {
   char   jb_Lock;
   char   jb_8088Go;
   struct JanusMemHead jb_ParamMem;
   struct JanusMemHead jb_BufferMem;
   UWORD  jb_Interrupts;
   UWORD  jb_Parameters;
   UWORD  jb_NumINterrupts;
   };
 
 
#define ADR_FNCTN_READ 1      /* Read from file */
#define ADR_FNCTN_WRITE 2     /* Write to file  */
#define ADR_FNCTN_OPEN_OLD 5  /* Open old file  */
#define ADR_FNCTN_OPEN_NEW 6  /* Open new file  */
#define ADR_FNCTN_CLOSE 7     /* Close file     */
 
struct AmigaDiskReq
{ short adr_Fnctn;
  short adr_Part;
  short adr_Offset_h;
  short adr_Offset_l;
  short adr_Count_h;
  short adr_Count_l;
  short adr_BufferOffset;
  short adr_Err;
};
 