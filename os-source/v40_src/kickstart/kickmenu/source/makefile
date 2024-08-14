#
#       Native makefile
#	modified for AMAKE by Kevin Klop	10/10/90
#
#
KICKPATH=v:disk/kickstart/3000/kick.$(REV).f8

superkickstart : bonus13.rom bonus14.rom 1.3.bryce $(KICKPATH)
	@'list $(KICKPATH)'
	@say ""
	@'makekick/makekick.ld df0: 1.3.bryce $(KICKPATH) bonus13.rom bonus14.rom'
	@'date >superkickstart.datestamp'

bonus13.rom :        V:src/kickstart/scsidisk/scsidisk.ld
#	-'delete bonus13/scsidisk.ld.old'
	@'copy bonus13/scsidisk.ld bonus13/scsidisk.ld.old'
	@'copy V:src/kickstart/scsidisk/scsidisk.ld bonus13/scsidisk.ld'
	@'execute bonus13/script'

bonus14.rom :    bonus14/bonus14.rom
	@'copy bonus14/bonus14.rom ""'

1.3.bryce :
	@'copy makekick/kf/1.3.bryce ""'

$(KICKPATH) :
	@say ""
	@say "This makefile takes a kickstart release number (make REV=253)"
	@say ""

clean :
	@'delete #?.rom'
