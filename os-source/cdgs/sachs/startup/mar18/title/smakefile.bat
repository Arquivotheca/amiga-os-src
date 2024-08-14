slink LIB:c.o tester_t.o title_t.o vblank.o color.o task.o sound.o decompdltax.o star.o  	sprite.o dltax.o pal1.o sample.o back.o back3plane.o number.o version.o to title BATCH NOICONS LIB LIB:debug.lib LIB:sc.lib LIB:amiga3.0.lib
bumpit -h -!
sc >nil: CPU=68020 STREQ NOSTKCHK CPU=ANY OPTSIZE STREQU IDIR=INCLUDE version.c
touch >nil: title
net
slink title to net:work/title NODEBUG
