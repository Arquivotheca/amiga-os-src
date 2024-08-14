#include <stdio.h>
#define read rstat = fread(&b, 1, 1, bs); bcount++;
#define vread rstat = fread(&b, 1, 1, bs); bcount++; vbcount++; vb<<=8; vb+=b;
#define aread rstat = fread(&b, 1, 1, bs); bcount++; abcount++; ab<<=8; ab+=b;


FILE *bs, *af;
int bcount=0, abcount=0, vbcount=0;
unsigned char b;
int rstat = 1;
double audiot, videot;
unsigned int vb = 0x55555555;
unsigned int ab = 0x55555555;
double t, vt, dvt, dat;
double dat_min=0, dat_max=0;
double dvt_min=0, dvt_max=0;

main(argc, argv)
int argc;
char **argv;
{

int zerocount=0, bpack=4;
int pack_count=0, apckt_count=0, vpckt_count=0;
unsigned int ut1;
unsigned int packet_length, data_bytes;

if(argc != 3) {
	fprintf(stderr, "USAGE:  %s system_stream audio_stream\n", argv[0]);
	fprintf(stderr, "extracts audio stream from system stream\n");
	exit(0);
	}

if((bs = fopen(argv[1], "r")) == NULL) {
	fprintf(stderr, "Cant open bitstream file: %s\n", argv[1]);
	exit(0);
	}

if((af = fopen(argv[2], "w")) == NULL) {
	fprintf(stderr, "Cant open audio output file: %s\n", argv[2]);
	exit(0);
	}


while (fread(&b, 1, 1, bs) == 1) {
	bcount++;
	if(b==0)
		zerocount++;
	else if((zerocount >= 2) & (b == 1)) {
		fread(&b, 1, 1, bs);
		bcount++;
		zerocount = 0;
		printf("%8d\t", bcount);
		printf("000001");
		printf("%02x\t", b);
		if(b == 0xb4)
			printf("MPEG ERROR code\n");
		else if(b == 0xB5)
			printf("extension start code\n");
		else if(b == 0xB9)
			printf("iso_11172_end_code\n");
		else if(b == 0xBA) {
			pack_count++;
			printf("pack %d\t%d from last", pack_count, bcount-bpack);
			bpack = bcount;
			t = (float) pack_count/75.0;
			printf("\t t=%f\n", t);
			}
		else if(b == 0xbb)
			printf("system header\n");
		else if(b == 0xbc)
			printf("reserved stream packet\n");
		else if(b == 0xbd)
			printf("private stream 1 packet\n");
		else if(b == 0xbe)
			printf("padding stream packet\n");
		else if(b == 0xbf)
			printf("private stream 2 packet\n");
		else if((b & 0xE0) == 0xC0) {
			apckt_count++;
			printf("audio packet %d (stream %d)\n", apckt_count, (b & 0x1F));
			read
			packet_length=b;
			packet_length <<=8;
			read
			packet_length+=b;
			printf("\t\tpacket_length=%d\t", packet_length);
			ut1 = bcount;

			read
			while (b == 0xFF) {
				read
				printf("stuff ");
			}

			if((b&0xC0)==0x40) {
				read
				read
				printf("STD ");
			}

			if((b&0xF0)==0x20) {
				read
				read
				read
				read
				printf("pts\t");
			}
			else if ((b&0xF0)==0x30) {
				read
				read
				read
				read
				read
				read
				read
				read
				read
				printf("pts DTS\t");
			}
			else if (b==0x0F) {}
			else
				printf("!!!!! Packet header error!!!\n");
			data_bytes = packet_length - (bcount - ut1);
			printf("audio pkt bytes = %d\n", data_bytes);
			aparse(data_bytes);

			}
		else if((b & 0xF0) == 0xE0) {
			vpckt_count++;
			printf("video packet %d (stream %d)\n", vpckt_count, (b & 0x0F));
			read
			packet_length=b;
			packet_length <<=8;
			read
			packet_length+=b;
			printf("\t\tpacket_length=%d\t", packet_length);
			ut1 = bcount;

			read
			while (b == 0xFF) {
				read
				printf("stuff ");
			}

			if((b&0xC0)==0x40) {
				read
				read
				printf("STD ");
			}

			if((b&0xF0)==0x20) {
				read
				read
				read
				read
				printf("pts\t");
			}
			else if ((b&0xF0)==0x30) {
				read
				read
				read
				read
				read
				read
				read
				read
				read
				printf("pts DTS\t");
			}
			else if (b==0x0F) {}
			else
				printf("!!!!! Packet header error!!!\n");
			data_bytes = packet_length - (bcount - ut1);
			printf("video pkt bytes = %d\n", data_bytes);
			vparse(data_bytes);

			}
		else if((b & 0xF0) == 0xF0)
			printf("reserved data stream #%d packet\n", (b & 0x0F));
		else
			printf("unknown\n");
	}
	else
		zerocount = 0;
}
printf("dat_min=%f \t dat_max=%f\t dt=%f\n", dat_min, dat_max,dat_max-dat_min);
printf("dvt_min=%f \t dvt_max=%f\t dt=%f\n\n", dvt_min, dvt_max,dvt_max-dvt_min);
printf("%d bytes audio\n", abcount);
printf("%d bytes video\n", vbcount);
printf("%d bytes read\n", bcount);
fclose(af);
fclose(bs);
}


float pic_rate(a)
unsigned char a;
{
	if (a == 1)
		return(23.976);
	else if(a == 2)
		return(24.0);
	else if(a == 3)
		return(25.0);
	else if(a == 4)
		return(29.97);
	else if(a == 5)
		return(30.0);
	else if(a == 6)
		return(50);
	else if(a == 7)
		return(59.94);
	else if(a == 8)
		return(60);
	else
		return(9999.9);
}



vparse(n)
int n;
{
int i;
static int pic_count=0, gop_count=0, slice_count=0;
unsigned int ut1, ut2;
static float fps = 23.976;

for(i=0; i<n; i++) {
        vread

	if(vb == 0x00000100) {
		pic_count++;
		slice_count = 0;
		printf("%8d\t %08x\t", bcount, vb);
		printf("picture %d", pic_count);
		vt=(float)pic_count/fps;
		dvt=vt-t;
		printf("t=%f   dt=%f\n", vt, dvt);
		if(dvt > dvt_max)
			dvt_max=dvt;
		if(dvt < dvt_min)
			dvt_min=dvt;
		}
	else if(vb == 0x000001B8) {
		gop_count++;
		printf("%8d\t %08x\t", bcount, vb);
		printf("GOP %d\n", gop_count);
		}
	else if(vb == 0x000001b2) {
		printf("%8d\t %08x\t", bcount, vb);
		printf("USER DATA\n");
		}
	else if(vb == 0x000001B3) {
		printf("%8d\t %08x\t", bcount, vb);
		printf("video sequence header");

		if(i < (n-7)) {
			i+=7;
			vread
			vread
			vread
			ut1 = (vb >> 12) & 0xFFF;
			ut2 = vb & 0xFFF;
			printf("  (%d x %d)\n", ut1, ut2);

			vread
			b &= 0x0F;
			fps = pic_rate(b);
			printf("\t\t %f fps", fps);

			vread
			vread
			vread
			ut1 = (vb >> 6) & 0x3FFFF;
			printf("\t %d bps\n", ut1*400);
			}
		}
	else if(vb == 0x000001b4) {
		printf("%8d\t %08x\t", bcount, vb);
		printf("MPEG ERROR code\n");
		}
	else if(vb == 0x000001B5) {
		printf("%8d\t %08x\t", bcount, vb);
		printf("extension start code\n");
		}
	else if(vb == 0x000001b7) {
		printf("%8d\t %08x\t", bcount, vb);
		printf("video sequence end code\n");
		}
	else if((vb >= 0x00000101) && (vb <= 0x000001af)) {
		slice_count++;
		printf("%8d\t %08x\t", bcount, vb);
		printf("slice %d\n", slice_count);
		}
	else if((vb == 0x000001b0) || (vb == 0x000001b1) || (vb == 0x000001b6)) {
		printf("%8d\t %08x\t", bcount, vb);
		printf("reserved MPEG video\n");
		}
	else if((vb & 0xFFFFFF00) == 0x00000100) {
		printf("%8d\t %08x\t", bcount, vb);
		printf("unknown start code !!!!\n");
		}
}

}




aparse(n)
int n;
{
static int abcountp=2;
static int aucount=0;
unsigned int ut1, ut2, i;



for(i=0; i<n; i++) {
	aread
	fwrite(&b, 1, 1, af);
	if(((ab&0x0000FFF0)==0x0000FFF0 || (ab&0x00000FFF)==0x00000FFF)
		&& ((abcount-abcountp)>600)) {

			aucount++;
			/*ut1 = ((abcount-abcountp)*44100*8)/1152;
			ut2 = ((abcount-2)*(44100.0*8/1152))/aucount;
			printf("%4d  %9d  %4d   ", aucount, abcount, abcount-abcountp);
			printf("%7d bps au   %7d total\n", ut1, ut2); */
			if(((abcount-abcountp) != 626) && ((abcount-abcountp) != 627))
				printf("!!!!!!! audio au size = %d\n", abcount-abcountp);
			printf("%8d\t", abcount);
			audiot = aucount * 0.026122449;
			dat = audiot -t;
			printf("audio au %5d\t %f sec    %f dsec\n", aucount, audiot, dat);
			if(dat > dat_max)
				dat_max = dat;
			if(dat < dat_min)
				dat_min = dat;
			abcountp=abcount;
			}
		}
}


