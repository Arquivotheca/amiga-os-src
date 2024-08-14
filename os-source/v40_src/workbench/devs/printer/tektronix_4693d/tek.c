/* Tektronix 4693D special support functions - Bill Koester */

#include "TEK4693D.h"

/************************************************************************
 *
 * Make_Preamble
 *
 *    Forms the Preamble in the char array pointed to by buf, computes
 *    the length and adds the checksum.
 *
 ************************************************************************/

Make_Preamble(buf,ppl,lines,comm_mode,copies,pixel_size,size_num,pixel_encode,
                  pixel_order,image_size,image_render,color_conv,
                  image_manip,BW_inversion,orientation,printer)
char *buf;
int   ppl,
      lines,
      comm_mode,
      copies,
      pixel_size,
      size_num,
      pixel_encode,
      pixel_order,
      image_size,
      image_render,
      color_conv,
      image_manip,
      BW_inversion,
      orientation,
      printer;
{
   int len;
   char *p;

   len = 0;
   p = buf;

   *(p++) = 0x14;                /* print request command */
   p++;                          /* leave room for length byte */
   if(ppl <= 63)                 /* pixels per line            */
   {
      *(p++) = ppl | 0xc0;
   } else {
      *(p++) = ((ppl>>6)&63)|0xc0;
      *(p++) = (ppl&63)|0x80;
   }
   if(lines <= 63)               /* number of lines            */
   {
      *(p++) = lines | 0xc0;
   } else {
      *(p++) = ((lines >> 6) & 63) | 0xc0;
      *(p++) =  (lines & 63) | 0x80;
   }
   *(p++) = comm_mode;           /* communication mode         */
   if(copies <= 63)              /* number of copies           */
   {
      *(p++) = copies | 0xc0;
   } else {
      *(p++) = ((copies >> 6) & 63) | 0xc0;
      *(p++) =  (copies & 63) | 0x80;
   }
   *(p++) = pixel_size;          /* pixel size                 */
   if(pixel_size == PIXEL_REC)
   {
      if(size_num <= 63)              /* number of copies           */
      {
         *(p++) = size_num | 0x80;
      } else {
         *(p++) = ((size_num >> 6) & 63) | 0x80;
         *(p++) =  (size_num & 63) | 0x80;
      }
   }

   *(p++) = pixel_encode;        /* pixel encoding             */
   *(p++) = pixel_order;         /* pixel ordering             */
   *(p++) = image_size;          /* image sizing               */
   *(p++) = image_render;        /* image rendering            */
   *(p++) = color_conv;          /* color conversion           */
   *(p++) = image_manip;         /* image manipulation         */
   *(p++) = BW_inversion;        /* black white inversion      */
   *(p++) = orientation;         /* image orientation          */
                                 /* printer type               */
   *(p++) = (unsigned char)((printer>>8)&0xff);
   *(p++) = (unsigned char)(printer&0xff);
   len = (p-buf)|0xc0;           /* compute preamble length    */
   buf[1] = len;                 /* store length               */
   *(p++) = CheckSum(buf,len&63);/* add cksum to preamble      */
   *p = 0x02;                    /* preamble terminator        */
   return(0);
}
/* Compute preamble checksum */
CheckSum(buf,len)
char *buf;
int len;
{
   int i,cksum;

   cksum = 0;
   for(i=0;i<len;i++)
      cksum = (cksum + buf[i])%128;
   cksum |= 128;
   return(cksum);
}
