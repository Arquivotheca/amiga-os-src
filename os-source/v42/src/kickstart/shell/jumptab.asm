* jumptab.asm

	XDEF	@link_setenv
	XDEF	@link_endif
	XDEF	@link_endcli
	XDEF	@link_echo
	XDEF	@link_resident
	XDEF	@link_getenv
	XDEF	@link_ask
	XDEF	@link_else
	XDEF	@link_failat
	XDEF	@link_fault
	XDEF	@link_if
	XDEF	@link_path
	XDEF	@link_prompt
	XDEF	@link_quit
	XDEF	@link_skip
	XDEF	@link_why
	XDEF	@link_stack
	XDEF	@link_cd
	XDEF	@link_newshell
	XDEF	@link_run

	XREF	@cmd_setenv
	XREF	@cmd_endif
	XREF	@cmd_endcli
	XREF	@cmd_echo
	XREF	@cmd_resident
	XREF	@cmd_getenv
	XREF	@cmd_ask
	XREF	@cmd_else
	XREF	@cmd_failat
	XREF	@cmd_fault
	XREF	@cmd_if
	XREF	@cmd_path
	XREF	@cmd_prompt
	XREF	@cmd_quit
	XREF	@cmd_skip
	XREF	@cmd_why
	XREF	@cmd_stack
	XREF	@cmd_cd
	XREF	@cmd_newshell
	XREF	@cmd_run


	section text,code
		CNOP 0,4

@link_setenv:
		dc.l	0
		bra.w	@cmd_setenv

		CNOP 0,4
@link_endif:
		dc.l	0
		bra.w	@cmd_endif

		CNOP 0,4
@link_endcli:
		dc.l	0
		bra.w	@cmd_endcli

		CNOP 0,4
@link_echo:
		dc.l	0
		bra.w	@cmd_echo


		CNOP 0,4
@link_resident:
		dc.l	0
		bra.w	@cmd_resident

		CNOP 0,4

@link_cd:
		dc.l	0
		bra.w	@cmd_cd

		CNOP 0,4

@link_getenv:
		dc.l 0
		bra.w	@cmd_getenv

		CNOP 0,4
@link_ask:
		dc.l 0
		bra.w	@cmd_ask

		CNOP 0,4

@link_else:
		dc.l	0
		bra.w	@cmd_else

		CNOP 0,4

@link_failat:
		dc.l 	0
		bra.w	@cmd_failat

		CNOP 0,4

@link_fault:
		dc.l 	0
		bra.w	@cmd_fault

		CNOP 0,4
@link_if:
		dc.l 	0
		bra.w	@cmd_if

		CNOP 0,4
@link_path:
		dc.l 	0
		bra.w	@cmd_path

		CNOP 0,4
@link_prompt:
		dc.l 	0
		bra.w	@cmd_prompt

		CNOP 0,4
@link_quit:
		dc.l 	0
		bra.w	@cmd_quit

		CNOP 0,4
@link_skip:
		dc.l	0
		bra.w	@cmd_skip

		CNOP 0,4
@link_why:
		dc.l	0
		bra.w	@cmd_why

		CNOP 0,4
@link_stack:
		dc.l	0
		bra.w	@cmd_stack

		CNOP 0,4

@link_newshell:
		dc.l	0
		bra.w	@cmd_newshell

		CNOP 0,4

@link_run:
		dc.l	0
		bra.w	@cmd_run

		CNOP 0,4

		END
