rem usage:
rem		makeall [ makefile | release | local ]

make %1

cd dslib
make %1
cd ..

cd speed
make %1
cd ..

cd adisk
make %1
cd ..

cd misc
make %1
cd ..

cd bios
make %1
cd ..

cd clib
make %1
cd ..

cd ftrans
make %1
cd ..

cd include
make %1
cd ..

cd jlink
make %1
cd ..

cd pcboot
make %1
cd ..

cd keyboard
make %1
cd ..

cd mice
make %1
cd ..

cd atime
make %1
cd ..

