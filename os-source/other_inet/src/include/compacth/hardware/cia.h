��HARDWARE_CIA_H�HARDWARE_CIA_H
�CIA{
�ciapra;
�pad0[0xff];
�ciaprb;
�pad1[0xff];
�ciaddra;
�pad2[0xff];
�ciaddrb;
�pad3[0xff];
�ciatalo;
�pad4[0xff];
�ciatahi;
�pad5[0xff];
�ciatblo;
�pad6[0xff];
�ciatbhi;
�pad7[0xff];
�ciatodlow;
�pad8[0xff];
�ciatodmid;
�pad9[0xff];
�ciatodhi;
�pad10[0xff];
�unusedreg;
�pad11[0xff];
�ciasdr;
�pad12[0xff];
�ciaicr;
�pad13[0xff];
�ciacra;
�pad14[0xff];
�ciacrb;
};�CIAICRB_TA 0�CIAICRB_TB 1�CIAICRB_ALRM 2�CIAICRB_SP 3�CIAICRB_FLG 4�CIAICRB_IR 7�CIAICRB_SETCLR 7�CIACRAB_START 0�CIACRAB_PBON 1�CIACRAB_OUTMODE 2�CIACRAB_RUNMODE 3�CIACRAB_LOAD 4�CIACRAB_INMODE 5�CIACRAB_SPMODE 6�CIACRAB_TODIN 7�CIACRBB_START 0�CIACRBB_PBON 1�CIACRBB_OUTMODE 2�CIACRBB_RUNMODE 3�CIACRBB_LOAD 4�CIACRBB_INMODE0 5�CIACRBB_INMODE1 6�CIACRBB_ALARM 7�CIAICRF_TA (1<<CIAICRB_TA)�CIAICRF_TB (1<<CIAICRB_TB)�CIAICRF_ALRM (1<<CIAICRB_ALRM)�CIAICRF_SP (1<<CIAICRB_SP)�CIAICRF_FLG (1<<CIAICRB_FLG)�CIAICRF_IR (1<<CIAICRB_IR)�CIAICRF_SETCLR (1<<CIAICRB_SETCLR)�CIACRAF_START (1<<CIACRAB_START)�CIACRAF_PBON (1<<CIACRAB_PBON)�CIACRAF_OUTMODE (1<<CIACRAB_OUTMODE)�CIACRAF_RUNMODE (1<<CIACRAB_RUNMODE)�CIACRAF_LOAD (1<<CIACRAB_LOAD)�CIACRAF_INMODE (1<<CIACRAB_INMODE)�CIACRAF_SPMODE (1<<CIACRAB_SPMODE)�CIACRAF_TODIN (1<<CIACRAB_TODIN)�CIACRBF_START (1<<CIACRBB_START)�CIACRBF_PBON (1<<CIACRBB_PBON)�CIACRBF_OUTMODE (1<<CIACRBB_OUTMODE)�CIACRBF_RUNMODE (1<<CIACRBB_RUNMODE)�CIACRBF_LOAD (1<<CIACRBB_LOAD)�CIACRBF_INMODE0 (1<<CIACRBB_INMODE0)�CIACRBF_INMODE1 (1<<CIACRBB_INMODE1)�CIACRBF_ALARM (1<<CIACRBB_ALARM)�CIACRBF_IN_PHI2 0�CIACRBF_IN_CNT (CIACRBF_INMODE0)�CIACRBF_IN_TA (CIACRBF_INMODE1)�CIACRBF_IN_CNT_TA (CIACRBF_INMODE0|CIACRBF_INMODE1)�CIAB_GAMEPORT1 (7)�CIAB_GAMEPORT0 (6)�CIAB_DSKRDY (5)�CIAB_DSKTRACK0 (4)�CIAB_DSKPROT (3)�CIAB_DSKCHANGE (2)�CIAB_LED (1)�CIAB_OVERLAY (0)�CIAB_COMDTR (7)�CIAB_COMRTS (6)�CIAB_COMCD (5)�CIAB_COMCTS (4)�CIAB_COMDSR (3)�CIAB_PRTRSEL (2)�CIAB_PRTRPOUT (1)�CIAB_PRTRBUSY (0)�CIAB_DSKMOTOR (7)�CIAB_DSKSEL3 (6)�CIAB_DSKSEL2 (5)�CIAB_DSKSEL1 (4)�CIAB_DSKSEL0 (3)�CIAB_DSKSIDE (2)�CIAB_DSKDIREC (1)�CIAB_DSKSTEP (0)�CIAF_GAMEPORT1 (1<<7)�CIAF_GAMEPORT0 (1<<6)�CIAF_DSKRDY (1<<5)�CIAF_DSKTRACK0 (1<<4)�CIAF_DSKPROT (1<<3)�CIAF_DSKCHANGE (1<<2)�CIAF_LED (1<<1)�CIAF_OVERLAY (1<<0)�CIAF_COMDTR (1<<7)�CIAF_COMRTS (1<<6)�CIAF_COMCD (1<<5)�CIAF_COMCTS (1<<4)�CIAF_COMDSR (1<<3)�CIAF_PRTRSEL (1<<2)�CIAF_PRTRPOUT (1<<1)�CIAF_PRTRBUSY (1<<0)�CIAF_DSKMOTOR (1<<7)�CIAF_DSKSEL3 (1<<6)�CIAF_DSKSEL2 (1<<5)�CIAF_DSKSEL1 (1<<4)�CIAF_DSKSEL0 (1<<3)�CIAF_DSKSIDE (1<<2)�CIAF_DSKDIREC (1<<1)�CIAF_DSKSTEP (1<<0)�