ÄàMELT2SIZEå"exec/types.h"å"exec/ports.h"å"exec/lists.h"å"libraries/dos.h"ÄLATTICE 1ÄLATTICE_V4 1ÄLATTICE_V5 1
•éÑbyte;ÄSECSIZ 512ÄFNSIZE 108ÄFMSIZE 256ÄFESIZE 32ÄDISKINFO InfoDataÄFILEINFO FileInfoBlock
ÉMELT
{
ÉMELT*fwd;
çsize;
};ÄMELTSIZE sizeof(ÉMELT)
ÉMELT2
{
ÉMELT2*fwd;
ÉMELT2*bwd;
ésize;
};ÄMELT2SIZE sizeof(ÉMELT2)
ÉProcID{
ÉProcID*nextID;
ÉProcess*process;
ÇUserPortFlag;
É©*parent;
É©*child;
°seglist;
};
ÉFORKENV{
çpriority;
çstack;
°std_in;
°std_out;
°console;
É©*msgport;
};
ÉTermMsg{
ÉØmsg;
çclass;
ütype;
ÉProcess*process;
çret;
};à__ARGS∞™Ä__ARGS(a) ()ùÄ__ARGS(a) aáá
ÅÇforkl __ARGS((Ñ*,Ñ*,...));
ÅÇforkv __ARGS((Ñ*,Ñ**,ÉFORKENV*,ÉProcID*));
ÅÇwait __ARGS((ÉProcID*));
ÅÉProcID*waitm __ARGS((ÉProcID**));
ÅÇ_dclose __ARGS((ç));
Åç_dcreat __ARGS((Ñ*,Ç));
Åç_dcreatx __ARGS((Ñ*,Ç));
ÅÇdfind __ARGS((ÉFILEINFO*,Ñ*,Ç));
ÅÇdnext __ARGS((ÉFILEINFO*));
Åç_dopen __ARGS((Ñ*,Ç));
Åé_dread __ARGS((ç,Ñ*,é));
Åç_dseek __ARGS((ç,ç,Ç));
Åé_dwrite __ARGS((ç,Ñ*,é));
ÅÇgetcd __ARGS((Ç,Ñ*));
ÅÇgetdfs __ARGS((Ñ*,ÉDISKINFO*));
ÅÇgetfa __ARGS((Ñ*));
Åçgetft __ARGS((Ñ*));
ÅÇgetpath __ARGS((°,Ñ*));
Å°findpath __ARGS((Ñ*));
ÅÇdatecmp __ARGS((ç[],ç[]));
ÅÇchgclk __ARGS((éÑ*));
Åãgetclk __ARGS((éÑ*));
ÅÇonbreak __ARGS((Ç__ARGS((*))__ARGS(())));
ÅÇposerr __ARGS((Ñ*));
ÅÇchkabort __ARGS((ã));
ÅÇChk_Abort __ARGS((ã));
ÉDeviceList*getasn __ARGS((Ñ*));Ägeta4 __builtin_geta4
Åãgeta4 __ARGS((ã));Ägetreg __builtin_getreg
Åçgetreg __ARGS((Ç));Äputreg __builtin_putreg
Åãputreg __ARGS((Ç,ç));Ä__emit __builtin_emit
Åã__emit __ARGS((Ç));ÄREG_D0 0ÄREG_D1 1ÄREG_D2 2ÄREG_D3 3ÄREG_D4 4ÄREG_D5 5ÄREG_D6 6ÄREG_D7 7ÄREG_A0 8ÄREG_A1 9ÄREG_A2 10ÄREG_A3 11ÄREG_A4 12ÄREG_A5 13ÄREG_A6 14ÄREG_A7 15á