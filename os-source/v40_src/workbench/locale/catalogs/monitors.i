	IFND LOCALESTR_MONITORS_I
LOCALESTR_MONITORS_I	SET	1


;-----------------------------------------------------------------------------


* This file was created automatically by CatComp.
* Do NOT edit by hand!
*


	IFND EXEC_TYPES_I
	INCLUDE 'exec/types.i'
	ENDC


;-----------------------------------------------------------------------------



	IFD DO_NTSC_MONITOR
MSG_NTSC_LORES EQU 69632
MSG_NTSC_HIRES EQU 102400
MSG_NTSC_SUPERHIRES EQU 102432
MSG_NTSC_LORES_LACED EQU 69636
MSG_NTSC_HIRES_LACED EQU 102404
MSG_NTSC_SUPERHIRES_LACED EQU 102436
	ENDC ; DO_NTSC_MONITOR

	IFD DO_PAL_MONITOR
MSG_PAL_LORES EQU 135168
MSG_PAL_HIRES EQU 167936
MSG_PAL_SUPERHIRES EQU 167968
MSG_PAL_LORES_LACED EQU 135172
MSG_PAL_HIRES_LACED EQU 167940
MSG_PAL_SUPERHIRES_LACED EQU 167972
	ENDC ; DO_PAL_MONITOR

	IFD DO_MULTISCAN_MONITOR
MSG_VGA_EXTRALORES EQU 200708
MSG_VGA_EXTRALORES_LACED EQU 200709
MSG_VGA_LORES EQU 233476
MSG_VGA_LORES_LACED EQU 233477
MSG_VGA_PRODUCTIVITY EQU 233508
MSG_VGA_PRODUCTIVITY_LACED EQU 233509
	ENDC ; DO_MULTISCAN_MONITOR

	IFD DO_A2024_MONITOR
MSG_A2024_10HZ EQU 266240
MSG_A2024_15HZ EQU 299008
	ENDC ; DO_A2024_MONITOR

	IFD DO_EURO72_MONITOR
MSG_EURO_72HZ_PRODUCT EQU 430116
MSG_EURO_72HZ_PRODUCT_LACED EQU 430117
	ENDC ; DO_EURO72_MONITOR

	IFD DO_EURO36_MONITOR
MSG_EURO_36HZ_LORES EQU 462848
MSG_EURO_36HZ_HIRES EQU 495616
MSG_EURO_36HZ_SUPERHIRES EQU 495648
MSG_EURO_36HZ_LORES_LACED EQU 462852
MSG_EURO_36HZ_HIRES_LACED EQU 495620
MSG_EURO_36HZ_SUPERHIRES_LACED EQU 495652
	ENDC ; DO_EURO36_MONITOR

	IFD DO_SUPER72_MONITOR
MSG_SUPER72_LORES EQU 528384
MSG_SUPER72_HIRES EQU 561152
MSG_SUPER72_SUPERHIRES EQU 561184
MSG_SUPER72_LORES_LACED EQU 528388
MSG_SUPER72_HIRES_LACED EQU 561156
MSG_SUPER72_SUPERHIRES_LACED EQU 561188
	ENDC ; DO_SUPER72_MONITOR

	IFD DO_DOUBLE_NTSC_MONITOR
MSG_DNTSC_LORES EQU 593920
MSG_DNTSC_LORES_FF EQU 593924
MSG_DNTSC_HIRES EQU 626688
MSG_DNTSC_HIRES_FF EQU 626692
MSG_DNTSC_LORES_LACED EQU 593925
MSG_DNTSC_HIRES_LACED EQU 626693
	ENDC ; DO_DOUBLE_NTSC_MONITOR

	IFD DO_DOUBLE_PAL_MONITOR
MSG_DPAL_LORES EQU 659456
MSG_DPAL_LORES_FF EQU 659460
MSG_DPAL_HIRES EQU 692224
MSG_DPAL_HIRES_FF EQU 692228
MSG_DPAL_LORES_LACED EQU 659461
MSG_DPAL_HIRES_LACED EQU 692229
	ENDC ; DO_DOUBLE_PAL_MONITOR


;-----------------------------------------------------------------------------


	IFD STRINGARRAY

   STRUCTURE AppString,0
	LONG   as_ID
	APTR as_Str
   LABEL AppString_SIZEOF


	IFD DO_NTSC_MONITOR
MSG_NTSC_LORES_STR: DC.B 'NTSC:Low Res',0
MSG_NTSC_HIRES_STR: DC.B 'NTSC:High Res',0
MSG_NTSC_SUPERHIRES_STR: DC.B 'NTSC:Super-High Res',0
MSG_NTSC_LORES_LACED_STR: DC.B 'NTSC:Low Res Laced',0
MSG_NTSC_HIRES_LACED_STR: DC.B 'NTSC:High Res Laced',0
MSG_NTSC_SUPERHIRES_LACED_STR: DC.B 'NTSC:Super-High Res Laced',0
	ENDC ; DO_NTSC_MONITOR

	IFD DO_PAL_MONITOR
MSG_PAL_LORES_STR: DC.B 'PAL:Low Res',0
MSG_PAL_HIRES_STR: DC.B 'PAL:High Res',0
MSG_PAL_SUPERHIRES_STR: DC.B 'PAL:Super-High Res',0
MSG_PAL_LORES_LACED_STR: DC.B 'PAL:Low Res Laced',0
MSG_PAL_HIRES_LACED_STR: DC.B 'PAL:High Res Laced',0
MSG_PAL_SUPERHIRES_LACED_STR: DC.B 'PAL:Super-High Res Laced',0
	ENDC ; DO_PAL_MONITOR

	IFD DO_MULTISCAN_MONITOR
MSG_VGA_EXTRALORES_STR: DC.B 'MULTISCAN:Extra-Low Res',0
MSG_VGA_EXTRALORES_LACED_STR: DC.B 'MULTISCAN:Extra-Low Res Laced',0
MSG_VGA_LORES_STR: DC.B 'MULTISCAN:Low Res',0
MSG_VGA_LORES_LACED_STR: DC.B 'MULTISCAN:Low Res Laced',0
MSG_VGA_PRODUCTIVITY_STR: DC.B 'MULTISCAN:Productivity',0
MSG_VGA_PRODUCTIVITY_LACED_STR: DC.B 'MULTISCAN:Productivity Laced',0
	ENDC ; DO_MULTISCAN_MONITOR

	IFD DO_A2024_MONITOR
MSG_A2024_10HZ_STR: DC.B 'A2024:10Hz',0
MSG_A2024_15HZ_STR: DC.B 'A2024:15Hz',0
	ENDC ; DO_A2024_MONITOR

	IFD DO_EURO72_MONITOR
MSG_EURO_72HZ_PRODUCT_STR: DC.B 'EURO:72Hz Productivity',0
MSG_EURO_72HZ_PRODUCT_LACED_STR: DC.B 'EURO:72Hz Productivity Laced',0
	ENDC ; DO_EURO72_MONITOR

	IFD DO_EURO36_MONITOR
MSG_EURO_36HZ_LORES_STR: DC.B 'EURO:36Hz Low Res',0
MSG_EURO_36HZ_HIRES_STR: DC.B 'EURO:36Hz High Res',0
MSG_EURO_36HZ_SUPERHIRES_STR: DC.B 'EURO:36Hz Super-High Res',0
MSG_EURO_36HZ_LORES_LACED_STR: DC.B 'EURO:36Hz Low Res Laced',0
MSG_EURO_36HZ_HIRES_LACED_STR: DC.B 'EURO:36Hz High Res Laced',0
MSG_EURO_36HZ_SUPERHIRES_LACED_STR: DC.B 'EURO:36Hz Super-High Res Laced',0
	ENDC ; DO_EURO36_MONITOR

	IFD DO_SUPER72_MONITOR
MSG_SUPER72_LORES_STR: DC.B 'SUPER72:Low Res',0
MSG_SUPER72_HIRES_STR: DC.B 'SUPER72:High Res',0
MSG_SUPER72_SUPERHIRES_STR: DC.B 'SUPER72:Super-High Res',0
MSG_SUPER72_LORES_LACED_STR: DC.B 'SUPER72:Low Res Laced',0
MSG_SUPER72_HIRES_LACED_STR: DC.B 'SUPER72:High Res Laced',0
MSG_SUPER72_SUPERHIRES_LACED_STR: DC.B 'SUPER72:Super-High Res Laced',0
	ENDC ; DO_SUPER72_MONITOR

	IFD DO_DOUBLE_NTSC_MONITOR
MSG_DNTSC_LORES_STR: DC.B 'DBLNTSC:Low Res',0
MSG_DNTSC_LORES_FF_STR: DC.B 'DBLNTSC:Low Res No Flicker',0
MSG_DNTSC_HIRES_STR: DC.B 'DBLNTSC:High Res',0
MSG_DNTSC_HIRES_FF_STR: DC.B 'DBLNTSC:High Res No Flicker',0
MSG_DNTSC_LORES_LACED_STR: DC.B 'DBLNTSC:Low Res Laced',0
MSG_DNTSC_HIRES_LACED_STR: DC.B 'DBLNTSC:High Res Laced',0
	ENDC ; DO_DOUBLE_NTSC_MONITOR

	IFD DO_DOUBLE_PAL_MONITOR
MSG_DPAL_LORES_STR: DC.B 'DBLPAL:Low Res',0
MSG_DPAL_LORES_FF_STR: DC.B 'DBLPAL:Low Res No Flicker',0
MSG_DPAL_HIRES_STR: DC.B 'DBLPAL:High Res',0
MSG_DPAL_HIRES_FF_STR: DC.B 'DBLPAL:High Res No Flicker',0
MSG_DPAL_LORES_LACED_STR: DC.B 'DBLPAL:Low Res Laced',0
MSG_DPAL_HIRES_LACED_STR: DC.B 'DBLPAL:High Res Laced',0
	ENDC ; DO_DOUBLE_PAL_MONITOR

	CNOP 0,4


AppString:

	IFD DO_NTSC_MONITOR
AS0:	DC.L MSG_NTSC_LORES,MSG_NTSC_LORES_STR
AS1:	DC.L MSG_NTSC_HIRES,MSG_NTSC_HIRES_STR
AS2:	DC.L MSG_NTSC_SUPERHIRES,MSG_NTSC_SUPERHIRES_STR
AS3:	DC.L MSG_NTSC_LORES_LACED,MSG_NTSC_LORES_LACED_STR
AS4:	DC.L MSG_NTSC_HIRES_LACED,MSG_NTSC_HIRES_LACED_STR
AS5:	DC.L MSG_NTSC_SUPERHIRES_LACED,MSG_NTSC_SUPERHIRES_LACED_STR
	ENDC ; DO_NTSC_MONITOR

	IFD DO_PAL_MONITOR
AS6:	DC.L MSG_PAL_LORES,MSG_PAL_LORES_STR
AS7:	DC.L MSG_PAL_HIRES,MSG_PAL_HIRES_STR
AS8:	DC.L MSG_PAL_SUPERHIRES,MSG_PAL_SUPERHIRES_STR
AS9:	DC.L MSG_PAL_LORES_LACED,MSG_PAL_LORES_LACED_STR
AS10:	DC.L MSG_PAL_HIRES_LACED,MSG_PAL_HIRES_LACED_STR
AS11:	DC.L MSG_PAL_SUPERHIRES_LACED,MSG_PAL_SUPERHIRES_LACED_STR
	ENDC ; DO_PAL_MONITOR

	IFD DO_MULTISCAN_MONITOR
AS12:	DC.L MSG_VGA_EXTRALORES,MSG_VGA_EXTRALORES_STR
AS13:	DC.L MSG_VGA_EXTRALORES_LACED,MSG_VGA_EXTRALORES_LACED_STR
AS14:	DC.L MSG_VGA_LORES,MSG_VGA_LORES_STR
AS15:	DC.L MSG_VGA_LORES_LACED,MSG_VGA_LORES_LACED_STR
AS16:	DC.L MSG_VGA_PRODUCTIVITY,MSG_VGA_PRODUCTIVITY_STR
AS17:	DC.L MSG_VGA_PRODUCTIVITY_LACED,MSG_VGA_PRODUCTIVITY_LACED_STR
	ENDC ; DO_MULTISCAN_MONITOR

	IFD DO_A2024_MONITOR
AS18:	DC.L MSG_A2024_10HZ,MSG_A2024_10HZ_STR
AS19:	DC.L MSG_A2024_15HZ,MSG_A2024_15HZ_STR
	ENDC ; DO_A2024_MONITOR

	IFD DO_EURO72_MONITOR
AS20:	DC.L MSG_EURO_72HZ_PRODUCT,MSG_EURO_72HZ_PRODUCT_STR
AS21:	DC.L MSG_EURO_72HZ_PRODUCT_LACED,MSG_EURO_72HZ_PRODUCT_LACED_STR
	ENDC ; DO_EURO72_MONITOR

	IFD DO_EURO36_MONITOR
AS22:	DC.L MSG_EURO_36HZ_LORES,MSG_EURO_36HZ_LORES_STR
AS23:	DC.L MSG_EURO_36HZ_HIRES,MSG_EURO_36HZ_HIRES_STR
AS24:	DC.L MSG_EURO_36HZ_SUPERHIRES,MSG_EURO_36HZ_SUPERHIRES_STR
AS25:	DC.L MSG_EURO_36HZ_LORES_LACED,MSG_EURO_36HZ_LORES_LACED_STR
AS26:	DC.L MSG_EURO_36HZ_HIRES_LACED,MSG_EURO_36HZ_HIRES_LACED_STR
AS27:	DC.L MSG_EURO_36HZ_SUPERHIRES_LACED,MSG_EURO_36HZ_SUPERHIRES_LACED_STR
	ENDC ; DO_EURO36_MONITOR

	IFD DO_SUPER72_MONITOR
AS28:	DC.L MSG_SUPER72_LORES,MSG_SUPER72_LORES_STR
AS29:	DC.L MSG_SUPER72_HIRES,MSG_SUPER72_HIRES_STR
AS30:	DC.L MSG_SUPER72_SUPERHIRES,MSG_SUPER72_SUPERHIRES_STR
AS31:	DC.L MSG_SUPER72_LORES_LACED,MSG_SUPER72_LORES_LACED_STR
AS32:	DC.L MSG_SUPER72_HIRES_LACED,MSG_SUPER72_HIRES_LACED_STR
AS33:	DC.L MSG_SUPER72_SUPERHIRES_LACED,MSG_SUPER72_SUPERHIRES_LACED_STR
	ENDC ; DO_SUPER72_MONITOR

	IFD DO_DOUBLE_NTSC_MONITOR
AS34:	DC.L MSG_DNTSC_LORES,MSG_DNTSC_LORES_STR
AS35:	DC.L MSG_DNTSC_LORES_FF,MSG_DNTSC_LORES_FF_STR
AS36:	DC.L MSG_DNTSC_HIRES,MSG_DNTSC_HIRES_STR
AS37:	DC.L MSG_DNTSC_HIRES_FF,MSG_DNTSC_HIRES_FF_STR
AS38:	DC.L MSG_DNTSC_LORES_LACED,MSG_DNTSC_LORES_LACED_STR
AS39:	DC.L MSG_DNTSC_HIRES_LACED,MSG_DNTSC_HIRES_LACED_STR
	ENDC ; DO_DOUBLE_NTSC_MONITOR

	IFD DO_DOUBLE_PAL_MONITOR
AS40:	DC.L MSG_DPAL_LORES,MSG_DPAL_LORES_STR
AS41:	DC.L MSG_DPAL_LORES_FF,MSG_DPAL_LORES_FF_STR
AS42:	DC.L MSG_DPAL_HIRES,MSG_DPAL_HIRES_STR
AS43:	DC.L MSG_DPAL_HIRES_FF,MSG_DPAL_HIRES_FF_STR
AS44:	DC.L MSG_DPAL_LORES_LACED,MSG_DPAL_LORES_LACED_STR
AS45:	DC.L MSG_DPAL_HIRES_LACED,MSG_DPAL_HIRES_LACED_STR
	ENDC ; DO_DOUBLE_PAL_MONITOR


	ENDC ; STRINGARRAY


;-----------------------------------------------------------------------------


	ENDC ; LOCALESTR_MONITORS_I
