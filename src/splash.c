#include<stdio.h>
#include "dfilepath.h"

#ifdef USEALLEGRO
#include <allegro.h>
#endif

#ifdef USESDL
#include<SDL.h>
#include"sdl_allegro.h"
#endif

// Version 2.2
// * New layout
// Version 2.1
// Changes from last version:
// * use dfilepath.c

BITMAP *bitmap;
BITMAP *lhlbmp, *ehlbmp, *lprbmp, *eprbmp, *laubmp, *exibmp;
char rcfilename[160],basedir[120],fname[180],bindir[120];
extern char mysep;

char cdroot[220];

void find_bindir(char *ddir, char *bin) {
   char path[160],dirname[160];
      
   abs_dirname(path,bin);
   if(strcmp(path,"")==0) {
      strcpy(path,INSTPREFIX);
   }
   strcpy(ddir,path);
}

void set_bg() {
//    Set background 
   
   char fname[90];
   PALETTE p; 
   get_palette(p);
   sprintf(fname,"%s/pics/emusplash.pcx",basedir);
   
   bitmap=load_bitmap(fname,p);
   if(bitmap) {
      blit(bitmap,screen,0,0,0,0,300,400);
      destroy_bitmap(bitmap);
   }      
}   

void ch_launch(BITMAP *x) 
{
   scare_mouse();
//   masked_blit(x,screen,0,0,24,268,86,27);
   masked_blit(x,screen,0,0,24,204,102,23);
   unscare_mouse();
}

void ch_exit(BITMAP *x) 
{
   scare_mouse();
//   masked_blit(x,screen,0,0,24,304,86,27);
   masked_blit(x,screen,0,0,24,239,102,23);
   unscare_mouse();
}

int load_buttons() {
   
   // These were written using an older version of dfixsep2() that
   // didn't require a third parameter.  I am assuming the third
   // parameter should be zero.  If problems occur, consider changing
   // it to 1
   
   PALETTE p; 
   get_palette(p);
   char *pbmp,tmpstr[200];

   pbmp=(char *)malloc(180);
   sprintf(tmpstr,"%s%cpics%claunch_hl.pcx", basedir,mysep,mysep);
   dfixsep2(pbmp,tmpstr,0);
#ifdef DEBUG
   printf("load_dfltimg: loading %s\n",pbmp);
#endif    
   lhlbmp=load_bitmap(pbmp,p);
   sprintf(tmpstr,"%s%cpics%cexit_hl.pcx", basedir,mysep,mysep);
   dfixsep2(pbmp,tmpstr,0);
   ehlbmp=load_bitmap(pbmp,p);
   sprintf(tmpstr,"%s%cpics%claunch_pr.pcx", basedir,mysep,mysep);
   dfixsep2(pbmp,tmpstr,0);
   lprbmp=load_bitmap(pbmp,p);
   sprintf(tmpstr,"%s%cpics%cexit_pr.pcx", basedir,mysep,mysep);
   dfixsep2(pbmp,tmpstr,0);
   eprbmp=load_bitmap(pbmp,p);
   sprintf(tmpstr,"%s%cpics%claunch_bu.pcx", basedir,mysep,mysep);
   dfixsep2(pbmp,tmpstr,0);
   laubmp=load_bitmap(pbmp,p);
   sprintf(tmpstr,"%s%cpics%cexit_bu.pcx", basedir,mysep,mysep);
   dfixsep2(pbmp,tmpstr,0);
   exibmp=load_bitmap(pbmp,p);
}

void init() {
   int w;
   allegro_init();
   install_mouse();
   install_keyboard();
   set_color_depth(16);
   
   w=set_gfx_mode(GFX_AUTODETECT_WINDOWED,300,400,0,0);
   load_buttons();  /* Load buttons */
//   w=set_gfx_mode(GFX_AUTODETECT_WINDOWED,640,480,0,0);
   if(w!=0) {
#ifdef DEBUG
      printf("Could not set graphics mode!\n");
#endif
      exit(34);
   }
}

void ef_shutdown() {
/*   allegro_exit(); */
   exit(0);
}

int main(int argc, char* argv[]) {
   long w;
   int keyp, my, mp, entidx, pslc, zpos, menf;
   int lmode, emode;
   char drv;
   
   strcpy(cdroot,"");
   if(getenv("CDROOT"))
     strcpy(cdroot,getenv("CDROOT"));
   lmode=emode=0;
//   strcpy(rcfilename,argv[0]); // not needed?
   find_bindir(bindir,argv[0]);
   // why do we need to set cdroot like this, is it right?
//     strcpy(cdroot,bindir);
   find_datadir(basedir,argv[0]);
#ifdef DEBUG
   printf("BASEDIR: %s\n",basedir);
   printf("cdroot: %s\n",cdroot);
   printf("BINDIR: %s\n",bindir);
   printf("argv0: %s\n",argv[0]);
#endif
#ifdef WIN32
//   drv=argv[0][0];
//   sprintf(basedir,"%c:\\emucd",drv);
//   sprintf(fname,"%c:\\cygwin\\emulator.bat",drv);
//   sprintf(fname,"%srunme.sh",basedir,mysep);
   sprintf(fname,"%s%cemufe.exe -n -i -ac -c",bindir,mysep);
#else
//   sprintf(basedir,"/usr/src/keep/emufe");
   sprintf(fname,"%s%cbin%cemufe -n -i -ac",bindir,mysep,mysep);
#endif
#ifdef DEBUG
   printf("fname: %s\n",fname);
#endif
//   exit(1);
   init();
   zpos=mouse_z;
   
   set_bg();
   clear_keybuf(); 
   show_mouse(screen);

   while(1) {
      rest(0);
      my=mouse_y;
     
/* LAUNCH BUTTON */
      if (mouse_x > 24 && mouse_x < 110 && my > 203 && my < 228) {
	 if ( lmode == 0 && mp == 0) {
	    ch_launch(lhlbmp);
	    lmode=1;
	 }
	 if ( lmode == 2 && mp == 0) {
	    ch_launch(lhlbmp);
	    lmode=1;
#ifdef DEBUG
	    printf("Execute:  %s\n", fname);
#endif
	    w=set_gfx_mode(GFX_TEXT,300,400,0,0);
	    system(fname);
	    ef_shutdown();
	 }
	 if ( mp == 1 && lmode != 2 ) {
	    ch_launch(lprbmp);
	    lmode=2;
	 }
      } else {
	 if ( lmode != 0 ) 
	   {
	      ch_launch(laubmp);
	      lmode=0;
	   }
      }
      
      /* EXIT Button */
      if (mouse_x > 24 && mouse_x < 110 && my > 238 && my < 263) {
	 if ( emode == 0 && mp == 0) {
	    ch_exit(ehlbmp);
	    emode=1;
	 }
	 if ( emode == 2 && mp == 0) {
	    ch_exit(ehlbmp);
	    emode=1;
	    ef_shutdown();
	 }
	 if ( mp == 1 && emode != 2 ) {
	    ch_exit(eprbmp);
	    emode=2;
	 }
      } else {
	 if ( emode != 0 ) {
	    ch_exit(exibmp);
	    emode=0;
	 }
      }
      
      if(!mouse_b) { mp=0; } else { mp=1; }
      

      if(keypressed()) {
	 keyp=readkey() >> 8; 
	 /*	 keyp=readkey(); 
	  printf("k=%d\n",keyp); */
	 if(keyp==85) {
	    /* Down Arrow */
	    printf("nothing");
	 }
      }
   } /* while */
   ef_shutdown();
//   return(0);
}
