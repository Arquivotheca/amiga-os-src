
/* asciibase.c */
Class *ASM ObtainASCIIEngine ( REG (a6 )struct ASCIILib *ascii );
struct Library *ASM LibInit ( REG (d0 )struct ASCIILib *ascii , REG (a0 )BPTR seglist , REG (a6 )struct Library *sysbase );
LONG ASM LibOpen ( REG (a6 )struct ASCIILib *ascii );
LONG ASM LibClose ( REG (a6 )struct ASCIILib *ascii );
LONG ASM LibExpunge ( REG (a6 )struct ASCIILib *ascii );

/* dispatch.c */
Class *initClass ( struct ASCIILib *ascii );
ULONG ASM Dispatch ( REG (a0 )Class *cl , REG (a2 )Object *o , REG (a1 )Msg msg );
ULONG layoutMethod ( struct ASCIILib *ascii , Class *cl , Object *o , struct gpLayout *gpl );
