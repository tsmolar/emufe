#include <SDL.h>
#include "sdl_allegro.h"

typedef struct pcxhead_t {
   Uint8 Identifier;        /* PCX Id (Always 0x0A) */
   Uint8 Version;           /* Version Number */
   Uint8 Encoding;          /* Encoding Format */
   Uint8 BitsPerPixel;      /* Bits per Pixel */
   Uint16 XStart;            /* Left of image */
   Uint16 YStart;            /* Top of Image */
   Uint16 XEnd;              /* Right of Image */
   Uint16 YEnd;              /* Bottom of image */
   Uint16 HorzRes;           /* Horizontal Resolution (Does this matter?) */
   Uint16 VertRes;           /* Vertical Resolution  (or this?) */
   Uint8 Palette[48];       /* EGA Palette 16 color */
   Uint8 Reserved1;         /* Reserved (Always 0) */
   Uint8 NumBitPlanes;      /* Number of Bit Planes */
   Uint16 BytesPerLine;      /* Bytes per Scan-line */
   Uint16 PaletteType;       /* Palette Type */
   Uint16 HorzScreenSize;    /* Horizontal Screen Size */
   Uint16 VertScreenSize;    /* Vertical Screen Size */
   Uint8 Reserved2[54];     /* Reserved (Always 0) */
} pcxhead_t;

int pdc;

int save_pcx_pf(FILE *pf, BITMAP *bm, RGB *pal) {
   // Write 24-bit images only
   pcxhead_t ph;
   int i,cx,cy,r,g,b,rc,px,vl,svl;
   int tc=0;
   
   ph.Identifier=0x0a;
   ph.Version=5;
   ph.Encoding=1;
   ph.HorzRes=0; ph.VertRes=0;
   ph.Reserved1=0;
   ph.HorzScreenSize=bm->w;
   ph.VertScreenSize=bm->h;
   ph.BitsPerPixel=bm->format->BitsPerPixel;
   ph.XStart=ph.YStart=0;
   ph.XEnd=bm->w-1;
   ph.YEnd=bm->h-1;
   ph.PaletteType=2;
   for(i=0;i<48;i++) ph.Palette[i]=0;
   for(i=0;i<54;i++) ph.Reserved2[i]=0;
   if(ph.BitsPerPixel>8) {
      ph.BitsPerPixel=8; // this is what imagemagick does
      ph.NumBitPlanes=3; // this is what imagemagick does
      ph.BytesPerLine=(ph.BitsPerPixel * bm->w)/8; // I think this is how
                                                   // it's calc'd
      tc=1;
   } else {
      printf("warning: sa_writepcx: indexed images not yet supported!\n");
      return 1;
   }
   
   if(fwrite(&ph, sizeof(char), sizeof(pcxhead_t), pf) !=
      sizeof(pcxhead_t))
     printf("Error writing PCX header!\n");
   if(tc==1) {
      for(cy=ph.YStart;cy<=ph.YEnd;cy++) {
	 for(i=0;i<3;i++) {
	    cx=ph.XStart;
	    rc=0;
	    svl=-1;
	    for(cx=0;cx<=ph.XEnd;cx++) {
	       px=getpixel(bm,cx,cy);
	       SDL_GetRGB(px,bm->format,&r,&g,&b);
	       // sometimes the rgb's need to be reversed to get the colors
	       // right.   Why?
	       switch(i) {
		case 0: vl=r; break;
		case 1: vl=g; break;
		case 2: vl=b; break;
	       }
	       if(vl==svl) {
		  rc++;
		  if(rc>63) {
		     fputc(255,pf);
		     fputc(svl,pf);
		     rc=1;
		  }
	       } else {
		  if(rc!=0) {
//		     if(0xC0 != (0XC0 & svl))
		     if ((rc != 1) || (0xC0 == (0xC0 & svl)))
		       fputc(192+rc,pf);
		     fputc(svl,pf);
		  }
		  svl=vl;
		  rc=1;
	       }
	    } // for cx
	    if(rc!=0) {  // store last value
	       if ((rc != 1) || (0xC0 == (0xC0 & svl)))
		 fputc(192+rc,pf);
	       fputc(svl,pf);
	    }
	 } // for i (bitplanes)
      } // for cy
   } // if tc==1
//   fclose(pf);   
   return 0;
}

int save_pcx(const char *fname, BITMAP *bm, RGB *pal) {
   FILE *fp;
   int r;

   printf("writepcx called! %s\n",fname);
   
   fp=fopen(fname,"wb");
   r=save_pcx_pf(fp,bm,pal);
   fclose(fp);
   return r;
}

int sa_getpcxline(Uint8 *linebuf, FILE *pf, int w) {
   int cx,dx,rleh;
   Uint8 pixd;
   cx=0;
   while(1) {
      rleh=fgetc(pf); pdc++;
      
      if((rleh & 0xC0)==0xC0) {
//	 printf("PCX 1st: %d\n", rleh);
	 rleh=rleh & 0x3F;
//	 printf("PCX 1st n: %d:%d  (%d)\n", rleh,cx,cx+rleh);
	 pixd=fgetc(pf);
	 pdc++;
//	 printf("PCX 2nd: %d\n", pixd);
//	 if((cx+rleh)>=w) {
//	    linebuf[w+1]=(cx+rleh)-w;
//	    linebuf[w+2]=pixd;
//	 } else
//	   linebuf[w+1]=0;
	 for(dx=cx;dx<cx+rleh;dx++) {
	    linebuf[dx]=pixd;
	    if(dx>=w) break;
	 }
	 cx=cx+rleh;
      } else {
	 pixd=rleh;
	 rleh=1;
//	 printf("PCX 1st n: %d:%d  (%d)\n", rleh,cx,cx+rleh);
//	 printf("PCX 2nd: %d\n", pixd);
	 linebuf[cx]=pixd;
	 cx++;
      }
      if(cx>=w) {
	 if(cx>w) printf("cx is now %d \n",cx);
//	 printf("overflow:  len:%d  pix:%d\n",linebuf[w+1],linebuf[w+2]);
	 break;
      }
   } // while
}

BITMAP *sa_readpcx_pf(FILE *pf, RGB *pal) {
   BITMAP *b;
   pcxhead_t ph;
//   int rleh, pixd;
   int cx,cy, c;
   // int orrl=0, orpx=0, ogrl=0, ogpx=0, obrl=0, obpx=0;
   Uint8 *rbuf, *gbuf, *bbuf;
   // read header
   pdc=0;
   if(fread(&ph, sizeof(char), sizeof(pcxhead_t), pf) !=
      sizeof(pcxhead_t))
     printf("Error reading PCX header.\n");
   pdc=pdc+sizeof(pcxhead_t);
   // fix for the gimp
   if(ph.VertScreenSize==0) ph.VertScreenSize=ph.YEnd+1;
   if(ph.HorzScreenSize==0) ph.HorzScreenSize=ph.XEnd+1;

   if(ph.Version==5) {
      rbuf=(Uint8 *)malloc(ph.BytesPerLine+5);
      gbuf=(Uint8 *)malloc(ph.BytesPerLine+5);
      bbuf=(Uint8 *)malloc(ph.BytesPerLine+5);

      printf("b=create_bitmap(%d,%d);\n",ph.BytesPerLine,ph.VertScreenSize);
      b=create_bitmap(ph.BytesPerLine,ph.VertScreenSize);
      for(cy=0;cy<ph.VertScreenSize;cy++) {
	 sa_getpcxline(rbuf, pf, ph.BytesPerLine);
	 sa_getpcxline(gbuf, pf, ph.BytesPerLine);
	 sa_getpcxline(bbuf, pf, ph.BytesPerLine);
	 for(cx=0;cx<ph.BytesPerLine;cx++) {
	    c=makecol(rbuf[cx],gbuf[cx],bbuf[cx]);
	    putpixel(b,cx,cy,c);
	 }
      } // for cy
      free(rbuf);
      free(gbuf);
      free(bbuf);
      printf("PCX loader: %d bytes loaded\n",pdc);
   } // if version
   return b;
}

BITMAP *sa_readpcx(const char *filename, RGB *pal) {
   FILE *fp;
   BITMAP *b;

   printf("readpcx called! %s\n",filename);
   
   fp=fopen(filename,"rb");
   b=sa_readpcx_pf(fp,pal);
   fclose(fp);
   printf("PCX loader: %d bytes loaded in file %s\n",pdc,filename);
   return b;
}

sa_pcxinfo(const char *filename) {
   // only reads and displays the header of an existing PCX
   pcxhead_t ph;
   FILE *fp;
   
   fp=fopen(filename,"rb");
   if(fread(&ph, sizeof(char), sizeof(pcxhead_t), fp) !=
      sizeof(pcxhead_t))
     printf("Error reading PCX header.\n");
   fclose(fp);
   printf("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");
   printf("PCX HEADER FOLLOWS\n");
   printf("ID: 0x%02x\n",ph.Identifier);
   printf("Version: %d\n",ph.Version);
   printf("Encoding: %d\n",ph.Encoding);
   printf("Bits: %d\n",ph.BitsPerPixel);
   printf("Size:  start:(%d,%d)  end:(%d,%d)\n",ph.XStart,ph.YStart,ph.XEnd,ph.YEnd);
   printf("Screen Res: %d,%d\n",ph.HorzRes,ph.VertRes);
   printf("Bit Planes: %d\n",ph.NumBitPlanes);  
   printf("Bytes Per Line: %d\n",ph.BytesPerLine);
   printf("Palette Type: %d\n",ph.PaletteType);
   printf("Screen Size: %d,%d\n",ph.HorzScreenSize,ph.VertScreenSize);
   printf("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");
}

