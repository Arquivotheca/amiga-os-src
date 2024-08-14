all: BuildNIPCPrefs BuildSMPrefs

BuildNIPCPrefs : BuildNIPCPrefs.o
	blink BuildNIPCPrefs.o to BuildNIPCPrefs lib lib:lc.lib lib:amiga.lib lib:debug.lib NOICONS

BuildNIPCPrefs.o : BuildNIPCPrefs.c
	lc -v -b0 BuildNIPCPrefs.c

%.c:

BuildSMPrefs : BuildSMPrefs.o
	blink BuildSMPrefs.o to BuildSMPrefs lib lib:lc.lib lib:amiga.lib lib:debug.lib NOICONS

BuildSMPrefs.o : BuildSMPrefs.c
	lc -v -b0 BuildSMPrefs.c