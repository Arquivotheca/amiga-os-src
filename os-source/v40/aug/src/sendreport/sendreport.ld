  �                �  Z  �  �H�~�$H$ I�    ,x G�  
tr  <   �`&�Q���)O
�)N
�B�
�p "<  0 N���C��p N���)@dfpd` �&n)k �
�J� �g  � �� 8�   �)@
x k �����"h ���� r )I
�Ё^�@��)@
�H�@@"<  N��:L�J�f <  �/ `  @)@
� S�ԁ�   S�Q����   S�� " �   Q���� "/`x)k :
xpR�Ѭ
xA� \N���A� \N���)@
�/ $@ * $g,ld @"(  N���)@
�" N���"*  g$<  �N���)@
�g
� @'h  � l
�/Hl
t h $)h 
�N�7�N�=�p ` / .l
�/  ,
�g @N�N�I6N�7�J�
�g*,ld",
�gN���",
�gN���,x N��|"l
�N���` ,
�g"l
�,x N��."ldN��b L�~Nudos.library   /I�    p (_Nu1.2 1.3 1.3.3 2.0 A2024 A2091.A590  A2065 A2090.A2090A  A2232 A2300 A2410 A2620 A2630 A3000 aa.chips  addmonitor  alink AmigaBasic  amiga.lib amigaguide  amigaterm amigavision arexx art appshell  as225 asl.library audio.device  aug.command autoconfig  autovecs  battclock battmem battcard  bindmonitor blink bootmenu  bolt  bridgeboard bru bullet  cdos.command  cdtv  cdtv.cr cia.resource  clipboard commodities compatible  con-handler console.device  creditcard  crossdos  custom.chips  datatypes debug.lib disk  diskfont  disk.resource documentation dos.command dos.library exec  expansion extras  filesysres  filesystem  fed fonts fountain  gadget.classes  gadtools  gameport  genlock graphics  handler hardware  hdbackup  hdtoolbox icon.library  iconedit  ide.device  iffparse  iprefs  includes  input.device  installer intuition kickstart keyboard  keymap  keymaps kickmenu  layers  locale.library  mathffp mathieee  mathieeedoub  mathieeesing  microemacs  multiview new.look  net narrator.device notepad pa.test parallel.device pcutils pipe-handler  port-handler  potgo.resource  preferences preferences.old printer.device  printer.driver  queue-handler ramdrive.device ram-handler ramlib  rawinput  resource  sana  sana2 script  scsi.device serial.device setpatch  shell speak-handler startup strap system.command  test.suite  tests timer.device  toolkit toolmaker tools trackdisk translator  translations  unix  util.command  utility.library wack  workbench eric  darren eric peter eric peter havemose spence chrisg havemose  steveb jesup andy bj dlarson gregm  jesup peter bryce hedley  havemose  daveh peter havemose chrisg spence raible bthomas lenthe  spence chrisg havemose vertex carolyn peter vertex jesup eric davidj  peter campbell cathy  mks ross peter  dlarson bj gregm  vertex  eric peter  darren mks  darren  walker@dev.sas.com  spence vertex dlarson gregm bj  havemose bill kodiak darren havemose davidj eric vertex carl andy hedley gail dong reichart rutgers!ucbvax!well!ewhac jerryh havemose havemose jerryh andy kaori jesup mikec jjszucs  darren andy jesup andy  eric darren jesup lenthe  darren havemose kodiak davidj dan izzy ross jesup vertex  steveb jesup darren steveb jesup  steveb  eric darren havemose kodiak davidj  davidj vertex peter vertex  spence chrisg steveb bart havemose  grr jesup kaori eric carolyn rutgers!ucbvax!well!ewhac  peter eric  vertex larry  peter darren  spence chrisg mks bart  andy havemose havemose peter chrisg spence joe mikec jjszucs janc givler lockhart mitchell jerryc dlarson jerryh peter  peter kodiak bill vertex davidj mitchell  mks peter carolyn jesup joe mikec chrisg darren davidj havemose spence peter  carolyn ricci eric vertex cbmehq!nymand johann ditto ag buck  eric carolyn vertex davidj   $VER: sendreport 39.3 (21.7.92)  ram:Report.txt   $VER: sendreport 39.3 (21.7.92)
Copyright (c) 1992 Commodore-Amiga, Inc.
All Rights Reserved
  Usage: sendreport [outfile] [help] (default outfile is ram:Report.txt)  Generates Bug Reports, Compatibility Reports, and Enhancement Requests

  Report creates/uses s:Report.sender,s:Report.config,s:Report.ks,Report.wb
  
PLEASE - DO NOT fill in the Subsystem field or any other restricted value
 fields by hand!  Only the REPORT program can validate these fields.

 Default output filename is ram:Report.txt (or you may specify a filename).

  Completed bug reports will be mailed to bugs@cbmvax, module owners and
 cc'ed to you.  To always cc others, use setenv CCBUGS <names>.  Note that
  the names must be separated by spaces, not commas.

  Default editor is TurboText.  You may specify a different editor using
 setenv EDITOR <editorname>.  Note that the editor command must NOT return
  until the document is saved (or canceled).

  Default mailer is Willey Langeveld's sendmail, using
 "inet:smtp/sendmail -h cbmvax".  You may specify a different mailer
  using setenv SENDMAIL <command>.
 Lastname,FirstName (no spaces) ->   # Refer: %s   Company Name (without Inc., Corp, etc.) ->  (%s   (AreaCode) Phone-Number ->  ,phone %s)
 Please enter your best electronic mail address.
If UUCP, just enter the address,  for example: jsmith@endor.COM
If other, enter address (SYSTEM), for example: jsmith (BIX)
  Enter electronic address (<RET> if none) ->   # Path: %s
 Developer Number ->   # ReferID: %s
  Model (a500,a1000,a2000,a2500,a3000)->  # Config: %s, Processor (68000,68020,68030,68040)->   %s, Agnus type (ECS,ECS-pal,old,old-pal) ->   A=%s, Denise type (ECS,old) ->  D=%s, Total Megs of CHIP ram ->   RAM=%smegC  Total Megs of FAST ram ->   /%smegF,  Number of floppies ->   TD=%s,  Hard disk controller (a2090,a2090a,a2091, etc) ->   HD=%s,  Other expansion cards->   %s
 
3rd Party Product Name->   # ProductName: %s 3rd Party Product Version->    (vers=%s)
 Manufacturer of Product (without Inc.,Corp., etc)->   # ProductInfo: %s
  
CBM Bug and Enhancement Report Generator
----------------------------------------
 1. System Software or Hardware Bug Report
 2. Third Party Product Compatibility
 3. Enhancement Request
  Enter 1-3:  123 
Enter a meaningful 1-line summary:
  
Classification:
 a. Crashes, Hangs, or Corrupts Data
 b. Behaves incorrectly
 c. Cosmetic problem
 Enter appropriate letter:   abc 
Classification:
 a. Crashes, Hangs, or Corrupts Data
 b. Behaves incorrectly
 c. Cosmetic problem
 w. Works correctly
 abcw  bug compat  enhancement System Bug Report Compatibility Report  Enhancement Request 
-------------------------- Valid Subsystems --------------------------------
  * NOTE * If you don't see specific command name, use cdos.command for 2.0 C:
 commands, util.command for Utilities/, sys.command for System/, tools for
  Tools/, and just hit return if you are not sure of correct subsystem.
  Please categorize all include and autodoc bugs under their OS subsystem name.
  -----------------------------------------------------------------------------
  %-16s%-16s%-15s%-15s%-15s
  intuition.library %s
%s
    help  Not enough memory
  ENV:user  unknown bugs@cbmvax ENV:CCBUGS    ENV:sendmail  inet:smtp/sendmail -h cbmvax  ENV:Editor  TURBOTEXT:TTX WAIT  Filename for output:  Output file %s already exists.  Overwrite <y or n> ?  yn  
Can't open %s for output
  %s selected.
 Enter subsystem, or <RET> if not sure, or ?<RET> to see list:   Press RETURN for More...  Enter correct subsystem from list,
   or <RET> if not sure, or ?<RET> to review list:   %s is not a listed subsystem
 bugs@cbmvax %s  From: %s
 Subject: %s
  To: %s
Cc: %s
  X-Mailer: Report [version %d.%d (%s)]

 21.7.92 # Subject: %s
# Type: %s
 e ok  s:Report.ks s:Report.wb 
Enter Kickstart version.rev (default %s):  
Enter Kickstart version.rev:   # Release: KS=%s, Enter Workbench version.rev (default %s):   Enter Workbench version.rev:  WB=%s 
If bug location is a ROM library/device or 2.0 disk command/library/device,
 use the VERSION command to find the program's internal version.
  i.e. the output of VERSION commandname | name.library | name.device 0 0 unit
 Or enter ?<RET> if you want to query the version now on this machine.
  Enter program's internal version.rev (or ?, or <RET> if n.a.):  ? Enter path/command, name.library, or name.device 0 0 unit:  version %s  ,Program=%s
  
 date > ram:bdate  ram:bdate inserting date...
  # Subsystem: %s
  # Severity: %s
 # Date: %s
 s:Report.sender 
Can't create %s
 
Creating %s for use in reports
  inserting %s...
  s:Report.config 
### ENHANCEMENT REQUEST:


  
### BRIEF BUG DESCRIPTION:


  
### BUG GENERATION PROCEDURE OR EXAMPLE:


  
### IF THIS WORKS DIFFERENTLY ON OTHER VERSIONS OR HARDWARE, EXPLAIN:
 
### WHAT DEVELOPER IS DOING IN THAT AREA OF SW OR HW:


 
### RELATED PROBLEMS:


 which >%s %s  t:Report.e  not found
  
Can't find editor "%s"
  Add  SETENV EDITOR pathto/youreditor  to your user startup sequence
  Meanwhile, enter the path/name of your editor:  unable to examine %s!!!
  %s %s %s %s %s %s s:SendReport.log  %s

   o %s : %s

  
Thank you.  Completed report is "%s"
  and has been sent to %s and %s
 unable to send mail
  no change to %s
  Requires 2.x or beyond
 abort...
 version.library 
(Note - This machine is running  KS=%d.%d  WB=%d.%d)
  %d.%d /I�    p (_NuNU��H�7:I�    .- &m A�  �C���p"�Q���C��p%,x N���)@@g `p )@
�)@
�)@
�B�
�r A
�AJ�gr��mr��f( k p?�f/,�/,�Hz��N��B�Hz��a ,O� p��o^ � s�C����fLJf�fF/,�/,�Hz��N�LO� |  �A��J� g �A��/0 N�'�XOR�`�B�Hz�da �PO <  @ rHA,x N��:)@
�J�fHx Hz�Da �PO <  hrHA,x N��:)@8J�fHx Hz�a vPO l
��� )H
�C� )I
�M� )N
�p��f)k �C���"	$<  �,ldN���)@
�g2 l8��L" $v<N��� l8��B(L+@��",
�,ldN���B�
�` l8��LC����f� l8"H��hM�L�f� l8���C����f�A���"$<  �,ldN���)@
�gP" $,
�vdփN���+@��",
�N���B�
� l
�����B l8��hHz�`/N�# l8��h.�
�/N�"�O� A��B"$<  �,ldN���)@
�g6 l8�� " $&<  ,N��� l8��B( +@��",
�,ldN���B�
�` l8�� C����f�A��"$<  �,ldN���)@
�g," A���$vPN���A�����B+@��",
�,ldN���B�
�`A���C���"�"�"�"�2�BJ��f$p / Hz��/ Hm�<a �O� J�g�A��<)H�`�",�$<  �,ldN���)@
�gJ" N���B�
�/,�Hz��/,
�N�#JHz��/,
�B�/,
�a bO�  l
�    ry�g
��)H�` �v",�$<  �,ldN���)@
�J�f/,�Hz�zN�JPOB��` �FHl$/,
�a �.�
�N�)�S�)@d�A�t.� Hz�`N�Hl4/,
�a �O�  l8���"l
��f� l8��x ,d�C�h"q �f�p��dg �|  �A�  J� gR�`�+F��p+@�� l
�Bp / Hz��/ /,
�a nO� ` �p"-��$-��&ցS�+@�� N�'�r +@�� -��+A��"-��N�'�ԀS�/@ $ "/ $N�'�Bm��z /,�+@��N�#�XOB��� -����l Jm��f  -��"-��N�'\"-��N�'T"-��$��+@�� -�찂m�� B���+@�� -������l  �Jm��f  �| ����lD -��Э��/@ $ -��"N�'"/ $Ҁ�A�  $p  
g
 �+��` �A���+��R�`�/-��/-��/-��/-��/-��/,�N�"�O� R�����f;| ��R���` �lJm��f.Hz��N��XOS�	mp  l	 R�	 `
Hl�N��XOr
��f�R���` ��/,�N�"t.��N�"l.��N�"d.��N�"\.��N�"T.��Hz�>N�� l
�Bp .�Hz�/ /,
�a �O�  l
�r?�g �0J gr `r;A��J g6|  �A�  J� g& �A�  /0 /,
�N��POJ�f;| ��`R�`�Jm��f/,
�Hz�rN�fPO` �� l8���"l
��f�z B�< �A�HJ� gLJ�<fF �A�H/0 /,
�N�POJ�f( �A�H��)H<"l8���/( Hz�$/	N��O� R�`�p��df | l8��L/Hz�/,
�N�z l
�Jf�S���
�&",
�$,
�,ldN��� l8���.�Hz��/,
�N�D l
�Jf�S���
�&",
�$,
�,ldN��� l8"H���M�h.�/	Hz�/,
�N� l
�Jf�S���
�&",
�$,
�,ldN���Hz�Hx Hx 'Hz�/,
�N�� l
�Jf�S���
�&",
�$,
�,ldN��� l8"H���M�x.�/	Hz�|/,
�N��O� @ l
�Jf�S���
�&",
�$,
�,ldN���z | p��l: �A����//,
�a L l
��� �.�
�C��/1/N�2O� ڀR�`�",
�$,
�&,ldN���p��dfA��"l
��f�`&p��dfHlT/,
�a �PO`HlD/,
�a �PO l
�@��rw�f
C���,H�f� l8���"l
��f�p��dg �A��"$<  �,ldN���)@
�g*" A�
�$v$N���A�
�B0 +@��",
�,ldN���B�
�A��n"$<  �N���)@
�g*" A�$v$N���A�B0 +@��",
�,ldN���B�
�a �z J,
�gHl
�Hz�*/,
�N�O� `Hz�D/,
�N��POp / /,
�/ /,
�a 
O� J,
�g l
�J fC�
�,H�f�`@A���"$<  �,ldN���)@
�g& l
�Jf�S���
�" &$,
�N���",
�N���B�
�/,
�Hz��/,
�N�pO� ڀJ,gHlHz��/,
�N�TO� `Hz��/,
�N�BPOp / /,
�/ /,
�a 
XO� J,g l
�J fA�"l
��f�`@A��"$<  �,ldN���)@
�g& l
�Jf�S���
�" &$,
�N���",
�N���B�
� l
���/,
�Hz�/N��O� ڀ l
�BJ�df  �Hz�jN��XOHz�N��XOHz��N�vXOHz�*N�lXOp / Hz�d/ /,
�a 	�O�  l
�C����f>J f�f8p / Hz�|/ /,
�a 	j.�
�Hz�/,
�N�0O� ",
�t &,ldN��"`� l
�Jg l
���/,
�Hz�~/N��O� ڀ` l
���Hz�t/N��POڀ l8��"l
��f�A��X"t &,ldN��"A��X"$<  �N���)@
�gDHz�LN��XO l8��h$",
�&<   ,ldN���+@��",
�N���B�
� l8"H����B)gp��dg � l8��L/Hz��/,
�N�F l
�Jf�S���
�&",
�$,
�,ldN��� l8���.�Hz�/,
�N� l
�Jf�S���
�&",
�$,
�,ldN��� l8"H���M�h.�/	Hz�~/,
�N�� l
�Jf�S���
�&",
�$,
�,ldN���Hz�Hx Hx 'Hz�V/,
�N�� l
�Jf�S���
�&",
�$,
�,ldN��� l8"H���M�x.�/	Hz�H/,
�N�\ l
�Jf�S���
�&",
�$,
�,ldN��� l8���.�Hz��/,
�N�&O� H l
�Jf�S���
�&",
�$,
�,ldN��� l8���/Hz��/,
�N��O�  l
�Jf�S���
�&",
�$,
�,ldN���p��dg( l8"H��M� NJf�S���$	&",
�,ldN��� l8��h/Hz�f/,
�N��O�  l
�Jf�S���
�&",
�$,
�,ldN���A��B"$<  �N���)@
�J�f  �A��("$<  �N���)@
�J�fHz�Hz�N�	vHx Hz�a O� Hz��Hz�N�	ZPOz | p��l: �A���//,
�a � l
��� �.�
�C�/1/N��O� ڀR�`�",
�$,
�&,ldN���",
�N���A��"$<  �N���)@
�Hz�tHz��N��PO",
�$,
�&<   ,ldN���+@��g& ",
�$,
�N���`�",
�,ldN���B�
�A��"$<  �N���)@
�J�f  �A��f"$<  �N���)@
�J�fHz�NHz�N�^Hx Hz�a  O� Hz�2Hz��N�BPOz | p	��l: �A�d��//,
�a � l
��� �.�
�C�d/1/N��O� ڀR�`�",
�$,
�&,ldN���",
�N���A���"$<  �N���)@
�Hz�Hz�N��PO",
�$,
�&<   ,ldN���+@��g& ",
�$,
�N���`�",
�,ldN���p )@
�* p��df l
���Hz�f/N�POڀ`Dpw�-��g( l
���Hz�f/N��ڀ l
���Hz�t/N��O� ڀ l
���Hz�/N��POڀp��dfpw�-��g l
���Hz�/N��POڀp��dgpw�-��g l
���Hz��/N��POڀ",
�$,
�&,ldN���",
�N���B�
�Hm��Hz�Hz�/,
�N�NO� ",
�t &,ldN��"A��"$<  �N���)@
�g  �" $,
�&<   N���+@��",
�N���B�
� -�� l
�"H��r � l
�Jgr��ob���� 
C��H�fRJ f�fLHm��Hz�@N�Hz�RN��p .�Hz�/ Hm��a �O� -��J f �8A��C���"�"�"�"�2�B` � ",�t�,ldN��� l8!@g>" $,ldN���J�g l8"H�� �M� I"Np,x N��� l8"(,ldN���`/,�Hz�.N�tPO/,�Hm��Hz�6/,
�N�O� ,ldN���+@��N���+@��",
�A���$N���J�f �",�t�N��� l8!@g>" $,ldN���J�g l8"H�� �M� I"Np,x N��� l8"(,ldN���`/,�Hz�N��PO l8"H��M�"	$,ldN��J�o ( l8"H�� M��/N (M�h/// ,/,�/	Hz�b/,
�N�.O� ",
�A���$,ldN���J�f  �A��F"$<  �N���)@
�g  �" t vN��� l8��h/Hz�,/,
�N�� l
�Jf�S���
�&",
�$,
�,ldN��� l8"H���M��.�/	Hz��/,
�N��O�  l
�Jf�S���
�&",
�$,
�,ldN���",
�N���/,�Hz��N��PO l8"H���M�h//	Hz��N��O� `Hz��N�(XO`/,�Hz��N��POB�Hz��a "PO`Hx Hz��a POL�\�N]NuH� 8I�    &o $o /* /* //a  O� L� NuNU  H� :I�    &m $m  
g/
N�lXOp " ,x N���  ga  � - g/ N�H.�N��.� /a  2POJ�g�p " ,x N���  ga  H KJf�S��� L�\ N]NuH� 8I�    &o $o  
fp`Jg� f�p`p L� Nu/I�    Hz��N� Hx Hz�a  
O� (_NuNU  H�I�    .- a  /N��L����N]NuH� 
I�     ,
�g
" ,ldN��� ,
�g
" ,ldN��� ,
�g
" ,ldN���J�
�g"l
� <  @ ,x N��.J�8g"l8 <  h,x N��.L�P NuNU��H�?I�     l
� ( . G  �    fp`p "t҂ A<"t҂ A:;@��C��p ,x N���&@ g8+ ;k ��"KN��b`p ( ;@��p 0r 2t 4v 6-��//// Hz��N�,O� ,
�J fp 0r 2// Hz��Hl
�N��O� ,J fp 0r 2-��// Hz��HlN��O� L�X�N]NuNuNuH�0./ &o ,/  /N��XO$@ 
fp�`6*  gHx B�/N��O� ///* N��O� * J�
�gp�` L��Nu        paH�&o  KJf�S���,~ J�g.S�	*m l	"R�	" �r  `� r  Hl	/N�&PO" `�Hl	Hx��N�PO L��Nu      paNU��H�0&m | A� +H��Jg  �p%� f  �p @ dgP@ gW@g[@g`h m��$X+H��`N m��(+H��E���zJ�k r��A� ����S��S�`�B-��E���` m��(+H��/Hm��N�xPOE���/
N���XO܀` �nR�S�	*m l	"R�	" �r  ` �Rp Hl	/ N�.PO" ` �<R�S�	*m l	"R�	" �r  ` � p Hl	/ N��PO" ` �
Hl	Hx��N�� L����N]Nu0123456789ACDEF   NU��H�'0&m $m ~ | z p |  ��r +A��t�+B��A���@��@��+A��+A��+H��g,r  A  gWAgQAgUAf~`|`
z`| ��R�`�r0�fR�A��p*�f RX�+P��R�`Hm��/N�rPO��r.�f"R�p*�f RX�+P��R�`Hm��/N�HPO��rl�f
| ��R�`rh�fR�r  @��A Pg lQAg zA g SAg$A g SAg LWAg �UAg  �WAg N` �J-��g RX� ` RX� +@��l
rD���+A�� -��gr-`
Jgr+`r A��r ��r ��gR���R���/-��/-��N�
�PO+@�� -��J�jr+A�� -��"-��H� ��o4 m����/ //-��N��O� p -��"-�� m��`�S�d� -��+@��ѭ��A���+H��Jg 0|  ��` &J-��g RX� ` RX� +@��` �dJ-��g RX� ` RX� +@��J-��g m��� 0r+A��+H��/ /-��N�
PO+@��` �2 -��J�jp+@��| ��J-��g RX� ` RX� +@��J-��g m��� 0� xr+A��+H��/ /-��N�	�PO+@��- ��g ��Hm��N�`XO` �� RX�"P+I��fA� �+H�� m��Jf�S�����+H�� -��J�k&��o"+@��`p+@�� RX� @��B-��`p `  � -��"-����lt +B��`����Jg6S���mp  m��/ +H�� m N�XO`�S���mHp -��/  m N�XO`�S���mp -��/  m N�XO`�S���mp  m��/ +H�� m N�XO`� L��N]Nu  NU��H�0&m $m +m ��Jg4p%� f"�fR�`/Hm��/
a �O� +@��g$@`�p / N�XO`�L��N]NuNU��H�!2&m �    �l  �r �gr	�gr
�fR�`�g  �",��R��A����$Hr"�frR� K$�+H��r"�gPr*�f>R�p @ Eg@ 	g` m��� +H��` m��� 
+H��`
 m���+H��R�`� m���+H��`�R� m��B+H��` �N$�gr �gr	�g
r
�gR�`�Jf`B` �& ,�f l
�`A��)H�J�f|C�$M��,�,�,�,�<�"l
� i $Hx (/( Hl�N�4O� A��"$<  �,ldN���)@H)@Pr)AL)@X)AT�+@���,x N��� m��"@#h  �~ +@��`*,ldN���)@HN���)@PA� �"$<  �N���)@X~  @���D  @���L �  �TJ�	dgp ` <  � . B�	  @ )@	p)@	:  @ )@	6p)@	\  @ �)@	XA���)H
�/,�/,�N���B�N�^L�L���N]Nucon:10/10/320/80/ *                                 pa/&o J� g+  fp `4/,
pN�
:XO'@ '@ f
p)@`p�`'l
p p��� p '@ '@ &_Nu            NU��H�/.- &m (p1�� gp�` d+  V�D H�H�, J� f  �+  fvp '@ r���g 6/N��RXOJ�g�  p�` �  Jg + " D�'A ` + '@ S� m k R�  �r  ` r  //a �VPO"  ` �+  gXp���fp ` � @��Jg"p
��fp/ Hz�/+ +@��N��DO� * `p/ Hm��/+ +@��N��(O� * ~�`  ��  JgNp���gHT� r
��f" k R� � "+ J�k
// a ��POR�  k R�  � + J�j ` ~� + �� +@��gr+  gRHx B�/+ N�\O� +@��Jg8S���m2B�/-��/+ N�<Hx Hm��/+ N�O� J�
�f
-��r�g�/-��/+ /+ N��LO� * `z p���f�  `����g�  Jg"+ $D�'B `+  gr 'A `"+ 'A  k 'H ��g*S� m k R�  �r  ` r  //a ��PO" p0�� gp�`p���fp ` L��N]Nu
    H�&o +  V�D H�H�. p0�� g
B� p�` F+  g+  g/Hx��N��.POJ� f4B� +  gp'@ A�  'H `v/N���XOJ�gj�  p�`  �JgZT�  + nP k R� |  r��gr��g$`.S� m k R� p `  �/a �<XO`  ��  p�`  � `  �+  fN�   /+ /+ /+ N�RO� * J�j�  J�f�  J�oJg
 D�'@ `'E  k 'H p2�� gJgp�'@ `p '@ p�`S� m k R� p `/a ��XOL��NuH� ./  ,�S�, JFk0 H��A�D*0 Jg f H��A�D/0N��XOSF`�/N��$XOL� �Nu        paH� 0&o $Kg$r  A�	m0  g
r  t ��`r  �R�`� L�Nu        paNU��H�0&m $m .-  JJf�S���, KJf�S��� "K��+I����c,  J`�S�d� m��B0h  L��N]Nu o "o p r   am
  zn    am
 zn  ��fJf�Nu  "o  o  Jf�S��f�Nu  /&o p A�	m0  gR�`� &_Nu /  o NU��"Or
N�A 0�J�f� 	��f�B��N]Nu   /  o NU��"O" A A 0��f� 	��f�B��N]Nu  0123456789abcdef /  o C� 2 A ���f� 	"X�Ᲊf�B��Nu /  o NU��"Ol� -D�r
N�pA 0�J�f���f�B N]�� Nu o "Hr p / +g -fRH  0m  	n$�҂ҁҀ`� -fD�$ S� o  ���Nu/./ R�L  lH�)HH.NuNU  H� 0&m $m B�L)KHHm /
Hz��N��" lHB ,LL� ��N]NuNU��H�2.- J�np�`  �p��d.  V�. G��$m  - Ї߬�A��&P+@��+H�� g  � K + ��H� ��"m���c$�%G ,m��,�p `v��f,S$� + " ҇%A ,m��,�p `X��d���p�`L��f* g��c���p�`8߫  g��f
 ) ѫ &�p `+K��+m����&S` �n m�� �B�%G p L�L�N]Nu        paH�0./ &o ,/  /N�8XO$@ 
fp�`///* N��O� * J�
�gp�` L��Nu  /./ R�PS�	*m l	"R�	" �r  ` r  Hl	/N���PO" .NuNU  /&m B�PHm /Hz��N��xHl	Hx��N��| ,P&m��N]Nu   o "o  / o��e���� S�f�Nu�S�f�Nu  H� 2&lT g$S"K ) ,x N��.&J`��)HX)HTL�L NuH�./ ,/ */  /N�(XO&@ fp�`///+ N�HO� ( J�
�gp�` L��Nu        paH�2./ pހ r ,x N��:&@ fp `8'G E�T j 'H ��&�J�f$� * g"@"�%K J��f)K�A�  L�L�Nu            H�0&o $KS�	mp  l	 )H	 `
Hl�N���XO. p���gp
��fB 
` �`ȵ�fp `B 
L��NuH�0./ J�np `  �p��d.  V�. G��E��&R g@ + ��m2��f S$���� `n + ��r��e K��$�$H$�%@ ��� `L$K&S`� ",	hЁS�N��",	hN��, P� V�, F��/N���XO&@ g//N��B.�a �TPO`p L��Nu        paH�./ G�� g4+  f(+  g  + �� , J�g//+ /+ N��6O� &S`�/N���XOL��Nu  H�7./  &o $,/ (J�
�gN�ԼB�
�"$&,ldN���* p���fN��|)@
�p)@` L�H�Nu  H�?./  ,/ $*/ (J�
�gN��pB�
� S�"$& ,ldN���( p���fN��|)@
�p)@` J�g
S�g
S�g` ` І`"t &,ldN���L�@�NuH�7./  &o $,/ (J�
�gN���B�
�"$&,ldN���* p���fN��|)@
�p)@` L�H�Nu  H�./ J�
�gN�Ӹ",ldN���p L�@�NuH�0 $ &HBHC�������CHBBBЂL� NuJ�j  D�J�j  D�a   D�Nua  D�D�NuJ�j  D�a  D�Nu/HA4f  "H@HAHB4 g  ��0H@4 ��0HB2$Nu/vA �d  �QCA d  �YCA  d  �UCJAk  �SC4 �HBBB�HC��6 04HA����d  SCЁd�r 2HC�H@�A&$Nu/./ p )@
�J�k"���l �A�DJ� g �A�D�� `p	)@`p .Nu  NU��/&m  fp `/N���&@Hm��/N��� -��&m��N]Nu  �        8Z  7�  7�  7�  7j  6�  6�  "  !�  �  "      
      �   __CXM33   NP   __dread   M�   _stcul_d  G|   _CXD33    Np   __pf  >�   _okA  7d   _write    9<   __dwrite  M    __pfmt    ;D   __tinyprintf  :   _printf   Jj   _rlsml    H�   __filbf   D�   _CXBRK    �   _stcl_o   G�   __exit    F   _CXD22    N�   _sprintf  H�   _movmem   J�   __getbf   A�   _stci_o   G�   _exit     L�   __CXM22   NP   _strcat   GD   _atoi     Ot   _strncat  F�   _stpblk   G\   _lsbrk    KH   __writes  9�   _main     "   @XCEXIT   �   __CXD22   N�   _lseek    J�   _chkufb   O4   __dseek   Ml   __main    ?T   _stcd_i   HP   _CXM22    NP   _strupr   Fl   _stci_h   G�   _gets     K�   _MemCleanup   J�   _chkabort     !�   __flsbf   B   _getml    L   _stcl_h   G�   _cleanup  7�   _stci_d   H   ___fpinit     98   _read     I�   _CXM33    NP   _stcu_d   G|   _stcl_d   H   ___fpterm     9:   _abort    7�   __dclose  N,   __CXD33   Np   _bye  7�   _XCEXIT   |   _doQ  6�   _doPQA    6�   _showos   8P   _stricmp  G      �  �  �    
          *  0  >  D  J  P  V  \  b  l  x  ~  �  �  �  �  �  �  �  �  �  �  �  �          (  4  :  D  J  V  Z  b  p  v  ~  �  �  �  �  �  �  �  �  �  �           .  :  F  L  V  ^  j  v  z  �  �  �  �  �  �  �  �  �  �  �  �  �  
      &  4  >  H  R  \  d  l  v  ~  �  �  �  �  �  �  �  �  �  �  �         &  4  D  P  `  p  �  �  �  �  �  �  �  �  �  �  �  �  �           0  <  B  P  X  b  h  r  ~  �  �  �  �  �        �  
  �    �    �    �      *    0  (  >  .  D  :  J  B  P  L  V  L  \  :  b  X  l  �  ~  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  	  �  �  �  	  �  �    �    	    	    	$  (  �  4  	,  :  	@  D  	N  J  	`  V  �  Z  	n  b  	�  p  	�  v  	�  ~  
  �  	$  �  	  �  �  �  
"  �  	$  �  	$  �  
.  �  
@  �  �  �  �    (    
H    (     
f  .  
t  :  (  F  �  L  �  V  �  ^  
�  j  
�  v  
�  z  
�  �  
�  �  
�  �  
�  �  	$  �  :  �  
�  �  (  �    �  �  �    �  �  �  �  �  (  
       	    H  &  	$  4  T  >  b  H  �  R  	$  \  	$  d  �  v  p  ~  	  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �     �         (  &  (  4  	$  D    P  �  `  	$  p  	$  �     �  	$  �  (  �  	  �  	$  �  �  �  �  �  �  �  	�  �  
�  �  .  �  *  �  
"    (    4    	$     	�  0  B  <  L  B  	  P  �  X  x  b  	  h  (  r  �  ~  �  �  �  �  �  �  	  �  �  �  �          �  �    d  �  �  B  �  �  "  j  �  �  4  �  �  �  *          L      p      ~      �      �      �  �  �      �      �      �      �                  D      H      r      x      �      �      �      �      �      �      �            :      B      \      `      |      �      �      �      �    �  �          �          �  Z  v      z  Z  �          �  �      "  8  L  �  �  6  ~  �  �  !    �  "    �  #    �  $   � �    � �   � �    � �               (                      	                                	@                                                                    �              (((((                  H����������������������          (((((                  H����������������������         �  �      �  �  �  �  �  �  �  |  x  t  p  l  h  \  X  T  L  H  D  8  ,  (  $              �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  x  p  h  `  X  P  H  D  @  8  0  (                �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  |  x  t  p  l  h  d  `  \  X  T  P  L  H  D  @  <  8  4  0  ,  (  $                      �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  |  x  t  p  l  h  d  `  \  X  T  P  L  H  D  @  <  8  4  0  ,  (  $                      �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  |  x  t  p  l  h  d  `  \  X  T  P  L  H  D  @  <  8  4  0  ,  (  $                      �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  |  x  t  p  l  h  d  `  \  X  T  P  L  H  D  @  <  8  4  0  ,  (  $                      �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  |  x  t  p  l  h  d  `  \  X  T  P  L  H  @  <  8  4  0  ,  (  $                      �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  �  |  x  t  p  l  h  d  `  \  X  T  P  L  H  D  @  <  8  4  0  ,  (  $                       �   �   �   �   �   �   �   �   �   �   �   �   �   �   �   �   �   �   �   �   �   �   �   �   �   �   �   �   �   �   �   �   |   x   t   p   l   h   d   `   \   X   T   P   L   H   D   @   <   8   4   0   ,   (   $                                     	  �      �   __fmode   	d   _rtypes   h   __OSERR   
�   _bout     �   __ONEXIT  
�   _curdir   
�   _longrtypes   t   _ibuf     
�   _subsys1  �   __oserr   
�   _vers     �   __iob     �   __SIGFPE  
�   _line     �   __pool    �   __SIGINT  
�   _Categories        _SysBase  
�   __mbase   
|   _ifile    
�   __bufsiz  
p   _subsys5  �   __msize   
�   _lfile    
�   __MemList     T   _usage    �   _ofile    
�   _mo   H   _wbver       _btype    d   _tbuf     
�   _helplines    �   __ONERR   
�   __mnext   
�   _NULL     
t   _cmo  <   __ufbs    D   __StackPtr    
�   _bi      _subsys4  �   __ctype   	l   _subsys3  �   _obuf     
�   _WBenchMsg    
�   __tsize   
�   __melt    �   _ksver    
�   __nufbs   �   _cbuf     
�   __mstep   	h   _bc   d   _rf   8   __ProgramName     
�   _subsys2  �   _IntuitionBase    @   _errno    `   __base    
x   _fsubsys  �   _bfile    
�   _co   �   __ONBREAK     
�   _qq   $   __FPERR   
�   _Copyright    �   _DOSBase  d      �