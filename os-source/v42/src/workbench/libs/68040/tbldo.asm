*
* $Id: tbldo.asm,v 0.1 91/06/26 08:59:14 mks Exp $
*
* $Log:	tbldo.asm,v $
* Revision 0.1  91/06/26  08:59:14  mks
* Initial check in...
* 
*

	include	"assembly_options.i"

*
*	tbldo.sa 3.1 12/10/90
*
* Modified:
*	8/16/90	chinds	The table was constructed to use only one level
*			of indirection in do_func for monoadic
*			functions.  Dyadic functions require two
*			levels, and the tables are still contained
*			in do_func.  The table is arranged for
*			index with a 10-bit index, with the first
*			7 bits the opcode, and the remaining 3
*			the stag.  For dyadic functions, all
*			valid addresses are to the generic entry
*			point.
*

*		Copyright (C) Motorola, Inc. 1990
*			All Rights Reserved
*
*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF MOTOROLA
*	The copyright notice above does not evidence any
*	actual or intended publication of such source code.

TBLDO	IDNT    2,1 Motorola 040 Floating Point Software Package

	section	8

	xref	ld_pinf,ld_pone,ld_ppi2
	xref	t_dz2,t_operr
	xref	serror,sone,szero,sinf,snzrinx
	xref	sopr_inf,spi_2,src_nan,szr_inf

	xref	smovcr
	xref	pmod,prem,pscale
	xref	satanh,satanhd
	xref	sacos,sacosd,sasin,sasind,satan,satand
	xref	setox,setoxd,setoxm1,setoxm1d,setoxm1i
	xref	sgetexp,sgetexpd,sgetman,sgetmand
	xref	sint,sintd,sintrz
	xref	ssincos,ssincosd,ssincosi,ssincosnan,ssincosz
	xref	scos,scosd,ssin,ssind,stan,stand
	xref	scosh,scoshd,ssinh,ssinhd,stanh,stanhd
	xref	sslog10,sslog2,sslogn,sslognp1
	xref	sslog10d,sslog2d,sslognd,slognp1d
	xref	stentox,stentoxd,stwotox,stwotoxd

*	instruction		;opcode-stag Notes
	xdef	tblpre
tblpre:
	dc.l	smovcr		;$00-0 fmovecr all
	dc.l	smovcr		;$00-1 fmovecr all
	dc.l	smovcr		;$00-2 fmovecr all
	dc.l	smovcr		;$00-3 fmovecr all
	dc.l	smovcr		;$00-4 fmovecr all
	dc.l	smovcr		;$00-5 fmovecr all
	dc.l	smovcr		;$00-6 fmovecr all
	dc.l	smovcr		;$00-7 fmovecr all

	dc.l	sint		;$01-0 fint norm
	dc.l	szero		;$01-1 fint zero
	dc.l	sinf		;$01-2 fint inf
	dc.l	src_nan		;$01-3 fint nan
	dc.l	sintd		;$01-4 fint denorm inx
	dc.l	serror		;$01-5 fint ERROR
	dc.l	serror		;$01-6 fint ERROR
	dc.l	serror		;$01-7 fint ERROR

	dc.l	ssinh		;$02-0 fsinh norm
	dc.l	szero		;$02-1 fsinh zero
	dc.l	sinf		;$02-2 fsinh inf
	dc.l	src_nan		;$02-3 fsinh nan
	dc.l	ssinhd		;$02-4 fsinh denorm
	dc.l	serror		;$02-5 fsinh ERROR
	dc.l	serror		;$02-6 fsinh ERROR
	dc.l	serror		;$02-7 fsinh ERROR

	dc.l	sintrz		;$03-0 fintrz norm
	dc.l	szero		;$03-1 fintrz zero
	dc.l	sinf		;$03-2 fintrz inf
	dc.l	src_nan		;$03-3 fintrz nan
	dc.l	snzrinx		;$03-4 fintrz denorm inx
	dc.l	serror		;$03-5 fintrz ERROR
	dc.l	serror		;$03-6 fintrz ERROR
	dc.l	serror		;$03-7 fintrz ERROR

	dc.l	serror		;$04-0 ERROR - illegal extension
	dc.l	serror		;$04-1 ERROR - illegal extension
	dc.l	serror		;$04-2 ERROR - illegal extension
	dc.l	serror		;$04-3 ERROR - illegal extension
	dc.l	serror		;$04-4 ERROR - illegal extension
	dc.l	serror		;$04-5 ERROR - illegal extension
	dc.l	serror		;$04-6 ERROR - illegal extension
	dc.l	serror		;$04-7 ERROR - illegal extension

	dc.l	serror		;$05-0 ERROR - illegal extension
	dc.l	serror		;$05-1 ERROR - illegal extension
	dc.l	serror		;$05-2 ERROR - illegal extension
	dc.l	serror		;$05-3 ERROR - illegal extension
	dc.l	serror		;$05-4 ERROR - illegal extension
	dc.l	serror		;$05-5 ERROR - illegal extension
	dc.l	serror		;$05-6 ERROR - illegal extension
	dc.l	serror		;$05-7 ERROR - illegal extension

	dc.l	sslognp1	;$06-0 flognp1 norm
	dc.l	szero		;$06-1 flognp1 zero
	dc.l	sopr_inf	;$06-2 flognp1 inf
	dc.l	src_nan		;$06-3 flognp1 nan
	dc.l	slognp1d	;$06-4 flognp1 denorm
	dc.l	serror		;$06-5 flognp1 ERROR
	dc.l	serror		;$06-6 flognp1 ERROR
	dc.l	serror		;$06-7 flognp1 ERROR

	dc.l	serror		;$07-0 ERROR - illegal extension
	dc.l	serror		;$07-1 ERROR - illegal extension
	dc.l	serror		;$07-2 ERROR - illegal extension
	dc.l	serror		;$07-3 ERROR - illegal extension
	dc.l	serror		;$07-4 ERROR - illegal extension
	dc.l	serror		;$07-5 ERROR - illegal extension
	dc.l	serror		;$07-6 ERROR - illegal extension
	dc.l	serror		;$07-7 ERROR - illegal extension

	dc.l	setoxm1		;$08-0 fetoxm1 norm
	dc.l	szero		;$08-1 fetoxm1 zero
	dc.l	setoxm1i	;$08-2 fetoxm1 inf
	dc.l	src_nan		;$08-3 fetoxm1 nan
	dc.l	setoxm1d	;$08-4 fetoxm1 denorm
	dc.l	serror		;$08-5 fetoxm1 ERROR
	dc.l	serror		;$08-6 fetoxm1 ERROR
	dc.l	serror		;$08-7 fetoxm1 ERROR

	dc.l	stanh		;$09-0 ftanh norm
	dc.l	szero		;$09-1 ftanh zero
	dc.l	sone		;$09-2 ftanh inf
	dc.l	src_nan		;$09-3 ftanh nan
	dc.l	stanhd		;$09-4 ftanh denorm
	dc.l	serror		;$09-5 ftanh ERROR
	dc.l	serror		;$09-6 ftanh ERROR
	dc.l	serror		;$09-7 ftanh ERROR

	dc.l	satan		;$0a-0 fatan norm
	dc.l	szero		;$0a-1 fatan zero
	dc.l	spi_2		;$0a-2 fatan inf
	dc.l	src_nan		;$0a-3 fatan nan
	dc.l	satand		;$0a-4 fatan denorm
	dc.l	serror		;$0a-5 fatan ERROR
	dc.l	serror		;$0a-6 fatan ERROR
	dc.l	serror		;$0a-7 fatan ERROR

	dc.l	serror		;$0b-0 ERROR - illegal extension
	dc.l	serror		;$0b-1 ERROR - illegal extension
	dc.l	serror		;$0b-2 ERROR - illegal extension
	dc.l	serror		;$0b-3 ERROR - illegal extension
	dc.l	serror		;$0b-4 ERROR - illegal extension
	dc.l	serror		;$0b-5 ERROR - illegal extension
	dc.l	serror		;$0b-6 ERROR - illegal extension
	dc.l	serror		;$0b-7 ERROR - illegal extension

	dc.l	sasin		;$0c-0 fasin norm
	dc.l	szero		;$0c-1 fasin zero
	dc.l	t_operr		;$0c-2 fasin inf
	dc.l	src_nan		;$0c-3 fasin nan
	dc.l	sasind		;$0c-4 fasin denorm
	dc.l	serror		;$0c-5 fasin ERROR
	dc.l	serror		;$0c-6 fasin ERROR
	dc.l	serror		;$0c-7 fasin ERROR

	dc.l	satanh		;$0d-0 fatanh norm
	dc.l	szero		;$0d-1 fatanh zero
	dc.l	t_operr		;$0d-2 fatanh inf
	dc.l	src_nan		;$0d-3 fatanh nan
	dc.l	satanhd		;$0d-4 fatanh denorm
	dc.l	serror		;$0d-5 fatanh ERROR
	dc.l	serror		;$0d-6 fatanh ERROR
	dc.l	serror		;$0d-7 fatanh ERROR

	dc.l	ssin		;$0e-0 fsin norm
	dc.l	szero		;$0e-1 fsin zero
	dc.l	t_operr		;$0e-2 fsin inf
	dc.l	src_nan		;$0e-3 fsin nan
	dc.l	ssind		;$0e-4 fsin denorm
	dc.l	serror		;$0e-5 fsin ERROR
	dc.l	serror		;$0e-6 fsin ERROR
	dc.l	serror		;$0e-7 fsin ERROR

	dc.l	stan		;$0f-0 ftan norm
	dc.l	szero		;$0f-1 ftan zero
	dc.l	t_operr		;$0f-2 ftan inf
	dc.l	src_nan		;$0f-3 ftan nan
	dc.l	stand		;$0f-4 ftan denorm
	dc.l	serror		;$0f-5 ftan ERROR
	dc.l	serror		;$0f-6 ftan ERROR
	dc.l	serror		;$0f-7 ftan ERROR

	dc.l	setox		;$10-0 fetox norm
	dc.l	ld_pone		;$10-1 fetox zero
	dc.l	szr_inf		;$10-2 fetox inf
	dc.l	src_nan		;$10-3 fetox nan
	dc.l	setoxd		;$10-4 fetox denorm
	dc.l	serror		;$10-5 fetox ERROR
	dc.l	serror		;$10-6 fetox ERROR
	dc.l	serror		;$10-7 fetox ERROR

	dc.l	stwotox		;$11-0 ftwotox norm
	dc.l	ld_pone		;$11-1 ftwotox zero
	dc.l	szr_inf		;$11-2 ftwotox inf
	dc.l	src_nan		;$11-3 ftwotox nan
	dc.l	stwotoxd	;$11-4 ftwotox denorm
	dc.l	serror		;$11-5 ftwotox ERROR
	dc.l	serror		;$11-6 ftwotox ERROR
	dc.l	serror		;$11-7 ftwotox ERROR

	dc.l	stentox		;$12-0 ftentox norm
	dc.l	ld_pone		;$12-1 ftentox zero
	dc.l	szr_inf		;$12-2 ftentox inf
	dc.l	src_nan		;$12-3 ftentox nan
	dc.l	stentoxd	;$12-4 ftentox denorm
	dc.l	serror		;$12-5 ftentox ERROR
	dc.l	serror		;$12-6 ftentox ERROR
	dc.l	serror		;$12-7 ftentox ERROR

	dc.l	serror		;$13-0 ERROR - illegal extension
	dc.l	serror		;$13-1 ERROR - illegal extension
	dc.l	serror		;$13-2 ERROR - illegal extension
	dc.l	serror		;$13-3 ERROR - illegal extension
	dc.l	serror		;$13-4 ERROR - illegal extension
	dc.l	serror		;$13-5 ERROR - illegal extension
	dc.l	serror		;$13-6 ERROR - illegal extension
	dc.l	serror		;$13-7 ERROR - illegal extension

	dc.l	sslogn		;$14-0 flogn norm
	dc.l	t_dz2		;$14-1 flogn zero
	dc.l	sopr_inf	;$14-2 flogn inf
	dc.l	src_nan		;$14-3 flogn nan
	dc.l	sslognd		;$14-4 flogn denorm
	dc.l	serror		;$14-5 flogn ERROR
	dc.l	serror		;$14-6 flogn ERROR
	dc.l	serror		;$14-7 flogn ERROR

	dc.l	sslog10		;$15-0 flog10 norm
	dc.l	t_dz2		;$15-1 flog10 zero
	dc.l	sopr_inf	;$15-2 flog10 inf
	dc.l	src_nan		;$15-3 flog10 nan
	dc.l	sslog10d	;$15-4 flog10 denorm
	dc.l	serror		;$15-5 flog10 ERROR
	dc.l	serror		;$15-6 flog10 ERROR
	dc.l	serror		;$15-7 flog10 ERROR

	dc.l	sslog2		;$16-0 flog2 norm
	dc.l	t_dz2		;$16-1 flog2 zero
	dc.l	sopr_inf	;$16-2 flog2 inf
	dc.l	src_nan		;$16-3 flog2 nan
	dc.l	sslog2d		;$16-4 flog2 denorm
	dc.l	serror		;$16-5 flog2 ERROR
	dc.l	serror		;$16-6 flog2 ERROR
	dc.l	serror		;$16-7 flog2 ERROR

	dc.l	serror		;$17-0 ERROR - illegal extension
	dc.l	serror		;$17-1 ERROR - illegal extension
	dc.l	serror		;$17-2 ERROR - illegal extension
	dc.l	serror		;$17-3 ERROR - illegal extension
	dc.l	serror		;$17-4 ERROR - illegal extension
	dc.l	serror		;$17-5 ERROR - illegal extension
	dc.l	serror		;$17-6 ERROR - illegal extension
	dc.l	serror		;$17-7 ERROR - illegal extension

	dc.l	serror		;$18-0 ERROR - illegal extension
	dc.l	serror		;$18-1 ERROR - illegal extension
	dc.l	serror		;$18-2 ERROR - illegal extension
	dc.l	serror		;$18-3 ERROR - illegal extension
	dc.l	serror		;$18-4 ERROR - illegal extension
	dc.l	serror		;$18-5 ERROR - illegal extension
	dc.l	serror		;$18-6 ERROR - illegal extension
	dc.l	serror		;$18-7 ERROR - illegal extension

	dc.l	scosh		;$19-0 fcosh norm
	dc.l	ld_pone		;$19-1 fcosh zero
	dc.l	ld_pinf		;$19-2 fcosh inf
	dc.l	src_nan		;$19-3 fcosh nan
	dc.l	scoshd		;$19-4 fcosh denorm
	dc.l	serror		;$19-5 fcosh ERROR
	dc.l	serror		;$19-6 fcosh ERROR
	dc.l	serror		;$19-7 fcosh ERROR

	dc.l	serror		;$1a-0 ERROR - illegal extension
	dc.l	serror		;$1a-1 ERROR - illegal extension
	dc.l	serror		;$1a-2 ERROR - illegal extension
	dc.l	serror		;$1a-3 ERROR - illegal extension
	dc.l	serror		;$1a-4 ERROR - illegal extension
	dc.l	serror		;$1a-5 ERROR - illegal extension
	dc.l	serror		;$1a-6 ERROR - illegal extension
	dc.l	serror		;$1a-7 ERROR - illegal extension

	dc.l	serror		;$1b-0 ERROR - illegal extension
	dc.l	serror		;$1b-1 ERROR - illegal extension
	dc.l	serror		;$1b-2 ERROR - illegal extension
	dc.l	serror		;$1b-3 ERROR - illegal extension
	dc.l	serror		;$1b-4 ERROR - illegal extension
	dc.l	serror		;$1b-5 ERROR - illegal extension
	dc.l	serror		;$1b-6 ERROR - illegal extension
	dc.l	serror		;$1b-7 ERROR - illegal extension

	dc.l	sacos		;$1c-0 facos norm
	dc.l	ld_ppi2		;$1c-1 facos zero
	dc.l	t_operr		;$1c-2 facos inf
	dc.l	src_nan		;$1c-3 facos nan
	dc.l	sacosd		;$1c-4 facos denorm
	dc.l	serror		;$1c-5 facos ERROR
	dc.l	serror		;$1c-6 facos ERROR
	dc.l	serror		;$1c-7 facos ERROR

	dc.l	scos		;$1d-0 fcos norm
	dc.l	ld_pone		;$1d-1 fcos zero
	dc.l	t_operr		;$1d-2 fcos inf
	dc.l	src_nan		;$1d-3 fcos nan
	dc.l	scosd		;$1d-4 fcos denorm
	dc.l	serror		;$1d-5 fcos ERROR
	dc.l	serror		;$1d-6 fcos ERROR
	dc.l	serror		;$1d-7 fcos ERROR

	dc.l	sgetexp		;$1e-0 fgetexp norm
	dc.l	szero		;$1e-1 fgetexp zero
	dc.l	t_operr		;$1e-2 fgetexp inf
	dc.l	src_nan		;$1e-3 fgetexp nan
	dc.l	sgetexpd	;$1e-4 fgetexp denorm
	dc.l	serror		;$1e-5 fgetexp ERROR
	dc.l	serror		;$1e-6 fgetexp ERROR
	dc.l	serror		;$1e-7 fgetexp ERROR

	dc.l	sgetman		;$1f-0 fgetman norm
	dc.l	szero		;$1f-1 fgetman zero
	dc.l	t_operr		;$1f-2 fgetman inf
	dc.l	src_nan		;$1f-3 fgetman nan
	dc.l	sgetmand	;$1f-4 fgetman denorm
	dc.l	serror		;$1f-5 fgetman ERROR
	dc.l	serror		;$1f-6 fgetman ERROR
	dc.l	serror		;$1f-7 fgetman ERROR

	dc.l	serror		;$20-0 ERROR - illegal extension
	dc.l	serror		;$20-1 ERROR - illegal extension
	dc.l	serror		;$20-2 ERROR - illegal extension
	dc.l	serror		;$20-3 ERROR - illegal extension
	dc.l	serror		;$20-4 ERROR - illegal extension
	dc.l	serror		;$20-5 ERROR - illegal extension
	dc.l	serror		;$20-6 ERROR - illegal extension
	dc.l	serror		;$20-7 ERROR - illegal extension

	dc.l	pmod		;$21-0 fmod all
	dc.l	pmod		;$21-1 fmod all
	dc.l	pmod		;$21-2 fmod all
	dc.l	pmod		;$21-3 fmod all
	dc.l	pmod		;$21-4 fmod all
	dc.l	serror		;$21-5 fmod ERROR
	dc.l	serror		;$21-6 fmod ERROR
	dc.l	serror		;$21-7 fmod ERROR

	dc.l	serror		;$22-0 ERROR - illegal extension
	dc.l	serror		;$22-1 ERROR - illegal extension
	dc.l	serror		;$22-2 ERROR - illegal extension
	dc.l	serror		;$22-3 ERROR - illegal extension
	dc.l	serror		;$22-4 ERROR - illegal extension
	dc.l	serror		;$22-5 ERROR - illegal extension
	dc.l	serror		;$22-6 ERROR - illegal extension
	dc.l	serror		;$22-7 ERROR - illegal extension

	dc.l	serror		;$23-0 ERROR - illegal extension
	dc.l	serror		;$23-1 ERROR - illegal extension
	dc.l	serror		;$23-2 ERROR - illegal extension
	dc.l	serror		;$23-3 ERROR - illegal extension
	dc.l	serror		;$23-4 ERROR - illegal extension
	dc.l	serror		;$23-5 ERROR - illegal extension
	dc.l	serror		;$23-6 ERROR - illegal extension
	dc.l	serror		;$23-7 ERROR - illegal extension

	dc.l	serror		;$24-0 ERROR - illegal extension
	dc.l	serror		;$24-1 ERROR - illegal extension
	dc.l	serror		;$24-2 ERROR - illegal extension
	dc.l	serror		;$24-3 ERROR - illegal extension
	dc.l	serror		;$24-4 ERROR - illegal extension
	dc.l	serror		;$24-5 ERROR - illegal extension
	dc.l	serror		;$24-6 ERROR - illegal extension
	dc.l	serror		;$24-7 ERROR - illegal extension

	dc.l	prem		;$25-0 frem all
	dc.l	prem		;$25-1 frem all
	dc.l	prem		;$25-2 frem all
	dc.l	prem		;$25-3 frem all
	dc.l	prem		;$25-4 frem all
	dc.l	serror		;$25-5 frem ERROR
	dc.l	serror		;$25-6 frem ERROR
	dc.l	serror		;$25-7 frem ERROR

	dc.l	pscale		;$26-0 fscale all
	dc.l	pscale		;$26-1 fscale all
	dc.l	pscale		;$26-2 fscale all
	dc.l	pscale		;$26-3 fscale all
	dc.l	pscale		;$26-4 fscale all
	dc.l	serror		;$26-5 fscale ERROR
	dc.l	serror		;$26-6 fscale ERROR
	dc.l	serror		;$26-7 fscale ERROR

	dc.l	serror		;$27-0 ERROR - illegal extension
	dc.l	serror		;$27-1 ERROR - illegal extension
	dc.l	serror		;$27-2 ERROR - illegal extension
	dc.l	serror		;$27-3 ERROR - illegal extension
	dc.l	serror		;$27-4 ERROR - illegal extension
	dc.l	serror		;$27-5 ERROR - illegal extension
	dc.l	serror		;$27-6 ERROR - illegal extension
	dc.l	serror		;$27-7 ERROR - illegal extension

	dc.l	serror		;$28-0 ERROR - illegal extension
	dc.l	serror		;$28-1 ERROR - illegal extension
	dc.l	serror		;$28-2 ERROR - illegal extension
	dc.l	serror		;$28-3 ERROR - illegal extension
	dc.l	serror		;$28-4 ERROR - illegal extension
	dc.l	serror		;$28-5 ERROR - illegal extension
	dc.l	serror		;$28-6 ERROR - illegal extension
	dc.l	serror		;$28-7 ERROR - illegal extension

	dc.l	serror		;$29-0 ERROR - illegal extension
	dc.l	serror		;$29-1 ERROR - illegal extension
	dc.l	serror		;$29-2 ERROR - illegal extension
	dc.l	serror		;$29-3 ERROR - illegal extension
	dc.l	serror		;$29-4 ERROR - illegal extension
	dc.l	serror		;$29-5 ERROR - illegal extension
	dc.l	serror		;$29-6 ERROR - illegal extension
	dc.l	serror		;$29-7 ERROR - illegal extension

	dc.l	serror		;$2a-0 ERROR - illegal extension
	dc.l	serror		;$2a-1 ERROR - illegal extension
	dc.l	serror		;$2a-2 ERROR - illegal extension
	dc.l	serror		;$2a-3 ERROR - illegal extension
	dc.l	serror		;$2a-4 ERROR - illegal extension
	dc.l	serror		;$2a-5 ERROR - illegal extension
	dc.l	serror		;$2a-6 ERROR - illegal extension
	dc.l	serror		;$2a-7 ERROR - illegal extension

	dc.l	serror		;$2b-0 ERROR - illegal extension
	dc.l	serror		;$2b-1 ERROR - illegal extension
	dc.l	serror		;$2b-2 ERROR - illegal extension
	dc.l	serror		;$2b-3 ERROR - illegal extension
	dc.l	serror		;$2b-4 ERROR - illegal extension
	dc.l	serror		;$2b-5 ERROR - illegal extension
	dc.l	serror		;$2b-6 ERROR - illegal extension
	dc.l	serror		;$2b-7 ERROR - illegal extension

	dc.l	serror		;$2c-0 ERROR - illegal extension
	dc.l	serror		;$2c-1 ERROR - illegal extension
	dc.l	serror		;$2c-2 ERROR - illegal extension
	dc.l	serror		;$2c-3 ERROR - illegal extension
	dc.l	serror		;$2c-4 ERROR - illegal extension
	dc.l	serror		;$2c-5 ERROR - illegal extension
	dc.l	serror		;$2c-6 ERROR - illegal extension
	dc.l	serror		;$2c-7 ERROR - illegal extension

	dc.l	serror		;$2d-0 ERROR - illegal extension
	dc.l	serror		;$2d-1 ERROR - illegal extension
	dc.l	serror		;$2d-2 ERROR - illegal extension
	dc.l	serror		;$2d-3 ERROR - illegal extension
	dc.l	serror		;$2d-4 ERROR - illegal extension
	dc.l	serror		;$2d-5 ERROR - illegal extension
	dc.l	serror		;$2d-6 ERROR - illegal extension
	dc.l	serror		;$2d-7 ERROR - illegal extension

	dc.l	serror		;$2e-0 ERROR - illegal extension
	dc.l	serror		;$2e-1 ERROR - illegal extension
	dc.l	serror		;$2e-2 ERROR - illegal extension
	dc.l	serror		;$2e-3 ERROR - illegal extension
	dc.l	serror		;$2e-4 ERROR - illegal extension
	dc.l	serror		;$2e-5 ERROR - illegal extension
	dc.l	serror		;$2e-6 ERROR - illegal extension
	dc.l	serror		;$2e-7 ERROR - illegal extension

	dc.l	serror		;$2f-0 ERROR - illegal extension
	dc.l	serror		;$2f-1 ERROR - illegal extension
	dc.l	serror		;$2f-2 ERROR - illegal extension
	dc.l	serror		;$2f-3 ERROR - illegal extension
	dc.l	serror		;$2f-4 ERROR - illegal extension
	dc.l	serror		;$2f-5 ERROR - illegal extension
	dc.l	serror		;$2f-6 ERROR - illegal extension
	dc.l	serror		;$2f-7 ERROR - illegal extension

	dc.l	ssincos		;$30-0 fsincos norm
	dc.l	ssincosz	;$30-1 fsincos zero
	dc.l	ssincosi	;$30-2 fsincos inf
	dc.l	ssincosnan	;$30-3 fsincos nan
	dc.l	ssincosd	;$30-4 fsincos denorm
	dc.l	serror		;$30-5 fsincos ERROR
	dc.l	serror		;$30-6 fsincos ERROR
	dc.l	serror		;$30-7 fsincos ERROR

	dc.l	ssincos		;$31-0 fsincos norm
	dc.l	ssincosz	;$31-1 fsincos zero
	dc.l	ssincosi	;$31-2 fsincos inf
	dc.l	ssincosnan	;$31-3 fsincos nan
	dc.l	ssincosd	;$31-4 fsincos denorm
	dc.l	serror		;$31-5 fsincos ERROR
	dc.l	serror		;$31-6 fsincos ERROR
	dc.l	serror		;$31-7 fsincos ERROR

	dc.l	ssincos		;$32-0 fsincos norm
	dc.l	ssincosz	;$32-1 fsincos zero
	dc.l	ssincosi	;$32-2 fsincos inf
	dc.l	ssincosnan	;$32-3 fsincos nan
	dc.l	ssincosd	;$32-4 fsincos denorm
	dc.l	serror		;$32-5 fsincos ERROR
	dc.l	serror		;$32-6 fsincos ERROR
	dc.l	serror		;$32-7 fsincos ERROR

	dc.l	ssincos		;$33-0 fsincos norm
	dc.l	ssincosz	;$33-1 fsincos zero
	dc.l	ssincosi	;$33-2 fsincos inf
	dc.l	ssincosnan	;$33-3 fsincos nan
	dc.l	ssincosd	;$33-4 fsincos denorm
	dc.l	serror		;$33-5 fsincos ERROR
	dc.l	serror		;$33-6 fsincos ERROR
	dc.l	serror		;$33-7 fsincos ERROR

	dc.l	ssincos		;$34-0 fsincos norm
	dc.l	ssincosz	;$34-1 fsincos zero
	dc.l	ssincosi	;$34-2 fsincos inf
	dc.l	ssincosnan	;$34-3 fsincos nan
	dc.l	ssincosd	;$34-4 fsincos denorm
	dc.l	serror		;$34-5 fsincos ERROR
	dc.l	serror		;$34-6 fsincos ERROR
	dc.l	serror		;$34-7 fsincos ERROR

	dc.l	ssincos		;$35-0 fsincos norm
	dc.l	ssincosz	;$35-1 fsincos zero
	dc.l	ssincosi	;$35-2 fsincos inf
	dc.l	ssincosnan	;$35-3 fsincos nan
	dc.l	ssincosd	;$35-4 fsincos denorm
	dc.l	serror		;$35-5 fsincos ERROR
	dc.l	serror		;$35-6 fsincos ERROR
	dc.l	serror		;$35-7 fsincos ERROR

	dc.l	ssincos		;$36-0 fsincos norm
	dc.l	ssincosz	;$36-1 fsincos zero
	dc.l	ssincosi	;$36-2 fsincos inf
	dc.l	ssincosnan	;$36-3 fsincos nan
	dc.l	ssincosd	;$36-4 fsincos denorm
	dc.l	serror		;$36-5 fsincos ERROR
	dc.l	serror		;$36-6 fsincos ERROR
	dc.l	serror		;$36-7 fsincos ERROR

	dc.l	ssincos		;$37-0 fsincos norm
	dc.l	ssincosz	;$37-1 fsincos zero
	dc.l	ssincosi	;$37-2 fsincos inf
	dc.l	ssincosnan	;$37-3 fsincos nan
	dc.l	ssincosd	;$37-4 fsincos denorm
	dc.l	serror		;$37-5 fsincos ERROR
	dc.l	serror		;$37-6 fsincos ERROR
	dc.l	serror		;$37-7 fsincos ERROR

	end
