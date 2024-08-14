//************************************************************************
//*                                                                      *
//*   CD-ROM Hardware Include File                                       *
//*                                                                      *
//*       CDGS Hardware register definitions                             *
//*                                                                      *
//************************************************************************


#define CDHARDWARE      0xB80000  

#define CDCHIPVERSION   0x0000    
#define CDSTATUS        0x0004    
#define CDINT2ENABLE    0x0008    
#define INTB_SUBCODE    31       
#define INTB_DRIVEXMIT  30       
#define INTB_DRIVERECV  29       
#define INTB_RXDMADONE  28       
#define INTB_TXDMADONE  27       
#define INTB_PBX        26       
#define INTB_OVERFLOW   25       
#define INTF_SUBCODE    0x80000000
#define INTF_DRIVEXMIT  0x40000000
#define INTF_DRIVERECV  0x20000000
#define INTF_RXDMADONE  0x10000000
#define INTF_TXDMADONE  0x08000000
#define INTF_PBX        0x04000000
#define INTF_OVERFLOW   0x02000000


#define CDROMHIGH       0x0010    
#define CDCOMHIGH       0x0014    
#define CDSUBINX        0x0018    
#define CDCOMTXINX      0x0019    
#define CDCOMRXINX      0x001A    
#define CDCOMTXCMP      0x001D    
#define CDCOMRXCMP      0x001F    

#define CDPBX           0x0020    
#define CDCONFIG        0x0024    
#define CB_SUBCODE      31       
#define CB_CDCOMTX      30
#define CB_CDCOMRX      29
#define CB_DBLCAS       28
#define CB_PBX          27       
#define CB_CDROM        26       
#define CB_DATA         25       
#define CB_2500Q        24       
#define CB_PALNTSC      23       
#define CF_SUBCODE      0x80000000
#define CF_CDCOMTX      0x40000000
#define CF_CDCOMRX      0x20000000
#define CF_DBLCAS       0x10000000
#define CF_PBX          0x08000000
#define CF_CDROM        0x04000000
#define CF_DATA         0x02000000
#define CF_2500Q        0x01000000
#define CF_PALNTSC      0x00800000
#define CDCOMPORT       0x0028    
#define CDPORTDAT       0x002C    
#define CDPORTDIR       0x002E    

#define CIAAPRA         0xBFE001  
#define MUTEB           0        
#define MUTEF           0x01
