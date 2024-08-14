/* Prototypes for functions defined in c/TokenTools.c */
char *tt_GetToken(char *b,
                  char *tok_buf,
                  struct TTBreak *brk,
                  struct TTBreak **brk_ent,
                  struct TTBreak *kwd,
                  struct TTBreak **kwd_ent,
                  char *kwd_brk);
