AJ          2   N       	RY� �@(#)rpc_svcout.c 1.6 87/06/24 (C) 1987 SMI ��rqstp ��transp ��argument ��result ��local �NU��+m�q ��`l m��+P�� m���    fL m��+h ��`:Hz�!` 0/-�r N��s PO m��/(  m��/N��t POHz�!` >/-�r N��s PO m��+h ��J���f� m��+h ��J���f�Hz�!` R�/-�r N��s POHz�!` U�/-�r N��s POHz�!` ]�/-�r N��s POHz�u Hz�!` P�/-�r N��s O� Hz�!` _�/-�r N��s PO+m�q ��`Z m��+P�� m���    g`8 m��+h ��`& m��/ m��/Hz�!` Q�/-�r N��s O�  m��+h ��J���f� m��+h ��J���f�N]Nu
static void  (); 

 main()
 {
 	SVCXPRT *%s;
 
 	pmap_unset(%s, %s);
  NU��Hz�!a 0/-�r N��s PO/- Hz�u Hz�!a 2/-�r N��s O� Hz�!a Q�/- N��w POJ�gHz�!a U�/-�r N��s POHz�!a \�/-�r N��s POHz�u Hz�!a P�/-�r N��s O� /- Hz�!a T�/-�r N��s O� Hz�!a W�/-�r N��s POHz�!a S�/-�r N��s PO+m�q ��`  � m��+P�� m���    g`  � m��+h ��`  � m��/ m��/Hz�u Hz�!a W�/-�r N��s O�  m��/(  m��/N��t POHz�!a Y�/- N��w POJ�gA��!a ]� `A��!a Q� / Hz�!a W�/-�r N��s O� /-  m��/ m��/Hz�!a U�/-�r N��s O� Hz�!a _�/-�r N��s POHz�!a [�/-�r N��s PO m��+h ��J���f �N m��+h ��J���f �N]Nu
 	%s = svc%s_create(RPC_ANYSOCK tcp , 0, 0 );
 	if (%s == NULL) {
 		fprintf(stderr, "cannot create %s service.\n");
 		exit(1);
 	}
 	if (!svc_register(%s, %s, %s,  , IPPROTO_%s)) {
 udp UDP TCP 		fprintf(stderr, "unable to register (%s, %s, %s).\n");
 		exit(1);
 	}
  NU  Hz�!b 0/-�r N��s POHz�!b =/-�r N��s POHz�!b V�/-�r N��s POHz�!b Q�/-�r N��s PON]Nu	svc_run();
 	fprintf(stderr, "svc_run returned\n");
 	exit(1);
 }
 NU��+m�q ��`, m��+P�� m���    f/- /-��aPO m��+h ��J���f�N]NuNU�� m +h ��` Hz�!c 0/-�r N��s POJ� g/- Hz�!c 2/-�r N��s O� Hz�!c 6/-�r N��s PO m��/(  m /N��t POHz�u Hz�z Hz�!c </-�r N��s O� Hz�z Hz�!c V�/-�r N��s O� Hz�u Hz�!c \�/-�r N��s O� Hz�!c [�/-�r N��s POB���Hz�!c ^�/-�r N��s PO m��+h ��`tHz�!c X� m��/( N��w POJ�g`R+|   ��Hz�!c ]�/-�r N��s POB� m��/(  m��/( N��{ O�  m��/(  m��/N��t POHz�!c P�/-�r N��s PO m��+h ��J���f�J���fHz�!c W�/-�r N��s POHz�| Hz�!c T�/-�r N��s O� Hz�} Hz�!c \�/-�r N��s O� Hz�} Hz�| Hz�!c X�/-�r N��s O� Hz�~ Hz�!c [�/-�r N��s O� Hz�!c \�/-�r N��s POHz�z Hz�!c ^�/-�r N��s O�  m��/( N�8XOJ�f0Hz�!c W�/-�r N��s POHz�u Hz�!c X�/-�r N��s O� Hz�!c ^�/-�r N��s PO m��+h ��`  � m��/Hz�!c Z�/-�r N��s O�  m��/( N�� XO/ Hz�| Hz�!c U�/-�r N��s O�  m��/( N�� XO/ Hz�} Hz�!c Y�/-�r N��s O� Hz�~ Hz�!c ]�/-�r N��s O�  m��/(  m��/N��t POHz�!c S�/-�r N��s POHz�!c V�/-�r N��s PO m��+h ��J���f �NHz�!c Q�/-�r N��s POHz�u Hz�!c \�N��POHz�!c S�/-�r N��s POHz�!c ^�/-�r N��s POHz�| Hz�| Hz�!c R�/-�r N��s O� Hz�| Hz�!c T�Hz�u Hz�!c \�N��O� Hz�u Hz�!c V�N�JPOHz�!c ]�/-�r N��s POHz�!c X�/-�r N��s POHz�z Hz�| Hz�~ Hz�} Hz�!c \�/-�r N��s O� Hz�} Hz�} Hz�u Hz�} Hz�!c S�/-�r N��s O� Hz�u Hz�!c Y�N��POHz�!c S�/-�r N��s POHz�| Hz�!c P�Hz�u Hz�!c W�N��O� Hz�!c R�/-�r N��s POHz�!c T�/-�r N��s POHz�!c P�/-�r N��s POHz�!c T�/-�r N��s PO m��+h ��J���f ��N]Nu
 %s  void
 (%s, %s)
 	struct svc_req *%s;
 	SVCXPRT *%s;
 {
 	union {
 void 		 _arg;
 		int fill;
 	} %s;
 	char *%s;
 	bool_t (*xdr_%s)(), (*xdr_%s)();
 	char *(*%s)();
 
 	switch (%s->rq_proc) {
 	case NULLPROC:
 		svc_sendreply(%s, xdr_void, NULL);
 		return;

 	case %s:
 		xdr_%s = xdr_%s;
 		xdr_%s = xdr_%s;
 		%s = (char *(*)())  ;
 		break;

 	default:
 noproc 		return;
 	}
 	bzero(&%s, sizeof(%s));
 getargs & decode 		return;
 	}
 	%s = (*%s)(&%s, %s);
 	if (%s != NULL && !svc_sendreply(%s, xdr_%s, %s)) {
 systemerr 	}
 freeargs & 		fprintf(stderr, "unable to free arguments\n");
 		exit(1);
 	}
 }

 NU  /- /- Hz�!d 0/-�r N��s O� N]Nu		svcerr_%s(%s);
 NU  /- /- /- /- /- Hz�!e 0/-�r N��s O� N]Nu	if (!svc_%s(%s, xdr_%s, %s%s)) {
  NU  `&Hz�!f 0 m /( N��w POJ�gpN]Nu m +h  J� f�p `�0 �    _write_most 
  _defined 
  _fout   _fprintf   _pvname   1_TRANSP ~_write_register   _streq &_write_rest �_write_programs   +_RQSTP   _ptype   8_ARG   A_RESULT   H_ROUTINE   _stringfix �_nullproc   .begin  �6��f�	�����0