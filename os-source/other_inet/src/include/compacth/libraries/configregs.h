��LIBRARIES_CONFIGREGS_H�LIBRARIES_CONFIGREGS_H�EXEC_TYPES_H�"exec/types.h"�
�ExpansionRom{
�er_Type;
�er_Product;
�er_Flags;
�er_Reserved03;
�er_Manufacturer;
�er_SerialNumber;
�er_InitDiagVec;
�er_Reserved0c;
�er_Reserved0d;
�er_Reserved0e;
�er_Reserved0f;
};
�ExpansionControl{
�ec_Interrupt;
�ec_Reserved11;
�ec_BaseAddress;
�ec_Shutup;
�ec_Reserved14;
�ec_Reserved15;
�ec_Reserved16;
�ec_Reserved17;
�ec_Reserved18;
�ec_Reserved19;
�ec_Reserved1a;
�ec_Reserved1b;
�ec_Reserved1c;
�ec_Reserved1d;
�ec_Reserved1e;
�ec_Reserved1f;
};�E_SLOTSIZE 0x10000�E_SLOTMASK 0xffff�E_SLOTSHIFT 16�E_EXPANSIONBASE 0xe80000�E_EXPANSIONSIZE 0x80000�E_EXPANSIONSLOTS 8�E_MEMORYBASE 0x200000�E_MEMORYSIZE 0x800000�E_MEMORYSLOTS 128�ERT_TYPEMASK 192�ERT_TYPEBIT 6�ERT_TYPESIZE 2�ERT_NEWBOARD 192�ERT_MEMMASK 7�ERT_MEMBIT 0�ERT_MEMSIZE 3�ERTB_CHAINEDCONFIG 3�ERTB_DIAGVALID 4�ERTB_MEMLIST 5�ERTF_CHAINEDCONFIG (1<<3)�ERTF_DIAGVALID (1<<4)�ERTF_MEMLIST (1<<5)�ERFB_MEMSPACE 7�ERFB_NOSHUTUP 6�ERFF_MEMSPACE (1<<7)�ERFF_NOSHUTUP (1<<6)�ERT_MEMNEEDED(t) \
(((t)&ERT_MEMMASK)?0x10000L<<(((t)&ERT_MEMMASK)-1):0x800000)�ERT_SLOTSNEEDED(t) \
(((t)&ERT_MEMMASK)?1L<<(((t)&ERT_MEMMASK)-1):128)�ECIB_INTENA 1�ECIB_RESET 3�ECIB_INT2PEND 4�ECIB_INT6PEND 5�ECIB_INT7PEND 6�ECIB_INTERRUPTING 7�ECIF_INTENA (1<<1)�ECIF_RESET (1<<3)�ECIF_INT2PEND (1<<4)�ECIF_INT6PEND (1<<5)�ECIF_INT7PEND (1<<6)�ECIF_INTERRUPTING (1<<7)�EC_MEMADDR(slot) (((�)slot)<<(E_SLOTSHIFT))�EROFFSET(er) ((�)&((�ExpansionRom*)0)->er)�ECOFFSET(ec) \
(sizeof(�ExpansionRom)+((�)&((�ExpansionControl*)0)->ec))
�DiagArea{
�da_Config;
�da_Flags;
�da_Size;
�da_DiagPoint;
�da_BootPoint;
�da_Name;
�da_Reserved01;
�da_Reserved02;
};�DAC_BUSWIDTH 192�DAC_NIBBLEWIDE 0�DAC_BYTEWIDE 64�DAC_WORDWIDE 128�DAC_BOOTTIME 48�DAC_NEVER 0�DAC_CONFIGTIME 16�DAC_BINDTIME 32�