/****************
  fakefreeanim.h

   W.D.L 931005
*****************/


#define	MAX_SPRITES		8
#define	MAX_SPRITE_WIDTH	((GfxBase->ChipRevBits0 & GFXF_AA_ALICE) ? 64 : 32)
#define	SPRITE_SEPERATION	((GfxBase->ChipRevBits0 & GFXF_AA_ALICE) ? 32 : 16)

typedef struct SpriteContext
{
    struct ExtSprite	* extsprite[MAX_SPRITES];
    LONG		  spritenum[MAX_SPRITES];
    ULONG		  flags;
    SHORT		  width;
    SHORT		  numsprites;

} SPRITECONTEXT;


struct FFAnimLibrary {
    struct			  Library ff_Lib;
    ULONG			  ff_SegList;
    ULONG			  ff_Flags;
    APTR			  ff_ExecBase;      /* pointer to exec base */
    struct Screen		* screen;
    SPRITECONTEXT		* spc;
    SHORT			  opencount;
    SHORT			  closecount;
#ifndef ONE_GLOBAL_SECTION	// [
    long			* ff_relocs;        /* pointer to relocs. */
    struct FFAnimLibrary	* ff_origbase; /* pointer to original library base  */
    long			  ff_numjmps;
#endif				// ]
};

int kprintf(const char *, ...);
VOID __saveds LIBStopFakeAnim( VOID );
int __saveds StartFakeAnim( VOID );


