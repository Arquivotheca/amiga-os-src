��MELT2SIZE�"exec/types.h"�"exec/ports.h"�"exec/lists.h"�"libraries/dos.h"�LATTICE 1�LATTICE_V4 1�LATTICE_V5 1
���byte;�SECSIZ 512�FNSIZE 108�FMSIZE 256�FESIZE 32�DISKINFO InfoData�FILEINFO FileInfoBlock
�MELT
{
�MELT*fwd;
�size;
};�MELTSIZE sizeof(�MELT)
�MELT2
{
�MELT2*fwd;
�MELT2*bwd;
�size;
};�MELT2SIZE sizeof(�MELT2)
�ProcID{
�ProcID*nextID;
�Process*process;
�UserPortFlag;
��*parent;
��*child;
�seglist;
};
�FORKENV{
�priority;
�stack;
�std_in;
�std_out;
�console;
��*msgport;
};
�TermMsg{
��msg;
�class;
�type;
�Process*process;
�ret;
};�__ARGS���__ARGS(a) ()��__ARGS(a) a��
��forkl __ARGS((�*,�*,...));
��forkv __ARGS((�*,�**,�FORKENV*,�ProcID*));
��wait __ARGS((�ProcID*));
��ProcID*waitm __ARGS((�ProcID**));
��_dclose __ARGS((�));
��_dcreat __ARGS((�*,�));
��_dcreatx __ARGS((�*,�));
��dfind __ARGS((�FILEINFO*,�*,�));
��dnext __ARGS((�FILEINFO*));
��_dopen __ARGS((�*,�));
��_dread __ARGS((�,�*,�));
��_dseek __ARGS((�,�,�));
��_dwrite __ARGS((�,�*,�));
��getcd __ARGS((�,�*));
��getdfs __ARGS((�*,�DISKINFO*));
��getfa __ARGS((�*));
��getft __ARGS((�*));
��getpath __ARGS((�,�*));
��findpath __ARGS((�*));
��datecmp __ARGS((�[],�[]));
��chgclk __ARGS((��*));
��getclk __ARGS((��*));
��onbreak __ARGS((�__ARGS((*))__ARGS(())));
��poserr __ARGS((�*));
��chkabort __ARGS((�));
��Chk_Abort __ARGS((�));
�DeviceList*getasn __ARGS((�*));�geta4 __builtin_geta4
��geta4 __ARGS((�));�getreg __builtin_getreg
��getreg __ARGS((�));�putreg __builtin_putreg
��putreg __ARGS((�,�));�__emit __builtin_emit
��__emit __ARGS((�));�REG_D0 0�REG_D1 1�REG_D2 2�REG_D3 3�REG_D4 4�REG_D5 5�REG_D6 6�REG_D7 7�REG_A0 8�REG_A1 9�REG_A2 10�REG_A3 11�REG_A4 12�REG_A5 13�REG_A6 14�REG_A7 15�