AJ          ,   V    $  	�:G� �@(#)rpc_main.c 1.7 87/06/24 (C) 1987 SMI �� �!` 0�MANX36 MANX36 ��!a 0� $VER: rpcgen 36.1 (7.11.90)  �/lib/cpp ��-C ���!b 0�!b 7�!b :�!b >�!b Q��rpcgen -s udp -s tcp  �   �NU��Hm��/- /- N�	O� J�fF/-�p Hz�!c 0Hz�!r \�N��s O� /-�p Hz�!c R�Hz�!r \�N��s O� /-�p Hz�!c W�Hz�!r \�N��s O� Hx N��t XOJ���g/-��B�Hz�!c V�/-��N�hO� `  �J���g/-��B�Hz�!c P�/-��N�<O� `  �J���g/-��B�Hz�!c Z�/-��N��O� `  �J���fJ���g$/-��/-��B�Hz�!c U�/-��/- /- N�pO� `xHz�!c Y�Hx Hz�!c _�/-��N��O� N��u Hz�!c Z�Hx Hz�!c P�/-��N��O� N��u Hz�!c X�Hx Hz�!c ]�/-��N�hO� N��u /-��Hz�!c Z�Hx Hz�!c P�/-��Hz�v /-�w N��O� B�N��t XON]Nuusage: %s infile
        %s [-c | -h | -l | -m] [-o outfile] [infile]
        %s [-s udp|tcp]* [-o outfile] [infile]
 -DRPC_XDR -DRPC_HDR -DRPC_CLNT -DRPC_SVC -DRPC_XDR _xdr.c -DRPC_HDR .h -DRPC_CLNT _clnt.c -DRPC_SVC _svc.c  NU��/- N��x XO/ /- N��x XO"ҀR�/N��y XO+@��J���fN��z Hx ./- N��{ PO+@��J���f/- N��x XOЭ +@��/- /-��N��| PO/-  m���� ����/N��| PO -��N]NuNU  J� fA��!r V�+HЂ} N]NuJ� g./- /- N��~ POJ�g/- /-�p Hz�!d 0Hz�!r \�N��s O� N�� Hz�!d _�/- N��pPO+@Ђ} J��} f"/-�p Hz�!d Q�Hz�!r \�N��s O� /- N��qXON�� /- N��rXO`�%s: output would overwrite %s
 w %s: unable to open   NU��B�N��tXO+@��J� fHz�!e 0Hz�!r \�N��s PON�� B�/-��/- /- Hz�!e P�Hz�!e Y�N��uO� +@��J���g/- Hz�!e W�Hz�!r \�N��s O� N�� Hz�!e V�/-��N��pPO+@ЂvJ��vf"/-�p Hz�!e X�Hz�!r \�N��s O� /-�wN��qXON�� /-��N��x XOR�/ N��y XO+@��J���g"/-��/-��N��| PO m�v�   m�v!m�� N]Numust specify input file
 gnucpp gnucpp could not pre-process file %s
 r %s:   NU��/- /- N���POJ� g/- /- N��VPO` - +@��/-��/- N���POHz�!f 0/-�} N��s POJ� g2Hz�!f V�/- N��PO+@��g/-��Hz�!f Y�/-�} N��s O� /-��N��xXO/-�} N��yXO+@��N��z+@��g/-��N��{XO`�J� g/-�} N��yXO����f
/-��N��|XON]Nu#include <rpc/rpc.h>
 .h #include "%s"
 NU��/- /- N���POJ� g/- /- N��bPO` - +@��/-��/- N���PO/-�} N��yXO+@��N��z+@��g/-��N��}XO`�J� g/-�} N��yXO����f
/-��N��|XON]NuNU��/- /- N��DPOJ� g/- /- N���PO` - +@��/-��/- N��RPOHz�!g 0/-�} N��s POHz�!g T�/-�} N��s POJ� g2Hz�!g Z�/- N���PO+@��g/-��Hz�!g ]�/-�} N��s O� /-��N��xXOB���N��z+@��g m���    fp`p ����`�J� gJ���f/-��N��|XON]NuJ�  g
B�N��~XO` N��/- /- N�rPON��pHz�!g \�N��~XO`�#include <stdio.h>
 #include <rpc/rpc.h>
 .h #include "%s"
 static  NU��/- /- N���POJ� g/- /- N���PO` - +@��/-��/- N��POHz�!h 0/-�} N��s POHz�!h V�/-�} N��s POJ� g2Hz�!h ]�/- N��JPO+@��g/-��Hz�!h P�/-�} N��s O� /-��N��xXOB���N��z+@��g m���    fp`p ����`�J� gJ���f/-��N��|XON]NuN��q`�#include <rpc/rpc.h>
 #include <sys/time.h>
 .h #include "%s"
  NU��+|   ��`:Hz�!i 0 -��� m /0 N��~ POJ�g -��R�� m /0 N��rXOR���R��� -���� m�N]Nu-s  NU�� m +PЂp  m B�  m B� �    lp N]NuB-�ZB-�_B-�jB-�fB-�cB-�d+|   ��` � -��� m "p  -g& m J� gp `� -��� m "m #p  ` �+|   ��` \ -��� m "0  m��p ��-��H�H�` 
-��H�H�A���J0 gp ` �X-��H�H�A����  `  -��� m "p ����) -��f -��� m "p ����J) gp ` �
-��H�H�A����  R��� -���� fp ` ��- s��f@Hz�!j 0 -��� m /0 N��~ POJ�f"Hz�!j 4 -��� m /0 N��~ POJ�fp ` ��`,- o��f$ m J� gp ` �� -��� m "m #p  `Np ` �h��   cg ��[�g ��Y�g ��S�g ��U�g �Y�g � `�R��� -��� m "0  m��J0 f ��R��� -���� m �8-�ZH�H� m  �-�_H�H� m !@ -�jH�H� m !@ -�cH�H� m !@ -�dH�H� m !@  m "m  ( Щ  m Ш  m Ш  m А+@��J���f m J� f
 m J� fp ` �x`�   ��op ` �fp` �`udp tcp �  	   _cmdname  B_main 
  _Cbuffs   _fprintf   _exit   _reinitialize   >_allv   R_allc   _strlen   _malloc   _abort   _rindex   _strcpy 
  _fout   _streq   _crash   _fopen   _perror   _record_open �_open_input   _tmpnam   _fexecl 
  _fin 
  _infilename   _free   _ftell   _get_definition   _emit   _unlink   _print_datadef   _write_programs   _write_most   _write_rest   _write_stubs   _write_register   .begin   �   �  � , �������������	V�$  