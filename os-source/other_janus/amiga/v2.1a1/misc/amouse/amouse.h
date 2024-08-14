/* Structure defs for buffer mem */
struct AMouseBufMem {
   UWORD AmigaPCX;
   UWORD AmigaPCY;
   UWORD AmigaPCLeftP;
   UWORD AmigaPCRightP;
   UWORD AmigaPCLeftR;
   UWORD AmigaPCRightR;
   UWORD AmigaPCStatus;
   };

/* structure defs for param mem */
struct AMouseParamMem {
   UWORD BufMemOffset;
   UWORD PCParam;
   UBYTE WriteLock;
   };

/*  Access masks */

/*
#define WordParam   (MEM_WORDACCESS+MEMF_PARAMETER)
#define WordBuff    (MEM_WORDACCESS+MEMF_BUFFER)
*/

/*  area sizes ( 8-byte aligned ) */

/*
#define DataSize    ((((AMouseBufMem_SIZEOF+7)>>3)<<3))
#define ParamSize   ((((AMouseParamMem_SIZEOF+7)>>3)<<3))
*/
