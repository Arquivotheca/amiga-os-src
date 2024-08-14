
/* makeanim.c */
APTR newdtobject ( STRPTR name , Tag tag1 , ...);
ULONG getdtattrs ( Object *o , ULONG data , ...);
ULONG setdtattrs ( Object *o , ULONG data , ...);
Object *loadPicture ( STRPTR name );
LONG *CreateDeltaData ( struct BitMap *bm1 , struct BitMap *bm2 , LONG *deltan );
BOOL saveDelta ( struct IFFHandle *iff , struct BitMap *bm1 , struct BitMap *bm2 );
BOOL savePicture ( STRPTR filename , struct IFFHandle *iff , Object *o );
struct BitMap *allocbitmap ( UWORD w , UWORD h , UBYTE d , ULONG attr );
void freebitmap ( struct BitMap *bm );
void CopyBitMap ( struct BitMap *bm1 , struct BitMap *bm2 );
int main ( int argc , char **argv );

/* skip.c */
LONG skip_count_plane ( UBYTE *in , UBYTE *last_in , LONG next_line , LONG rows );
UBYTE *skip_comp_plane ( UBYTE *in , UBYTE *last_in , UBYTE *out , LONG next_line , LONG rows );
