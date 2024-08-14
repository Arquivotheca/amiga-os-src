struct RexxMsg *CreateRexxMsg(struct MsgPort *  /* rexx_port    */,
                              char *            /* extension    */,
                              char *            /* host         */
                             );

STRPTR CreateArgstring(char *                   /* buf          */,
                       int                      /* len          */
                      );

void DeleteRexxMsg(struct RexxMsg *             /* rexxmessage  */
                  );

void DeleteArgstring(STRPTR             /* arg  */
                    );

int __stdargs SetRexxVar(struct RexxMsg *, char *, char *, int);