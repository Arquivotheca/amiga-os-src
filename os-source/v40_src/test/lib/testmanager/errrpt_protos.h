
#ifndef ERRRPT_PROTOS_H
#define ERRRPT_PROTOS_H

/* Prototypes for functions defined in ErrRpt.c */
struct errReport *initTstData(STRPTR clientName,
                              STRPTR serverName);
void freeTstData(struct errReport *er);
BYTE errRpt(struct errReport *er,
            UWORD eL,
            BYTE eT,
            LONG eC,
            STRPTR fmt,
            LONG arg1, ...);
BOOL configRpt(struct errReport *er,
               UWORD el,
               BYTE et,
               LONG ec);
BYTE quickRpt(struct errReport *er,
              STRPTR fmt,
              LONG arg1, ...);
BYTE quickARpt(struct errReport *er,
               STRPTR fmt,
               APTR arg1, ...);
BYTE forceRpt(struct errReport *er,
              STRPTR fmt,
              LONG arg1, ...);
BYTE forceARpt(struct errReport *er,
               STRPTR fmt,
               APTR arg1, ...);
BYTE errARpt(struct errReport *er,
             UWORD eL,
             BYTE eT,
             LONG eC,
             STRPTR fmt,
             APTR arg1, ...);
BYTE forceErrRpt(struct errReport *er,
                 UWORD eL,
                 BYTE eT,
                 LONG eC,
                 STRPTR fmt,
                 LONG arg1, ...);
BYTE forceAErrRpt(struct errReport *er,
                  UWORD eL,
                  BYTE eT,
                  LONG eC,
                  STRPTR fmt,
                  APTR arg1, ...);

#endif /* ERRRPT_PROTOS_H */
