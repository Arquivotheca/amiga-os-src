/* Prototypes for functions defined in c/FlagTools.c */
ULONG DoFlagBits(struct KeyMode *kma,
                 UBYTE *str);
ULONG _DoFlagValue(struct KeyMode *kma,
                   UBYTE *str,
                   WORD start,
                   WORD end);
ULONG DoFlagState(struct KeyMode *kma,
                  UBYTE *str);
