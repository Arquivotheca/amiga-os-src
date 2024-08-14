amouse2.obj: amouse2.asm
	masm amouse2;

amouse2.drv: amouse2.obj amouse2.def
	link4 amouse2,,,,amouse2.def
	copy amouse2.dll amouse2.drv
