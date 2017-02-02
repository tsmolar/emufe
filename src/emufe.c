#include <stdio.h>
#ifdef USESDL
#include <SDL.h>
#include "sdl_allegro.h"
#endif

#ifdef USEALLEGRO
#include <allegro.h>
#endif

#include <string.h>
#include <time.h>
#include "font.h"
#include "font_legacy.h"
#include "dfilepath.h"
#include "fileio.h"
#include "emufe.h"
// #include "fileio.h"

/* #include <allegro/internal/aintern.h> */

// debug level can be 0-5
#define DEBUG_LEVEL 3

BITMAP *bitmap, *descmap;
BITMAP *defbmp;
BITMAP *menumap, *titlemap;
BITMAP *selection;
// shouldn't these be in imgbx_t?
BITMAP *imgbx_bmp[12];
BITMAP *imgbx_ovl[12];
int in_gfxmode=0, jflag=0;

fnt_t* LoadedFont;
fnt_t* myttf;

// Note, this might ultimately be used for the menu, but not now
typedef struct menu_t {
   char type;
   char name[42];
   char rom[28];
} menu_t;


menuinfo_t imenu;
char cdroot[220];
prop_t rc;
imgbox_t imgbx[12];
menu_t menu[600];
char dirname[120], bgpic[90], titlebox[40], picsdir[90], menuname[20], rcfilename[20], defimg[20];
char *commands[30], *lmenus[30];
char descdir[90], descbox[40], menubox[40], picbox[40], theme[200], gthemedir[96], fullscr;
char tfontbmp[30],fullpath[120];
char basedir[160], restr[20];
char startdir[160], lastitem[160], *fname, debugtxt[300];
int menulength, usembmap, usedbmap;
/* FORCE picload */
int cdclock=0;
int joy_enable=1;
int minx=640, miny=480, usex=640, usey=480;
int rx0=0, ry0=0; // Where screen starts
time_t starttime, endtime;
extern char mysep;

/* Counters for debug purposes 
nt menuload, menudisp;
menuload=0;
menudisp=0;
*/

// #ifdef USESDL
//gfx_sdlflip() {
// # ifdef SDL1
//   SDL_Flip(screen);
//# else 
//   s2a_flip(screen);
//# endif
//}
//#endif

void debug(int level,char *text) {
   // log debugging in a way that can be switched off at ru
   // Should switch to new macro
   if (level <= DEBUG_LEVEL)
     printf("XLOG(%d): %s\n",level,text);
}

// debug print macro
#ifdef DEBUG
//# define LOG(level, x) ( level <= DEBUG_LEVEL ) ? 0 : printf x
# define LOG(level, x) do { if ( level <= DEBUG_LEVEL ) { printf("log(%d): ",level); printf x;} } while(0)
#else
# define LOG(level, x)
#endif

int st_txt_col(char etype) {
   /* Return menu txt color based on entry type,  colors can eventually
    * be defined in the rc file */
   /* Shortcut warning!  Instead of calling set_font_fcolor() like we
    * should, we set fnfgcol directly.*/
   switch(etype) {
    case 'm':
      fnfgcol=makecol16(textfgr,textfgg,textfgb);
      break;
    case 'd':
      fnfgcol=makecol16(textfgr,textfgg,textfgb);
      break;
    case 'a':
      fnfgcol=makecol16(textier,textieg,textieb);
      break;
    case 'i':
      fnfgcol=makecol16(textier,textieg,textieb);
      break;
    case 's':
      fnfgcol=makecol16(255,255,255);
      break;
    case 'N':
      fnfgcol=makecol16(255,255,255);
      break;
    case 'e':
      fnfgcol=makecol16(80,64,16);
      break;
    case ' ':
      fnfgcol=makecol16(80,64,16);
      break;
    case 'u':
      fnfgcol=makecol16(0,64,80);
      break;
   }   
}

int restore_menuback() {
   int black, white, gray128;
   
   scare_mouse();
   if(usembmap==0) {
      black=makecol16(0,0,0); 
      gray128=makecol16(128,128,128); 
      bbox(rc.mb_x+rx0, rc.mb_y+ry0, rc.mb_x2+rx0, rc.mb_y2+ry0, gray128, black);
   } else {
     blit(menumap, screen,0,0,rc.mb_x+rx0,rc.mb_y+ry0,rc.mb_w,rc.mb_h);
   }      
   unscare_mouse();
}

int menu_uhlight(int index, int slct) {

   /* Unhilight the menu selection */
   
   int offset;
   
   offset=(slct-index)+1;
/*   set_font_fcolor(0,0,0); */
   st_txt_col(menu[slct].type);
   set_font_bcolor(textbgr,textbgg,textbgb);
   scare_mouse();
   if(usembmap==1) {
      blit(menumap, screen,0,(offset*rc.font_h)-13,rc.mb_x+rx0,ry0+rc.mb_y-13+(offset*rc.font_h),rc.mb_w,rc.font_h);
      show_string((rc.mb_x+10)+rx0,(rc.mb_y-13)+(offset*rc.font_h)+ry0,menu[slct].name);
   } else {
      solid_string((rc.mb_x+10)+rx0,(rc.mb_y-13)+(offset*rc.font_h)+ry0,menu[slct].name);
      rectfill(screen,(rc.mb_x+2)+rx0,(rc.mb_y-13)+(offset*rc.font_h)+ry0,(rc.mb_x+9)+rx0,rc.mb_y+2+(offset*rc.font_h)+ry0, fnbgcol);
   }
   if(menu[slct].type=='f' || menu[slct].type=='d') {
     rectfill(screen,(rc.mb_x+3)+rx0,(rc.mb_y-8)+(offset*rc.font_h)+ry0,(rc.mb_x+8)+rx0,(rc.mb_y-4)+(offset*rc.font_h)+ry0, fnfgcol);
   }
   if(menu[slct].type=='s') {
/*      rect(screen,37,88+(offset*16),44,95+(offset*16), fnbgcol); */
     rect(screen,(rc.mb_x+4)+rx0,(rc.mb_y-9)+(offset*rc.font_h)+ry0,(rc.mb_x+11)+rx0,(rc.mb_y-2)+(offset*rc.font_h)+ry0, fnfgcol);
   }
#ifdef USESDL
//   gfx_sdlflip();
   s2a_flip(screen);
#endif
  unscare_mouse();
} // menu_uhlight

int menu_hlight(int index, int slct) {

   /* Hilight the menu selection */
   
   int offset;
   
   offset=(slct-index)+1;
   set_font_fcolor(textsdr,textsdg,textsdb);
/*   set_font_bcolor(0,0,0); */
   set_font_bcolor(texthlr,texthlg,texthlb);
   scare_mouse();
//   solid_string((rc.mb_x+10)+rx0,(rc.mb_y-13)+(offset*rc.font_h)+ry0,menu[slct].name);
//   fnt_print_string(screen,(rc.mb_x+10)+rx0,(rc.mb_y-13)+(offset*rc.font_h)+ry0,menu[slct].name,makecol(textsdr,textsdg,textsdb),makecol(texthlr,texthlg,texthlb),-1);
//   
   
   rectfill(screen,(rc.mb_x+2)+rx0,(rc.mb_y-13)+(offset*rc.font_h)+ry0,
	   (rc.mb_x+2)+rc.mb_w-4+rx0,(rc.mb_y)+2+(offset*rc.font_h)+ry0,fnbgcol);
// fnt_print_string(screen,(rc.mb_x+10)+rx0,(rc.mb_y-13)+(offset*rc.font_h)+ry0,menu[slct].name,makecol(textsdr,textsdg,textsdb),fnbgcol,-1);
   fnt_print_string(screen,(rc.mb_x+10)+rx0,(rc.mb_y-13)+(offset*rc.font_h)+ry0,menu[slct].name,makecol(textsdr,textsdg,textsdb),-1,-1);
   rectfill(screen,(rc.mb_x+2)+rx0,(rc.mb_y-13)+(offset*rc.font_h)+ry0,(rc.mb_x+9)+rx0,(rc.mb_y+2)+(offset*rc.font_h)+ry0, fnbgcol);
//   rect(screen,(rc.mb_x)+rx0,(rc.mb_y-13)+(offset*rc.font_h)+ry0,(rc.mb_x+60)+rx0,(rc.mb_y+2)+(offset*rc.font_h)+ry0, fnbgcol);
//printf("VONG: pos %d,%d,%d,%d\n",rc.mb_x+2,rc.mb_y-13+(offset*rc.font_h)+ry0,(rc.mb_x+9)+rx0,(rc.mb_y+2)+(offset*rc.font_h)+ry0);
   
   
   if(menu[slct].type=='f' || menu[slct].type=='d') {
      rectfill(screen,(rc.mb_x+3)+rx0,(rc.mb_y-8)+(offset*rc.font_h)+ry0,(rc.mb_x+8)+rx0,(rc.mb_y-4)+(offset*rc.font_h)+ry0, fnfgcol);
//      printf("dmr::x=%d;w=%d\n",(rc.mb_x+3)+rx0,(rc.mb_x+8)+rx0);
//      printf("dmr::y=%d;h=%d\n",(rc.mb_y-13)+(offset*rc.font_h)+ry0,(rc.mb_y+2)+(offset*rc.font_h)+ry0);
   }
   if(menu[slct].type=='s') {
/*       rect(screen,37,88+(offset*16),44,95+(offset*16), fnbgcol); */
     rect(screen,(rc.mb_x+4)+rx0,(rc.mb_y-9)+(offset*rc.font_h)+ry0,(rc.mb_x+11)+rx0,(rc.mb_y-2)+(offset*rc.font_h)+ry0, fnfgcol);
     rectfill(screen,(rc.mb_x+6)+rx0,(rc.mb_y-7)+(offset*rc.font_h)+ry0,(rc.mb_x+9)+rx0,(rc.mb_y-4)+(offset*rc.font_h)+ry0, fnfgcol);
   }
#ifdef USESDL
   //gfx_sdlflip();
   s2a_flip(screen);
#endif
   unscare_mouse();
}

int menu_uhlight2(int index, int slct) {

   /* New, experimental version of menu_unlight */
   
   /* Unhilight the menu selection */
   
   int offset;
   
   offset=(slct-index)+1;
/*   set_font_fcolor(0,0,0); */
   st_txt_col(menu[slct].type);
   set_font_bcolor(textbgr,textbgg,textbgb);
   scare_mouse();
   if(usembmap==1) {
      blit(menumap, screen,0,(offset*rc.font_h)-13,rc.mb_x+rx0,ry0+rc.mb_y-13+(offset*rc.font_h),rc.mb_w,rc.font_h);
      show_string((rc.mb_x+10)+rx0,(rc.mb_y-13)+(offset*rc.font_h)+ry0,menu[slct].name);
   } else {
      solid_string((rc.mb_x+10)+rx0,(rc.mb_y-13)+(offset*rc.font_h)+ry0,menu[slct].name);
      rectfill(screen,(rc.mb_x+2)+rx0,(rc.mb_y-13)+(offset*rc.font_h)+ry0,(rc.mb_x+9)+rx0,rc.mb_y+2+(offset*rc.font_h)+ry0, fnbgcol);
   }
   if(menu[slct].type=='f' || menu[slct].type=='d') {
     rectfill(screen,(rc.mb_x+3)+rx0,(rc.mb_y-8)+(offset*rc.font_h)+ry0,(rc.mb_x+8)+rx0,(rc.mb_y-4)+(offset*rc.font_h)+ry0, fnfgcol);
   }
   if(menu[slct].type=='s') {
/*      rect(screen,37,88+(offset*16),44,95+(offset*16), fnbgcol); */
     rect(screen,(rc.mb_x+4)+rx0,(rc.mb_y-9)+(offset*rc.font_h)+ry0,(rc.mb_x+11)+rx0,(rc.mb_y-2)+(offset*rc.font_h)+ry0, fnfgcol);
   }
#ifdef USESDL
//   gfx_sdlflip();
   s2a_flip(screen);
#endif
  unscare_mouse();
} // menu_uhlight

int menu_hlight2(int index, int slct) {

   /* New, experimental version of menu_hlight */
   /* Hilight the menu selection */
   
   int offset;

   offset=(slct-index)+1;
   scare_mouse();

//   SDL_SetAlpha(selection, SDL_SRCALPHA, 128);
   blit(selection,screen,0,0,rc.mb_x+rx0,(rc.mb_y-13)+(offset*rc.font_h)+ry0);
//   SDL_SetAlpha(selection, SDL_SRCALPHA, 255);

//   fnt_print_string(screen,(rc.mb_x+10)+rx0,(rc.mb_y-13)+(offset*rc.font_h)+ry0,menu[slct].name,makecol(textsdr,textsdg,textsdb),makecol(texthlr,texthlg,texthlb),-1);
//   rectfill(screen,(rc.mb_x+2)+rx0,(rc.mb_y-13)+(offset*rc.font_h)+ry0,(rc.mb_x+9)+rx0,(rc.mb_y+2)+(offset*rc.font_h)+ry0, fnbgcol);
//   
//   if(menu[slct].type=='f' || menu[slct].type=='d') {
//     rectfill(screen,(rc.mb_x+3)+rx0,(rc.mb_y-8)+(offset*rc.font_h)+ry0,(rc.mb_x+8)+rx0,(rc.mb_y-4)+(offset*rc.font_h)+ry0, fnfgcol);
//   }
//   if(menu[slct].type=='s') {
//     rect(screen,(rc.mb_x+4)+rx0,(rc.mb_y-9)+(offset*rc.font_h)+ry0,(rc.mb_x+11)+rx0,(rc.mb_y-2)+(offset*rc.font_h)+ry0, fnfgcol);
//     rectfill(screen,(rc.mb_x+6)+rx0,(rc.mb_y-7)+(offset*rc.font_h)+ry0,(rc.mb_x+9)+rx0,(rc.mb_y-4)+(offset*rc.font_h)+ry0, fnfgcol);
//   }
#ifdef USESDL
//   gfx_sdlflip();
   s2a_flip(screen);
#endif
   unscare_mouse();
}

int find_menu_e(char *rom) {
   int i,r;
   r=1;
   for(i=1; i<menulength+1; i++) 
     {
	if(strncmp(rom, menu[i].rom, strlen(rom)) == 0) {
	   r=i;
	}
     }   
   return r;
}
   
int display_menu(int index) {
/*   int red=makecol16(240,16,0); */
   int i,io;
   
//   fnt_setactive(myttf);
   scare_mouse();
   for(i=index; i<index+(rc.mb_h/rc.font_h);i++) {
      io=i-index+1;
      if (i>menulength) break;
/*     textout(screen,font,names[i],34,90+(i*8),red); */
      st_txt_col(menu[i].type);
      if(menu[i].type=='m' || menu[i].type=='d') {
/*	 set_font_fcolor(80,0,80); */
//	 printf("fontprt: passing -1 to fnt_print_screen\n");
	 fnt_print_string(screen,(rc.mb_x+10)+rx0,(rc.mb_y-13)+(io*rc.font_h)+ry0,menu[i].name,fnfgcol,-1,shdcol);
//	show_string((rc.mb_x+10)+rx0,(rc.mb_y-13)+(io*rc.font_h)+ry0,menu[i].name);
	rectfill(screen,(rc.mb_x+3)+rx0,(rc.mb_y-8)+(io*rc.font_h)+ry0,(rc.mb_x+8)+rx0,(rc.mb_y-4)+(io*rc.font_h)+ry0, fnfgcol);
      } else {
	 if(menu[i].type=='s') {
/*	    rect(screen,37,88+(io*16),44,95+(io*16), fnbgcol); */
	   rect(screen,(rc.mb_x+4)+rx0,(rc.mb_y-9)+(io*16)+ry0,(rc.mb_x+11)+rx0,(rc.mb_y-2)+(io*rc.font_h)+ry0, fnfgcol); 
	 }	 
/*	 set_font_fcolor(80,64,16); */
	 fnt_print_string(screen,(rc.mb_x+10)+rx0,(rc.mb_y-13)+(io*rc.font_h)+ry0,menu[i].name,fnfgcol,-1,shdcol);
      }
/*     printf("here:%d,%s\n",i,names[i]); */
   }   
#ifdef USESDL
//   gfx_sdlflip();
   s2a_flip(screen);
#endif
   unscare_mouse();
//   fnt_setactive(LoadedFont);
}

void info_bar() {
/*   set_font_fcolor(232,232,0); */
   set_font_fcolor(descbgr,descbgg,descbgb);
//   set_font_bcolor(64,64,64);
   set_font_bcolor(texthlr,texthlg,texthlb);
   rectfill(screen, rc.db_x+2+rx0, rc.db_y+rc.db_h-17+ry0, rc.db_x+rc.db_w-2+rx0, rc.db_y+rc.db_h-2+ry0, fnbgcol);
   solid_string(rc.db_x+40+rx0,rc.db_y+rc.db_h-17+ry0,"Press mouse/joystick button or '<ENTER>' to select this option");  
/*   set_font_fcolor(232,64,0); */
//   set_font_fcolor(textsdr,textsdg,textsdb);
   set_font_fcolor(255,255,255);
   solid_string(rc.db_x+88+rx0,rc.db_y+rc.db_h-17+ry0,"mouse/joystick button");  
   solid_string(rc.db_x+296+rx0,rc.db_y+rc.db_h-17+ry0,"<ENTER>");  
}

void rem_info_bar() 
{
   int white;
   white=makecol16(descbgr,descbgg,descbgb); 
/*   rectfill(screen, 34, 431, 606, 446, white); */
   if(usedbmap==0)
     rectfill(screen, rc.db_x+2+rx0, rc.db_y+rc.db_h-17+ry0, rc.db_x+rc.db_w-2+rx0, rc.db_y+rc.db_h-2+ry0, white);
   else
     blit(descmap,screen, 0,rc.db_h-20,rc.db_x+rx0,rc.db_y+rc.db_h-20+ry0,rc.db_w,20);
}

void write_option(int selected) 
{
   // used by old-style setup, depricated
   FILE *fp;
   char optfile[60];
   
#ifdef LINUX
   strcpy(optfile,"/tmp/emufe.options");
#endif
#ifdef CYGWIN
   strcpy(optfile,"c:/emulator/tmp/emufe.options");
#endif


   if ((fp=fopen(optfile, "a+")) != NULL) {
      fprintf(fp,"%s\n", commands[selected]);
      fclose(fp);
   }
}

void desc_wrapa(char* rstr, char *istr, int len) {

// Word wrap for show_desc2,  this breaks and returns the left side of 
// a break

  int lastspc,i;

  for(i=0;i<len;i++) {
    if(istr[i]==' ') lastspc=i;
    if(istr[i]=='\n') {
      lastspc=i;
      break;
    }
  }
 
  for(i=0;i<lastspc;i++)
   rstr[i]=istr[i];
  rstr[i]=0;
}

void desc_wrapb(char* rstr, char *istr, int len) {

// Word wrap for show_desc2,  this breaks and returns the right side of 
// a break

  int lastspc,i;

  for(i=0;i<len;i++) {
    if(istr[i]==' ') lastspc=i;
    if(istr[i]=='\n') {
      lastspc=i;
      break;
    }
  }
 
  for(i=lastspc+1;i<strlen(istr);i++)
   rstr[i-(lastspc+1)]=istr[i];
  rstr[i-(lastspc+1)]=0;
}

void show_desc2(char *desc) {
   FILE *fp;
   char line[300], nxline[300], descffname[90], *ww;
   char title[128], year[12], company[70];
   int lineno, white, black, box_cw, box_ch,i;
   
   box_cw=(rc.db_w-8)/8;
   box_ch=(rc.db_h-8)/16;
   strcpy(title,desc);
   strcpy(year,"unknown");
   strcpy(company,"unknown");
   strcpy(nxline,"\n");

#ifdef DEBUG
   sprintf(debugtxt,"Loading Description (2)...%s\n",desc);
   debug(3,debugtxt);
#endif
   if(imenu.mode==0) {
     sprintf(line, "%s/%s.desc",descdir,desc);
      dfixsep2(descffname,line,1);
   }
   if(imenu.mode==1) {
      sprintf(line, "%s/%s.desc",descdir,desc);
      dfixsep2(descffname,line,1);
   }
   if(imenu.mode==2) {
      sprintf(line, "%s%s/%s.desc",imenu.sysbase,descdir,desc);
      dfixsep2(descffname,line,1);
   }

   if(usedbmap==0)
     bbox(rc.db_x+rx0, rc.db_y+ry0, rc.db_x2+rx0, rc.db_y2+ry0, white, black);
   else
     blit(descmap,screen, 0,0,rc.db_x+rx0,rc.db_y+ry0,rc.db_w,rc.db_h);
#ifdef DEBUG
   sprintf(debugtxt,"descffname=%s\n",descffname);
   debug(3,debugtxt);
#endif

   if ((fp=fopen(descffname, "r")) != NULL) {
      lineno=0;
      while( !feof(fp) ) {
	 fgets(line, box_cw-strlen(nxline), fp);
	 if(feof(fp))
	   break;
         if( strncmp(line,"Name|",5)==0 || strncmp(line,"Year|",5)==0 || strncmp(line,"Company|",8)==0 ) {
           if(strncmp(line,"Name|",5)==0) hss_index(title,line,1,'|'); 
           if(strncmp(line,"Year|",5)==0) hss_index(year,line,1,'|'); 
           if(strncmp(line,"Company|",8)==0) hss_index(company,line,1,'|'); 
         } else {
   	   lineno++;
	   if(lineno > box_ch)
	     break;
           if(lineno==1) 
	     fnt_print_string(screen,rc.db_x+4+((rc.db_w-(strlen(title)*8))/2)+rx0,(rc.db_y-14)+(lineno*16)+ry0,title,makecol(255,255,255),-1,-1);
	   else {
// word wrap
             strcat(nxline,line);
             desc_wrapa(line,nxline,box_cw);
//      	     fnt_print_string(screen,rc.db_x+4+rx0,(rc.db_y-14)+(lineno*16)+ry0,line,makecol(textfgr,textfgg,textfgb),-1,-1);
//           shadow
      	     fnt_print_string(screen,rc.db_x+5+rx0,(rc.db_y-14)+(lineno*16)+1+ry0,line,makecol(22,22,22),-1,-1);
      	     fnt_print_string(screen,rc.db_x+4+rx0,(rc.db_y-14)+(lineno*16)+ry0,line,makecol(texthlr,texthlg,texthlb),-1,-1);
             desc_wrapb(line,nxline,box_cw);
	     strcpy(nxline,line);

           }
        }
      }
      fclose(fp);
#ifdef USESDL
   //   gfx_sdlflip();
      s2a_flip(screen);
#endif
   }
}

void show_desc(char *desc) {
   FILE *fp;
   char line[72], descffname[90], *ww;
   int lineno, white, black;
#ifdef DEBUG
   sprintf(debugtxt,"Loading Description...%s\n",desc);
   debug(1,debugtxt);
#endif
   if(imenu.mode==0)
     sprintf(descffname, "%s/%s.desc",descdir,desc);
   if(imenu.mode==1) {
      sprintf(line, "%s/%s.desc",descdir,desc);
      dfixsep2(descffname,line,1);
   }
   if(imenu.mode==2) {
      sprintf(line, "%s%s/%s.desc",imenu.sysbase,descdir,desc);
      dfixsep2(descffname,line,1);
   }
   white=makecol16(descbgr,descbgg,descbgb); 
/* Probably should be set by a theme item instead */
   black=makecol16(descbgr,descbgg,descbgb); 

   if(usedbmap==0)
     bbox(rc.db_x+rx0, rc.db_y+ry0, rc.db_x2+rx0, rc.db_y2+ry0, white, black);
   else
     blit(descmap,screen, 0,0,rc.db_x+rx0,rc.db_y+ry0,rc.db_w,rc.db_h);
#ifdef DEBUG
   sprintf(debugtxt,"descffname=%s\n",descffname);
   debug(3,debugtxt);
#endif
//   fnt_setactive(myttf);
   if ((fp=fopen(descffname, "r")) != NULL) {
      lineno=0;
/*      set_font_fcolor(textier,textieg,textieb);*/
//      set_font_fcolor(textfgr,textfgg,textfgb);
      while( !feof(fp) ) {
	 fgets(line, 72, fp);
	 if(feof(fp))
	   break;
	 lineno++;
	 if(lineno > 10)
	   break;
//	 printf("desc: %s\n",line);
//	 show_string(rc.db_x+4+rx0,(rc.db_y-14)+(lineno*16)+ry0,line);
//	 fnt_print_string(screen,rc.db_x+4+rx0,(rc.db_y-14)+(lineno*16)+ry0,line,makecol(textfgr,textfgg,textfgb),-1,-1);
	 fnt_print_string(screen,rc.db_x+4+rx0,(rc.db_y-14)+(lineno*16)+ry0,line,makecol(rc.txdesc_r,rc.txdesc_g,rc.txdesc_b),-1,-1);
      }
      fclose(fp);
//      fnt_setactive(LoadedFont);
#ifdef USESDL
//      gfx_sdlflip();
   s2a_flip(screen);
#endif
   }
}

int AddPicExt(char *outpath, char *inpath) {
   /* In order to support other image types besides pcx, we should
    * allow arbitrary types to be used,  this function will check
    * to see which extenstion exists and returns the full path name
    * with the correct extension */
   int i,c,rv=1;
   // disabled to prevent too much checking (not that it was a problem)
   //   char* extlist = "pcx,png,jpg,bmp,gif,PCX,PNG,JPG,BMP,GIF";
   char* extlist = "pcx,png,jpg,gif";
   char testpath[355],ext[8];
   
   c = hss_count(extlist, ',');
   for(i=0;i<c;i++) {
      hss_index(ext, extlist, i,',');
      sprintf(testpath, "%s.%s", inpath, ext);
      LOG(1, ("JIO: testing path: %s\n",testpath));
      if (fileio_file_exists(testpath)) {
	 strcpy(outpath,testpath);
	 rv=0;
	 LOG(1, ("JIO: selected: %s\n",testpath));
	 break;
      }
      if(rv!=0) strcpy(outpath,"null");
   }
   return(rv);
}


void display_info(int slc)  {
   char picname[300];
   // is this still used here???
   sprintf(picname, "%s%c%s.pcx", picsdir, mysep, menu[slc].rom);

#ifdef DEBUG
   sprintf(debugtxt,"\n>>picname is: %s\n", picname);
   debug(2,debugtxt);
#endif
//   disp_image(picname);
   do_imgboxes(picsdir,menu[slc].rom);
   show_desc2(menu[slc].rom);
//   info_bar();
}


int load_menu(char *lmenu) {
   FILE *fp;
   char type=',',w,picname[180],*ww;
/*   char name[53], rom[25];*/
//   char *name, *rom, waste[15];
   char *name, rom[30], waste[39];
   int i=1,sl;
#ifdef DEBUG
   sprintf(debugtxt,"Loading Menu...%s\n",lmenu);
   debug(1,debugtxt);
   sprintf(debugtxt,"Menu strlen: %d\n",strlen(menu));
   debug(5,debugtxt);
#endif
   fp=fopen(lmenu, "r");
   while( type != 'e' ) {
      if(feof(fp)) break;
      type=fgetc(fp);
/*#ifdef DEBUG
      if( type == 'e') break;
#endif*/
      w=fgetc(fp);
      /* Freeze seems to occur here, if 'e' entry in menu doesn't have romname */
      name=(char *)malloc(39);

      fgets(name, 39, fp);
      fgets(waste, 15, fp);
      fgets(rom, 25, fp);
      sl=strlen(rom);
      rom[sl-1]='\0';
/*      printf("2:rom: %s, %d\n",rom, sl); */
      if( type != ' ' ) {
	 switch (type) {
	  case 't':
	    if(hss_count(name,'"')==1)
	      strcpy(imenu.title,name);
	    else
	      hss_index(imenu.title,name,1,'"');
//	    title(320,40,imenu.title);
	    title(-1,-1,imenu.title);
//	    sprintf(picname, "%s/%s.pcx",picsdir ,rom); 
#ifdef DEBUG
	    sprintf(debugtxt,"\npicsdir is: %s  rom is %s\n", picsdir,rom);
	    debug(2,debugtxt);
#endif
            do_imgbox(B_PICBOX,picsdir,rom);
            do_imgbox(B_KEYBOARD,picsdir,rom);
            do_imgbox_scale(B_BOXSCAN,picsdir,rom);
// Note: We need a replacement for this! 
//	    disp_image(picname);
	    break;
	  case 'f':
	    if(strncmp(currfont,rom,strlen(currfont))!=0)
	      sprintf(fullpath,"%s%c%s",fontdir,mysep,rom);
#ifdef DEBUG
	    sprintf(debugtxt,"menu bitmap.font.load\n");
	    debug(3,debugtxt);
#endif
	    // Kludge, don't load fonts from menu if using TTF
	    if(ActiveFont->type != TTF)
	      font_load(fullpath);
	    break;
	  case 'S':
//	    (char *)commands[i]=name;
//	    (char *)lmenus[i]=rom;
	    strcpy(commands[i],name);
	    strcpy(lmenus[i],rom);
	    break;
	  case 'F':
	    /* File selector */
	    menu[i].type=type;
	    name=strtok(name," ");
	    if(strncmp(name, "FORCE", 5) == 0) {
	       strcat(fname, "/");
	       file_select_ex("Select a Directory", fname, NULL, 80, 520, 350);
	    } else {
	       if (getenv(name)) {
		  strcpy(fname,getenv(name));
	       } else {
		  strcpy(fname,name);
	       }
	    }
	    snprintf(name, 39, "%s                               ", fname);
	    sprintf(commands[i],"%s=%s",rom, fname);
	    write_option(i);
	    /*	      (char *)commands[i]=fname; */
	    commands[i]=(char *)malloc(480);
	    strcpy(commands[i],fname);
	    strcpy(menu[i].name,name);
	    strcpy(menu[i].rom,rom);
	    //	      (char *)names[i]=name;
	    //	      (char *)roms[i]=rom;
	    i++;
	    break;
	  default:	       
	    menu[i].type=type;
	    strcpy(menu[i].name,name);
	    strcpy(menu[i].rom,rom);
	    //	      (char *)names[i]=name;
	    //	      (char *)roms[i]=rom;
	    /*         printf("%d->:%d\n",i,names[i]);  */
#ifdef DEBUG
	    printf(".");
#endif
	    i++;
	 }
	 
#ifdef DEBUG
      } else {
	 sprintf(debugtxt,"ignored: %s\n",name);
	 debug(3,debugtxt);
	 /* put a free here */
#endif
      }      
   }
   
   menulength=i-2;
   fclose(fp);

   LOG(3,("done\n"));
   LOG(3,(debugtxt,"menulength: %d\n",menulength));

   return i-2;
}

int draw_screen() {
   int white,black,gray128,menubg,i;

   black=makecol16(0,0,0); 
   gray128=makecol16(128,128,128); 
   menubg=makecol16(textbgr,textbgg,textbgb); 
   white=makecol16(255,255,255); 

   set_bg();
   draw_menubox(menubg,black);
//   draw_picbox(gray128,black);
   draw_title(black,white);
   draw_desc(white,black);
   for(i=0;i<12;i++)
     draw_imgbx(i);
}

void restore_screen(int index, int slc) {
   // After changing gfx mode, call this
   set_font_fcolor(textfgr,textfgb,textfgg);
   set_font_scolor(shadowr,shadowg,shadowb);

   draw_screen();
   
   restore_menuback();
   display_menu(index);
   menu_hlight(index,slc);
}

void settxtmode() {
   int w;
   if(in_gfxmode==1) {
#ifdef WIN32XXX
      // this MIGHT be useful for testing
      w=set_gfx_mode(GFX_AUTODETECT_WINDOWED,usex,usey,0,0);
# ifdef DEBUG
      debug(3,"settxtmode() attempting window mode rather than text mode to address a bug\n");
# endif
#else
   // if using GFX_TEXT causes trouble, use GFX_AUTODETECT_WINDOWED to
   // put this app into a window
      
      w=set_gfx_mode(GFX_AUTODETECT_WINDOWED,usex,usey,0,0);
//      w=set_gfx_mode(GFX_TEXT,usex,usey,0,0);
#endif
   }
   in_gfxmode=0;
}

void setgfxmode() {
   int w=0;
   
#ifdef DEBUG
   sprintf(debugtxt,"setgfxmode() fullscr=%c  usex=%d  usey=%d, in_gfxmode:%d\n",fullscr,usex,usey,in_gfxmode);
   debug(3,debugtxt);
#endif
   if(in_gfxmode==0) {
      if(fullscr=='n') 
	w=set_gfx_mode(GFX_AUTODETECT_WINDOWED,usex,usey,0,0);
      if(fullscr=='y')
	w=set_gfx_mode(GFX_AUTODETECT_FULLSCREEN,usex,usey,0,0);
      if(fullscr!='n' && fullscr!='y')
	w=set_gfx_mode(GFX_AUTODETECT,usex,usey,0,0);
#ifdef USESDL
      set_keyboard_rate(400,40);
#endif
   }
   if(w!=0) {
#ifdef DEBUG
      debug(1,"Could not set graphics mode!\n");
#endif
      exit(34);
   }
   in_gfxmode=1;
}

init() {
   char tmpstr[20], tstr2[20];

   allegro_init();

   install_mouse();
   install_keyboard();
   install_joystick(JOY_TYPE_AUTODETECT);
#ifdef SDL1
   // test to see if SDL1 can take 32
   set_color_depth(16);
#else
   set_color_depth(32);
#endif
   set_keyboard_rate(400,40);
   
   load_defaults();
   // probably need to adjust this
   sprintf(fullpath,"%s%c%s",basedir,mysep,rcfilename);
//   printf("xdescbox: %s\n",descbox);
   if(imenu.mode>=1) {env_clear(); load_settings(); 
//      env_get(tmpstr,"EMUFEwide");
//      if ( tmpstr[0] == 'y' || tmpstr[0] == 'Y') {
//	 widescreen=1;
//	 if(usex==640) usex=800;
//	 minx=640;
//	 // usex=minx=800;
//	 rx0=(usex-minx)/2;
//      }
      strcpy(tmpstr,"");
      env_get(tmpstr,"EMUFEres");
      if(strcmp(tmpstr,"")!=0) {
//	 printf("SHICK::%s\n",tmpstr);
	 hss_index(tstr2,tmpstr,0,'x');
	 usex=atoi(tstr2);
	 hss_index(tstr2,tmpstr,1,'x');
	 usey=atoi(tstr2);
	 if(usex<minx || usey<miny) {
	    printf("Error: Minimum resolution is %dx%d\n",minx,miny);
	    exit(0);
	 }
	 rx0=(usex-minx)/2;
	 ry0=(usey-miny)/2;
      }
   }
   if(imenu.mode>1) 
     load_rc(imenu.rc); 
   else
     load_rc(fullpath);
//   printf("ydescbox: %s\n",descbox);
   load_dfltimg(defimg);  /* Load Default Image */

   setgfxmode();
   set_font_bcolor(0,0,0);
   set_font_fcolor(textfgr,textfgb,textfgg);
//   printf("Setting font shadow to %d,%d,%d\n",shadowr,shadowg,shadowb);
   set_font_scolor(shadowr,shadowg,shadowb);

   // init font here
   fnt_init();
#ifdef BLITFONT
   LOG(3, ("BLITFONT status: enabled\n"));
   LOG(3, ("blitfont.font.load\n"));
   if(strncmp(tfontbmp, "na", 2) == 0)
     font_load(tfont);
   else
     bmp_font_load(tfontbmp);
#else
   LOG(3, ("BLITFONT status: disabled\n"));
# ifdef USE_FREETYPE
   if(strncmp(rc.ttfont, "na", 2) != 0) {
     fnt_destroy(myttf);
     sprintf(fullpath,"%s%c%s",fontdir,mysep,rc.ttfont);
      LOG(3,("ttf.font.load\n"));
      myttf=fnt_loadfont(fullpath,TTF);
      LOG(3, ("ttf.font.load done\n"));
      
      fnt_setactive(myttf);
      myttf->scale_w=12; myttf->scale_h=16;
   } else {
     fnt_destroy(LoadedFont);
     sprintf(fullpath,"%s%c%s",fontdir,mysep,tfont);
      LOG(3, ("bitmap.font.load\n"));
     LoadedFont=fnt_loadfont(fullpath,BIOS_8X16);
     fnt_setactive(LoadedFont);
   }
# else
   sprintf(fullpath,"%s%c%s",fontdir,mysep,tfont);
#  ifdef DEBUG
   sprintf(debugtxt,"Font: %s\n",fullpath);
   debug(3,debugtxt);
   sprintf(debugtxt,"zdescbox: %s\n",descbox);
   debug(3,debugtxt);
#  endif
   // so font_load is messing up descbox!!!
   LOG(3, ("bitmap.font.load (no freetype)\n"));
   font_load(fullpath);
# endif
#endif

}


int title(int x1,int y1,char *s) {
   // draw title
   int width, length, fpx, fpy, shcol=-1, bgcol=-1;
   
   blit(titlemap,screen,0,0,rc.bb_x+rx0,rc.bb_y+ry0,rc.bb_w,rc.bb_h);

   if(rc.banr.bg.enable=='Y')
     bgcol=makecol(rc.banr.bg.r,rc.banr.bg.g,rc.banr.bg.b);
  if(rc.banr.sh.enable=='Y')
     shcol=makecol(rc.banr.sh.r,rc.banr.sh.g,rc.banr.sh.b);
   if(strcmp(rc.fontsize,"16x32")==0) {
      // Use arcade styled large font instead of default font
      length=strlen(s)*8;
      if(x1=-1) {
	 fpx=rx0+rc.bb_x+(rc.bb_w-(length*2))/2;
         fpy=ry0+rc.bb_y+(rc.bb_h-32)/2;
      } else {
         fpx=rx0+rc.bb_x+x1-(length*2);
	 fpy=ry0+rc.bb_y+y1-24;
      }      
      print_string_16x32(screen,fpx,fpy,s,
			 makecol(rc.banr.fg.r,rc.banr.fg.g,rc.banr.fg.b),bgcol,
			 shcol);
   } else {
      length=strlen(s)*8;
      if(x1=-1) {
	 fpx=rx0+rc.bb_x+(rc.bb_w-length)/2;
         fpy=ry0+rc.bb_y+(rc.bb_h-16)/2;
      } else {
         fpx=rx0+rc.bb_x+x1-length;
	 fpy=ry0+rc.bb_y+y1-24;
      }
      fnt_print_string(screen,fpx,fpy,s,
	 makecol(rc.banr.fg.r,rc.banr.fg.g,rc.banr.fg.b),-1,
		       shcol);      
   }
}

bbox(int x1, int y1, int x2, int y2, int color, int bcolor) {
   rectfill(screen, x1, y1, x2, y2, color);
   rect(screen, x1, y1, x2, y2, bcolor);
   rect(screen, x1+1, y1+1, x2-1, y2-1, bcolor);
} // bbox()

load_dfltimg(char *fname) {
   PALETTE p; 
   char picname[180], *ww;
   get_palette(p);

   sprintf(picname,"%s%c%s",picsdir,mysep,fname);
#ifdef DEBUG
   sprintf(debugtxt,"load_dfltimg: loading %s\n",picname);
   debug(1,debugtxt);
#endif 
   defbmp=load_bitmap(picname,p);
} // load_dfltimg()

do_imgbox_scale(int i, char *imgdir, char *iname) {
  // Use this if scaling is enabled
  PALETTE p;
  char picname[350],picnoext[346];
  int dsx,dsy,new_w,new_h,x2,y2;
  float fow,foh,fdw,fdh,xf,yf,uf;
  BITMAP *sc_bitmap;
   
  get_palette(p);
//  for(i=0;i<12;i++) {
    if(imgbx[i].enabled==1) {
//      sprintf(picname,"%s%c%s%s.pcx",imgdir,mysep,imgbx[i].pfx,iname);
       sprintf(picnoext,"%s%c%s%s",imgdir,mysep,imgbx[i].pfx,iname);
       AddPicExt(picname,picnoext);
#ifdef DEBUG
       sprintf(debugtxt,"do_imgbox_scale(): looking for picname:%s\n",picname);
       debug(3,debugtxt);
#endif
       bitmap=load_bitmap(picname,p);
       if(imgbx_bmp[i] && i!=B_KEYBOARD) 
	 masked_blit(imgbx_bmp[i], screen,0,0,imgbx[i].x+rx0,imgbx[i].y+ry0,imgbx[i].w,imgbx[i].h);
       else 
	 printf("do_imgbox_scale(): I HAVE NO BG!\n");
		 

       if(bitmap) {
	  
	  if(bitmap->w != imgbx[i].w || bitmap->h != imgbx[i].h) {
	     //        if(bitmap->w == 4 || bitmap->h == 5) {
	     // downscale
	     // compute scale factor, need to use floats here
	     // This will keep aspect and works for both down and upscaling
	     fow=bitmap->w;foh=bitmap->h;fdw=imgbx[i].w;fdh=imgbx[i].h;
	     xf=fow/fdw;
	     yf=foh/fdh;
	     if(xf>yf) uf=xf; else uf=yf;
	     new_w=fow/uf;
	     new_h=foh/uf; 
	     // mask the dest with black bars
	     //   or bitmap!
	     x2=imgbx[i].x + imgbx[i].w;
	     y2=imgbx[i].y + imgbx[i].h;
	     
#ifdef DEBUG
	     sprintf(debugtxt,"rectfill x:%d y:%d w:%d h:%d\n",imgbx[i].x,imgbx[i].y,x2,y2);
	     debug(3,debugtxt);
#endif
	     // Figure out how to mask this (if masktype==0 then ignore)
	     // bitmap
	     // black bars
	     if(imgbx[i].masktype==2)
	       rectfill(screen,imgbx[i].x,imgbx[i].y,x2,y2,makecol16(0,0,0));
	     // blit (need to fix dest w/h)
	     // uncomment to center:
	     dsx=((imgbx[i].w-new_w)/2)+imgbx[i].x+rx0 + imgbx[i].mgn;
	     dsy=((imgbx[i].h-new_h)/2)+imgbx[i].y+ry0 + imgbx[i].mgn;
	     //	   dsx=((imgbx[i].w-sc_bitmap->w)/2)+imgbx[i].x+rx0;
	     //	   dsy=((imgbx[i].h-sc_bitmap->h)/2)+imgbx[i].y+ry0;
	     //	   dsx=imgbx[i].x+rx0;
	     //	   dsy=imgbx[i].y+ry0;

	     // implement bottom/right  margins
	     new_w=new_w - (imgbx[i].mgn*2);
	     new_h=new_h - (imgbx[i].mgn*2);

	     stretch_blit(bitmap,screen,0,0,bitmap->w,bitmap->h,dsx,dsy,new_w,new_h);
	     //	   sc_bitmap=sa_scale_bm(bitmap,new_w,new_h);
	     //	   blit(sc_bitmap,screen,0,0,dsx,dsy,sc_bitmap->w,sc_bitmap->h);
	     //	   destroy_bitmap(sc_bitmap);
	     
	     destroy_bitmap(bitmap);
	     // draw overlay
	     if(imgbx[i].ovpct>0) {
		
		
		sa_setalpha(imgbx_ovl[i], (25500/(10000/imgbx[i].ovpct)));
		
		//	      printf("XFMT: alpha: %d\n",(25500/(10000/imgbx[i].ovpct)));
		// debug info follows for surfaces, uncomment to help debug
		// sa_surface_info(screen, "screen");
		//	      sa_surface_info(imgbx_ovl[i], "imgbx_ovl");
		//	      sa_debug_info();
		
		//	      SDL_SetAlpha(imgbx_ovl[i], SDL_SRCALPHA, (25500/(10000/imgbx[i].ovpct)));
		//	      SDL_SetAlpha(imgbx_ovl[i], SDL_SRCALPHA, 128);
		masked_blit(imgbx_ovl[i], screen,0,0,imgbx[i].x+rx0,imgbx[i].y+ry0,imgbx[i].w,imgbx[i].h);	      
		sa_setalpha(imgbx_ovl[i], 255);
		//	      printf("HEBLO?\n");
		//	      printf("screen XFMT: %d\n",screen->format->format);
		//	      printf("imgbx_bmp[] XFMT: %d\n",imgbx_ovl[i]->format->format);
		
		//	      SDL_SetAlpha(imgbx_ovl[i], SDL_SRCALPHA, 255);
	     }
	  } else {
	   // no scaling
	     dsx=imgbx[i].x+rx0;
	     dsy=imgbx[i].y+ry0;
	     blit(bitmap,screen,0,0,dsx,dsy,bitmap->w,bitmap->h);
	     destroy_bitmap(bitmap);
	  }
#ifdef DEBUG
	  debug(3,"do_imgbox_scale():loaded bitmap\n");
#endif
       } // endif (if bitmap)
    } // endif
} // do_imgbox_scale

do_imgbox(int i, char *imgdir, char *iname) {
   // I added the new extension testing, but commented it out here
   // because I want to make sure it works well elsewhere first
  PALETTE p;
  BITMAP *sc_bitmap;
  char picname[350];
//  char picname[350],picnoext[346];
  int dsx,dsy;

  get_palette(p);
//  for(i=0;i<12;i++) {
printf("picname (jk) boxtype(%d) enabled:%d\n",i,imgbx[i].enabled);
    if(imgbx[i].enabled==1) {
       sprintf(picname,"%s%c%s%s.pcx",imgdir,mysep,imgbx[i].pfx,iname);
       //sprintf(picnoext,"%s%c%s%s",imgdir,mysep,imgbx[i].pfx,iname);
       //AddPicExt(picname,picnoext);
      printf("NEW: boxtype(%d) looking for picname:%s\n",i,picname);
      bitmap=load_bitmap(picname,p);
      if(imgbx_bmp[i] && i!=B_KEYBOARD)
        masked_blit(imgbx_bmp[i], screen,0,0,imgbx[i].x+rx0,imgbx[i].y+ry0,imgbx[i].w,imgbx[i].h);

      if(bitmap) {
        // TODO: Centering and maybe scaling (course scaling)
        if(bitmap->w > imgbx[i].w || bitmap->h > imgbx[i].h) {
#ifdef DEBUG
	   sprintf(debugtxt,"Scaling: bitmap(w):%d  bitmap(h):%d\n",bitmap->w,bitmap->h);
	   debug(3,debugtxt);
	   sprintf(debugtxt,"Scaling: box(w):%d  box(h):%d\n",imgbx[i].w,imgbx[i].h);
	   debug(3,debugtxt);
#endif
           sc_bitmap=sa_scale_bm(bitmap,imgbx[i].w,imgbx[i].h);
	} else
           sc_bitmap=bitmap;
        dsx=((imgbx[i].w-sc_bitmap->w)/2)+imgbx[i].x+rx0;
        dsy=((imgbx[i].h-sc_bitmap->h)/2)+imgbx[i].y+ry0;

	LOG(2, ("loaded bitmap\n"));
        blit(sc_bitmap,screen,0,0,dsx,dsy,sc_bitmap->w,sc_bitmap->h);
        if(sc_bitmap==bitmap) 
	  sc_bitmap=NULL;
        else
          destroy_bitmap(sc_bitmap);
	destroy_bitmap(bitmap);
      } // endif
    } // endif
//  } // end for
} // do_imgbox

do_imgboxes(char *imgdir, char *iname) {
  int i;
  // only really need to go to 8, not 12
  for(i=0;i<8;i++)
//   do_imgbox(i, imgdir, iname);
   do_imgbox_scale(i, imgdir, iname);
}

//disp_image(char *fname) {
//   PALETTE p;
//   get_palette(p);
//   bitmap=load_bitmap(fname,p);
//#ifdef DEBUG
//   printf("Loading Image: %s\n",fname);
//   printf("DBG: after load_bit\n");
//#endif
//   if(bitmap) {
//     blit(bitmap,screen,0,0,rc.pb_x+2+rx0,rc.pb_y+2+ry0,rc.pb_w,rc.pb_h);
//     destroy_bitmap(bitmap);
//   } else {
//#ifdef DEBUG
//      printf("Copying default image\n");
//#endif
//   if(defbmp) 
//	blit(defbmp,screen,0,0,rc.pb_x+2+rx0,rc.pb_y+2+ry0,rc.pb_w,rc.pb_h);
////	blit(defbmp,screen,0,0,366+rx0,98+ry0,240,148);
//   }
//   
//	
//#ifdef DEBUG
//   printf("DBG: after blit\n");
//#endif
//}

gradient(int x1, int y1, int x2, int y2) {
   int r,g,b,cy,rc;
   
   g=0;
   for(cy=y1;cy<y2;cy++) {
      b=cy / 2;
      r=cy / 2;
      rc=makecol16(r,g,b); 
      hline(screen, x1, cy, x2, rc);
   }
}

set_bg() {
   /* Set background
    * 
    * Either a default gradient or a 640x480 bitmap in any format
    * that allegro supports */
   
   char fname[90];
   PALETTE p; 
   
//   if(widescreen==1) strcpy(bgpic,bgwpic);
   if(strncmp(bgpic, "default", 7)==0) {
      gradient(2+rx0,2+ry0,minx-2,miny-2);
   } else {
      get_palette(p);
      if(strncmp(gthemedir, "na", 2) == 0)
        sprintf(fname,"%s/%s",picsdir,bgpic);
      else
	sprintf(fname,"%s/%s",gthemedir,bgpic);
      
#ifdef DEBUG
      sprintf(debugtxt,"set_bg: gthemedir:%s\n",gthemedir);
      debug(3,debugtxt);
//      sprintf(debugtxt,"set_bg: ws:%d  file:%s\n",widescreen,bgpic);
//      debug(3,debugtxt);
      sprintf(debugtxt,"set_bg: load_bitmap:%s\n",fname);
      debug(3,debugtxt);
#endif
      bitmap=load_bitmap(fname,p);
      if(bitmap) {
	 //	   blit(bitmap,screen,0,0,rx0,ry0,640,480);
	 blit(bitmap,screen,0,0,rx0,ry0,usex,usey);
	 destroy_bitmap(bitmap);
      }      
   }   
}

comp_load(int x1, int y1, int x2, int y2, char *picname) {
   PALETTE p;
   char fname[90];
   get_palette(p);
   if(strncmp(gthemedir, "na", 2) == 0)
     sprintf(fname,"%s/%s",picsdir,picname);
   else
     sprintf(fname,"%s/%s",gthemedir,picname);
//   printf("\nfname is: %s\n", fname); 

   bitmap=load_bitmap(fname,p);
   if(bitmap) {
      masked_blit(bitmap, screen,0,0,x1+rx0,y1+ry0,x2,y2);
//      blit(bitmap, screen,0,0,x1+rx0,y1+ry0,x2,y2);
      destroy_bitmap(bitmap);
   }   
}

draw_title(int fg, int bg) {
   if(strncmp(titlebox, "default", 7)==0) {
      bbox(rc.bb_x+rx0, rc.bb_y+ry0, rc.bb_x2+rx0, rc.bb_y2+ry0, fg, bg);
   } else {
      comp_load(rc.bb_x,rc.bb_y,rc.bb_w,rc.bb_h,titlebox);
   }
   if(!titlemap)
      titlemap=create_bitmap(rc.bb_w,rc.bb_h);
   blit(screen,titlemap,rc.bb_x+rx0,rc.bb_y+ry0,0,0,rc.bb_w,rc.bb_h);
   title(-1,-1,"Emufe");
}

// Deprecated 
//draw_picbox(int fg, int bg) {
//   
//   if(strncmp(picbox, "default", 7)==0) {
//      bbox(rc.pb_x+rx0, rc.pb_y+ry0, rc.pb_x2+rx0, rc.pb_y2+ry0, fg, bg);
//   } else {
//      comp_load(rc.pb_x,rc.pb_y,rc.pb_w,rc.pb_h,picbox);
//   }
//}

void draw_imgbx(int boxno) {
   // load background bitmaps for the image boxes, and draw them
   int bgcol;
   char pathname[255];
//#ifdef SDL2
//   SDL_Surface *junkit;
//#endif
   PALETTE p; 

#ifdef DEBUG
   sprintf(debugtxt,"in draw_imgbx(%d)\n",boxno);
   debug(3,debugtxt);
#endif
   if(imgbx[boxno].enabled==1) {
      destroy_bitmap(imgbx_bmp[boxno]);
      if(strncmp(imgbx[boxno].imgname, "na", 2)==0) {
	imgbx_bmp[boxno]=create_bitmap(imgbx[boxno].w,imgbx[boxno].h);
	bgcol=makecol16(imgbx[boxno].r,imgbx[boxno].g,imgbx[boxno].b);
	rectfill(imgbx_bmp[boxno],0,0,imgbx[boxno].w,imgbx[boxno].h,bgcol);
     } else {
	if(strncmp(gthemedir, "na", 2) == 0)
	  sprintf(pathname,"%s%c%s",picsdir,mysep,imgbx[boxno].imgname);
	else
	  sprintf(pathname,"%s%c%s",gthemedir,mysep,imgbx[boxno].imgname);
#ifdef DEBUG
	sprintf(debugtxt,"IMGBOX #%d: %s\n",boxno,pathname);
	debug(3,debugtxt);
#endif
	imgbx_bmp[boxno]=load_bitmap(pathname,p);
     }
      if(imgbx_bmp[boxno])
	blit(imgbx_bmp[boxno], screen,0,0,imgbx[boxno].x+rx0,imgbx[boxno].y+ry0,imgbx[boxno].w,imgbx[boxno].h);
      else
	printf("IMGBOX failed\n");
   }
   
   // new: load overlays
   
   if(imgbx[boxno].ovpct>0) {
      // making an assumption that if overlay is set, a filename has
      // been set, because that's how it currently loads.
      destroy_bitmap(imgbx_ovl[boxno]);
      if(strncmp(gthemedir, "na", 2) == 0)
	sprintf(pathname,"%s%c%s",picsdir,mysep,imgbx[boxno].ovname);
      else
	sprintf(pathname,"%s%c%s",gthemedir,mysep,imgbx[boxno].ovname);

#ifdef DEBUG
      sprintf(debugtxt,"Overlay: #%d: %s\n",boxno,pathname);
      debug(3,debugtxt);
#endif
      imgbx_ovl[boxno]=load_bitmap(pathname,p);
      sa_setalphablendmode(imgbx_ovl[boxno]);
//#ifdef SDL_BLENDMODE_BLEND
//      SDL_SetSurfaceBlendMode(imgbx_ovl[boxno],SDL_BLENDMODE_BLEND);
//#endif
   }
}

draw_menubox(int fg, int bg) {
   
   if(strncmp(menubox, "default", 7)==0) {
      bbox(rc.mb_x+rx0, rc.mb_y+ry0, rc.mb_x2+rx0, rc.mb_y2+ry0, fg, bg);
   }
   if(strcmp(menubox, "trans")==0) {
      menumap=create_bitmap(rc.mb_w,rc.mb_h);
#ifdef USESDL
      //     SDL only
      rectfill(menumap, 0,0 , rc.mb_w, rc.mb_h, makecol16(0,0,0));
//      SDL_SetAlpha(menumap, SDL_SRCALPHA, 80);
      sa_setalpha(menumap, 80);
      blit(menumap,screen,0,0,rc.mb_x+rx0,rc.mb_y+ry0,rc.mb_w,rc.mb_h);
//      SDL_SetAlpha(menumap, SDL_SRCALPHA, 255);
      sa_setalpha(menumap, 255);
      // End SDL
#endif
      blit(screen, menumap,rc.mb_x+rx0,rc.mb_y+ry0,0,0,rc.mb_w,rc.mb_h);
      usembmap=1;
   }
   if(strncmp(menubox, "default", 7)!=0 && strcmp(menubox, "trans")!=0) {
     comp_load(rc.mb_x,rc.mb_y,rc.mb_w,rc.mb_h,menubox);
      if(usembmap==0)
	menumap=create_bitmap(rc.mb_w,rc.mb_h);
      blit(screen, menumap,rc.mb_x+rx0,rc.mb_y+ry0,0,0,rc.mb_w,rc.mb_h);
      usembmap=1;
   }
}

draw_desc(int fg, int bg) {
   if(strncmp(descbox, "default", 7)==0) {
      bbox(rc.db_x+rx0, rc.db_y+ry0, rc.db_x2+rx0, rc.db_y2+ry0, fg, bg);
      printf("drawdesc: %d,%d\n",rc.db_x2,rc.db_y2);
   } else {
//      printf("comp_load: %s\n",descbox);
    if(usedbmap==0) {
       comp_load(rc.db_x, rc.db_y, rc.db_w, rc.db_h, descbox);
       descmap=create_bitmap(rc.db_w,rc.db_h);
       blit(screen, descmap,rc.db_x+rx0,rc.db_y+ry0,0,0,rc.db_w,rc.db_h);
       usedbmap=1;
    } else
	blit(descmap,screen, 0,0,rc.db_x+rx0,rc.db_y+ry0,rc.db_w,rc.db_h);
   }
}

updir_old(char *s) {
   char *c;

   c=strrchr(s,'/');
   c[0]='\0';
   c=strrchr(s,'/');
   if(!c) {
      s[0]='\0';
   } else {
      c[0]='/';
      c[1]='\0';
   }
}

updir(char *s) {
   char *c;

   c=strrchr(s,mysep);
   c[0]='\0';
   c=strrchr(s,mysep);
   if(!c) {
      s[0]='\0';
   } else {
      c[0]=mysep;
      c[1]='\0';
   }
}

updir_safer(char *dirstr) {
   char c[250];
   int i, sl=0;
   strcpy(c,dirstr);
   for(i=strlen(c);i>=0;i--) {
      if(c[i]==mysep && (i<strlen(c)-1))
	sl=i;
   }
   strncpy(dirstr,c,sl);
   dirstr[sl+1]='\0';
//   printf("start pos is %d, strlen is %d\n",sl,strlen(c));
//   printf("new dir is %s\n",dirstr);
}

int find_entry(char stletter, int startpos) {
   /* This procedure finds a menu entry that begins with the key pressed */
   int i, r;
   char altletter;  /* For case insensitivity */
   
   if ( stletter > 64 && stletter < 91 ) 
     altletter=stletter+32;
   else
     altletter=stletter;
   
/*   printf("startpos is %d\n",startpos); */
   r=-1;
   for(i=startpos+1; i<menulength+1; i++) {
/*	printf("test %s\n", names[i]);
 	printf("test %c\n", names[i][0]); */
        if (stletter == menu[i].name[0] || altletter == menu[i].name[0]) {
	   r=i;
	   break;
	}
      
   }
   if(r==-1 && startpos > 0) 
     {
	r=find_entry(stletter, 0);
     }
   return r;
}

int ef_shutdown() {
/*   allegro_exit(); */
   exit(0);
}

// does this only work with SDL?
Uint32 timercb(Uint32 interval, void *param) {
   jflag=0;
   return(interval);  // If you don't do this, it gets slower
}

int print_version() {
   printf("%s Version %s\n",PACKAGE_NAME,VERSION);
   fnt_version_info();
   widget_version_info();
#ifdef USESDL
   sa_version_info();
#endif
}

/* ****************************************
 *  Special Arcade Font section for titles
 * ****************************************
*/
int display_char_16x32(BITMAP *b,int x, int y, unsigned char chr,int fcolor) {
   // Display 80's arcade style font
   int t,c,dc,cx;
   int fh=ActiveFont->height;
   unsigned char* fdata;
   BITMAP* bfdata;
#ifdef USESDL
   SDL_Rect srect, drect;
#endif

   if(ActiveFont->type==BIOS_8X8 || ActiveFont->type==BIOS_8X16) {
      fdata=ActiveFont->data;
      for(t=0; t<fh; t++) {
	 //      c=fontdata[(chr*16)+t];
	 c=fdata[(chr*fh)+t];
	 for(cx=0;cx<8;cx++) {
	    c<<=1;
	    if(c>255) {
	       //	    putpixel(b, x+cx, y+t, fnfgcol);
	       putpixel(b, x+(cx*2), y+(t*2), fcolor);
	       c=c-256;  
	    }
	 }
      }
   }
} // display_char_16x32

int print_string_16x32(BITMAP *b, int x, int y, char *str, int fg, int bg, int sd) {
   // New method that can draw on any bitmap, and hand solid, shadow and plain
   int c=0, l=0, cx,cy, sl=strlen(str)*16;
   int fh=(ActiveFont->height*2)-1;

   if(bg>-1)
     rectfill(b,x,y,x+sl,y+fh,bg);
#ifdef USESDL
      // Put here for speed optimization
      //   if(SDL_MUSTLOCK(screen)){
      if(b==screen && ActiveFont->type<2)
	if(SDL_LockSurface(b) < 0) return;
#endif
      
      while(*str) {
	 if (*str == '\n'){
	    l++;
	    str++;
	    c=0;
	 } else {
	    cx=x+(c++)*16;
	    cy=y+l*9;
	    if(sd>-1) // shadow
	      display_char_16x32(b, cx+1, cy+1, *(str),sd);
	    display_char_16x32(b, cx, cy, *(str++),fg);
	 }
      }
      
      
#ifdef USESDL
      if(b==screen) {
	 if(ActiveFont->type<2)
	   SDL_UnlockSurface(b);
//# ifdef SDL1
//	 if(SA_AUTOUPDATE==1)
//	   SDL_UpdateRect(screen,x,y,sl,16);
//# endif
//# ifdef SDL2
	 if(SA_AUTOUPDATE==1)
	   s2a_updaterect(screen,x,y,sl,16);
//# endif
	 //      printf("hokee: %d %d %d\n",x,y,sl);
      }
#endif
   
} // print_string_16x32

/* ****************************************
 *  End special font section
 */

int main(int argc, char* argv[]) {
   long w;
   char type, newmenu[300], *ww, picname[180], letsel, usemenu[25];
   int keyp, index, slc, menuitems, stln, por, tmpc;
   int my, mp, entidx, pslc, zpos, jlf, jrt, jup, jdn, jbu, menf;
   int aidx;
   
   imenu.noexec=imenu.autosel=0;
   usembmap=0; usedbmap=0;
   por=0;
   jlf=jrt=jup=jdn=0;
   jbu=1;
   
//   printf("DMM: SDL_BYTEORDER %d\n",SDL_BYTEORDER);
//   printf("DMM: SDL_BIG_ENDIAN %d\n",SDL_BIG_ENDIAN);
//   printf("DMM: SDL_LITTLE_ENDIAN %d\n",SDL_LITTLE_ENDIAN);
   
   
   strcpy(cdroot,"");
   if(getenv("CDROOT"))
     strcpy(cdroot,getenv("CDROOT"));
   find_datadir(basedir,argv[0]);
   fname=(char *)malloc(480);
   strcpy(rcfilename,"emufe.rc");
   strcpy(startdir,"");
   strcpy(lastitem,"");
   imenu.mode=0;
   if(argc > 1 && strcmp(argv[1],"-n")==0 ) {
//      printf("using NEW cmd line processing\n");
      for(aidx=0;aidx<argc;aidx++) {
	 if(strcmp("-i",argv[aidx])==0) imenu.mode=1;
	 if(strcmp("-ac",argv[aidx])==0) imenu.autosel=1;
	 if(strcmp("-nx",argv[aidx])==0) imenu.noexec=1;
	 if(strcmp("-rc",argv[aidx])==0) strcpy(rcfilename,argv[aidx+1]);
	 if(strcmp("-last",argv[aidx])==0) strcpy(lastitem,argv[aidx+1]);
	 if(strcmp("-sd",argv[aidx])==0) strcpy(startdir,argv[aidx+1]);
	 if(strcmp("-v",argv[aidx])==0) { print_version(); exit(0); }
	 if(strcmp("-res",argv[aidx])==0) {
	    hss_index(restr,argv[aidx+1],0,'x');
	    usex=atoi(restr);
	    hss_index(restr,argv[aidx+1],1,'x');
	    usey=atoi(restr);
	    if(usex<minx || usey<miny) {
	       printf("Error: Minimum resolution is %dx%d\n",minx,miny);
	       exit(0);
	    }
	    rx0=(usex-minx)/2;
	    ry0=(usey-miny)/2;
	 }
	 if(strcmp("-b",argv[aidx])==0) {
	    mod_readbm(argv[aidx+1]);
	    // might want to rm file here
	    fileio_dirname(startdir,imenu.game);
	    strcat(startdir,"/");
	    strcpy(dirname,imenu.sysbase); // for now
	    emu_basename(lastitem,imenu.game);
//	    printf ("setting dirname to %s\n",startdir);
	 }
	 if(strcmp("-c",argv[aidx])==0) {
	    abs_dirname(cdroot,argv[0]);
	    find_datadir(basedir,argv[0]);  //recheck this
//	    printf("basedir is now %s\n",basedir);
	 }
	 if(strcmp("-test",argv[aidx])==0) {
	    //system:emulator:sysbase:game
	    hss_index(imenu.system,argv[aidx+1],0,':');
	    hss_index(imenu.emulator,argv[aidx+1],1,':');
	    hss_index(imenu.sysbase,argv[aidx+1],2,':');
	    hss_index(imenu.game,argv[aidx+1],3,':');
	    imenu.noexec=2;
	    module_exec();
	    exit(0);
	 }
      }
#ifdef DEBUG
      sprintf(debugtxt,"rx0=%d   ry0=%d\n",rx0,ry0);
      debug(3,debugtxt);
      sprintf(debugtxt,"Use CDROOT: %s\n",cdroot); 
      debug(1,debugtxt);
#endif
      init();
   } else {
      if(argc > 1) strcpy(rcfilename,argv[1]);
      init();
      /* new code
    * only dirname[0]=0; is original */
      if(argc > 2) strcpy(startdir,argv[2]);
      if(argc > 3) strcpy(lastitem,argv[3]);
   }
      
   if(strlen(startdir)>0) {
      strcpy(dirname, startdir);
      sprintf(newmenu,"%s%c%sindex.menu",basedir,mysep, startdir);
   } else {
      dirname[0]=0;
      sprintf(newmenu,"%s%c%s",basedir,mysep,menuname);
   }
   zpos=mouse_z;
   
   slc=1; index=1; mp=0; 
   draw_screen();
   // Setup selection for new selection type
   selection=create_bitmap(rc.mb_w,16);
   rectfill(selection,0,0,rc.mb_w,16,makecol(texthlr,texthlg,texthlb));
//   rect(selection,0,0,rc.mb_w,16,makecol(255,255,255));
//   SDL_SetAlpha(selection, SDL_SRCALPHA, 128);
   sa_setalpha(selection, 128);

   
   menuitems=load_menu(newmenu);
   if(strlen(lastitem)>0) {
      slc=index=find_menu_e(lastitem);
   }
   display_menu(index);
   menu_hlight(index,slc);
   clear_keybuf(); 
/*   position_mouse(320,200); */
   show_mouse(screen);
   imenu.no_launch=0;
   
   if(joy_enable==1)
     install_int(timercb,300);
   while(1) {
      // SDL only,  rest() is the SDL equiv
//      SDL_WaitEvent(NULL);
      if(cdclock!=1 && (jflag==0 && joy_enable==1))      
	rest(0);
      my=mouse_y;

      /* START JOYSTICK SECTION */

      if ( joy_enable == 1 ) 
	{ 
	   poll_joystick();
	   
	   /* Joystick Up */
	   if(joy[0].stick[0].axis[1].d1 && jflag==0) {
#ifdef USESDL
	      s2a_sim_keypress(KEY_UP);
#else
	      simulate_keypress(KEY_UP << 8);
#endif
	      jflag=1;
	   }
	   if(joy[0].stick[0].axis[1].d1==0) jup=0; 
	     
	   /* Joystick Down */
	   if(joy[0].stick[0].axis[1].d2 && jflag==0) {
#ifdef USESDL
	      s2a_sim_keypress(KEY_DOWN);
#else
	      simulate_keypress(KEY_DOWN << 8);
#endif
	      jflag=1;
	   }
	   if(!joy[0].stick[0].axis[1].d2) jdn=0; 
	   
	   /* Joystick Button A or B */
	   if(joy[0].button[0].b && jflag==0) {
	      simulate_keypress(KEY_ENTER << 8);
	      jflag=1;
	   }
	   
	   if(joy[0].button[1].b && jflag==0) {
	      simulate_keypress(KEY_ENTER << 8);
	      jflag=1;
	   }
	   
	   if(!joy[0].button[0].b && !joy[0].button[1].b 
	       && !joy[0].stick[0].axis[1].d2 
	      && !joy[0].stick[0].axis[1].d1) jflag=0;
	}
      
      /* END JOYSTICK SECTION */

      
      if (mouse_b & 1 && mouse_x > (rc.mb_x+rx0) && mouse_x < (rc.mb_x2+rx0) && my > (rc.mb_y+ry0) && my < (rc.mb_y2+ry0) && mp==0) {
	 entidx=(my-100)/16;
	 if (entidx < (menuitems-index+1) )  {
	    menu_uhlight(index,slc);
	    pslc=slc;
	    slc=entidx+index;
	    menu_hlight(index,slc);
/*	    printf("button pressed entry %d my:%d\n",entidx,my); */
	 /* Mouse pressed */
	    mp=1;
	    /* If we click on the same thing twice, it's selected */
	    if(pslc!=slc) por=0;
	    /* This is kinda kludgy, but I don't want to rewrite a lot of
	     * code right now */
	    while (mouse_b & 1) 
	      {
		 if(mouse_needs_poll() )
		   poll_mouse();   // Needed for sdl_allegro
	      }
	    
//	    simulate_keypress(17165);
	    simulate_keypress(KEY_ENTER << 8);
	 }
	 
      }
//      if (mouse_b & 1 && mouse_x > (rc.mb_x+rx0) && mouse_x < (rc.mb_x2+rx0) && my > (rc.mb_y+ry0) && my < (rc.mb_y+16+ry0) && mp==0) {
//	 /* This is basically the same as pushing the up arrow, so we
//	  * may as well force the pushing of the up arrow */
////	    simulate_keypress(21504);
//	    simulate_keypress(KEY_UP << 8);
//      }
//
//      if (mouse_b & 1 && mouse_x > (32+rx0) && mouse_x < (348+rx0) && my > (ry0+243) && my < (ry0+260) && mp==0) {
//	 /* This is basically the same as pushing the down arrow, so we
//	  * may as well force the pushing of the up arrow */
////	    simulate_keypress(21760);
//	    simulate_keypress(KEY_DOWN << 8);
//      }
      if (mouse_b & 4 && mp==0) {
	 /* Middle button simulates an enter key press */
	   simulate_keypress(KEY_ENTER << 8);
	   mp=1;
      }
      
      /* Wheel support up */
      if (mouse_z < zpos) {
#ifdef USESDL
	 s2a_sim_keypress(KEY_UP);
#else
	 simulate_keypress(KEY_UP << 8);
#endif
	 zpos=mouse_z;
      }

      /* Wheel support down */
      if (mouse_z > zpos) {
#ifdef USESDL
	 s2a_sim_keypress(KEY_DOWN);
#else
	 simulate_keypress(KEY_DOWN << 8);
#endif
	 zpos=mouse_z;
      }
      
      if(!mouse_b) { mp=0; }

      
      /* Countdown clock */
      if (cdclock == 1) {
	 endtime = time(NULL);
	 /* One Second Later */
	 if(endtime > starttime + 0) {
	      if(menu[slc].type == 'i'  || menu[slc].type == 's') {
		 display_info(slc);
		 por=1;
	      }
	      cdclock=0;  // will moving this out of the if fix the
	                  // bug?
	   }
      }
      /* End Countdown clock */
      
      if(keypressed()) {
	 keyp=readkey() >> 8; 
	 printf("key:%d--%d\n",keyp,KEY_DOWN);
#ifdef DEBUG
	 printf(debugtxt,"keyp=%d\n",keyp,KEY_DOWN);
	 debug(3,debugtxt);
#endif
	 if(keyp==KEY_F2) {
//	    run_setup("setup1");
	    setup_go();
//	    setup_test();
	 }
	 if(keyp==KEY_F11) {
	    if(fullscr=='n') {
	       set_gfx_mode(GFX_AUTODETECT_FULLSCREEN,usex,usey,0,0);
	       fullscr='y';
	    } else {
	       set_gfx_mode(GFX_AUTODETECT_WINDOWED,usex,usey,0,0);
	       fullscr='n';
	    }
	    restore_screen(index,slc);
	 }
	 if(keyp==KEY_DOWN) {
	    /* Down Arrow */
	    imenu.no_launch=0;
	    cdclock=1; starttime=time(NULL);
            if(por>0) rem_info_bar();
	    por=0;
	    menu_uhlight(index,slc);
	    if(slc < menuitems)
	      slc++;
	    if(slc>index+((rc.mb_h/rc.font_h)-1)) {
	       index++;
	       restore_menuback();
	       display_menu(index);
	    }
#ifdef DEBUG
	    sprintf(debugtxt,"slc is %d index is %d\n",slc, index);
	    debug(3,debugtxt);
	    sprintf(debugtxt,"Type: %c",menu[slc].type);
	    debug(3,debugtxt);
#endif
	    menu_hlight(index,slc);
	 }
	 if(keyp==KEY_PGDN) {
	    /* Page Down */
	    imenu.no_launch=0;
	    cdclock=1; starttime=time(NULL);
            if(por>0) rem_info_bar();
	    por=0;
	    menu_uhlight(index,slc);
	    slc=slc+8;
	    index=index+8;
	    if(slc > menuitems) 
	      {		 
		 tmpc=(slc-menuitems);
		 slc=slc-tmpc;
		 index=index-tmpc;
	      }
	    
	    restore_menuback();
	    display_menu(index);

#ifdef DEBUG
	    sprintf(debugtxt,"Type: %c",menu[slc].type);
	    debug(3,debugtxt);
#endif
	    menu_hlight(index,slc);
	 }
	 
	 if(keyp==KEY_UP) {
	    /* Up arrow */
	    imenu.no_launch=0;
	    cdclock=1; starttime=time(NULL);
            if(por>0) rem_info_bar();
	    por=0;
	    menu_uhlight(index,slc);
	    if(slc > 1)
	      slc--;
	    if(slc<index) {
	       index--;
	       restore_menuback();
	       display_menu(index);
	    }
	    menu_hlight(index,slc);
	 }
	 /* HOME */
	 if(keyp==KEY_HOME) {
	    imenu.no_launch=0;
	    menu_uhlight(index,slc);
	    slc=index=1;
	    restore_menuback();
	    display_menu(index);
	    menu_hlight(index,slc);
	 }

	 /* END */
	 if(keyp==KEY_END) {
	    imenu.no_launch=0;
	    menu_uhlight(index,slc);
	    slc=index=menulength;
	    restore_menuback();
	    display_menu(index);
	    menu_hlight(index,slc);
	 }
	 
	 /* Page UP */
	 if(keyp==KEY_PGUP) {
	    imenu.no_launch=0;
	    cdclock=1; starttime=time(NULL);
            if(por>0) rem_info_bar();
	    por=0;
	    menu_uhlight(index,slc);
	    slc=slc-8;
	    index=index-8;
	    if(slc < 1) 
	      {		 
		 slc=1;
		 index=1;
	      }
	    /* This was added to prevent the crashing problem
	     * which occured when index became negative.  I want
	     * to keep an eye on this to make sure that slc is doing 
	     * the right thing (ie nothing) when index gets reset to 1
	     */
	    if ( index < 1 )  index = 1; 
		 
	    restore_menuback();
	    display_menu(index);

#ifdef DEBUG
	    sprintf(debugtxt,"slc is %d index is %d\n",slc, index);
	    debug(3,debugtxt);
	    sprintf(debugtxt,"Type: %c",menu[slc].type);
	    debug(3,debugtxt);
#endif
	    menu_hlight(index,slc);
	 }

	 if(keyp==KEY_LEFT) {
	    // hand left key as back, except at top level
	    keyp=KEY_ESC;
	    for(tmpc=0;tmpc<8;tmpc++){
	       // if there is a back function in this menu
	       // change to enter key and move menu to the
	       // u back function
	       if(menu[tmpc].type=='u') {
		  keyp=KEY_ENTER;
		  slc=tmpc;
		  break;
	       }
	    }
	 }
	 
	 
	 if(keyp==KEY_ENTER || keyp==KEY_ENTER_PAD || keyp==KEY_RIGHT) {
	    cdclock=0;
	    type=menu[slc].type;
#ifdef DEBUG
	    sprintf(debugtxt,"SLC was %d\n",slc);
	    debug(3,debugtxt);
#endif
	    if( type=='m' ) {
#ifdef DEBUG
	       sprintf(debugtxt,"Loading File___%s\n",menu[slc].rom);
	       debug(3,debugtxt);
#endif
	       sprintf(newmenu, "%s/%s", dirname, menu[slc].rom);
	       menuitems=load_menu(newmenu);
	       index=1;slc=1;
	       restore_menuback();
               display_menu(index);
               menu_hlight(index,slc);
	    }
	    if( type=='d' ) {
	       ww=menu[slc].rom;
#ifdef DEBUG
	       sprintf(debugtxt,"Loading Directory___%s\n",menu[slc].rom);
	       debug(3,debugtxt);
	       sprintf(debugtxt,"dirname is %s\n",dirname);
	       debug(3,debugtxt);
	       sprintf(debugtxt,"romname is %s\n",ww);
	       debug(3,debugtxt);
#endif
	       /* strcat is very picky, I can only seem to get it to 
		work if the first parameter is a static string, and the
		second is dynamic or constant */

	       sprintf(dirname,"%s%s%c",dirname,menu[slc].rom,mysep);
	       sprintf(newmenu,"%s%c%sindex.menu",basedir,mysep,dirname);
#ifdef DEBUG
	       sprintf(debugtxt,"dirname is now %s\n",newmenu);
	       debug(3,debugtxt);
#endif
	       menuitems=load_menu(newmenu);
//	       printf("menuitems is %d\n",menuitems);
	       index=1;slc=1;
	       if(menuitems==2 && imenu.autosel==1) {
		  slc=2;
		  simulate_keypress(KEY_ENTER << 8);
	       }
               restore_menuback();
	       display_menu(index);
               menu_hlight(index,slc);
	    }
	    
	    if( type=='u' && keyp != KEY_RIGHT ) {
	       sprintf(usemenu,"index.menu");
	       
	       if(strlen(dirname)==0) {
		  if(imenu.mode==0) {   // Old way
		     printf("BACK\n");
		     break;
		  }
	       } 
	       if(strcmp(imenu.sysbase,dirname)==0 && imenu.mode>1) {
		  imenu.mode--;
	       } else {
		  updir(dirname);
		  if(strcmp(imenu.sysbase,dirname)==0 && imenu.mode>1) {
		     strcpy(usemenu,imenu.lastmenu);
		  }
	       }

	       sprintf(newmenu,"%s%c%s%s",basedir,mysep,dirname,usemenu);
//		  strcat(newmenu, usemenu);
#ifdef DEBUG
	       sprintf(debugtxt,"newmenu %s   dirname:%s\n",newmenu,dirname);
	       debug(3,debugtxt);
#endif
		  menuitems=load_menu(newmenu);

	       index=1;slc=1;
	       if(menuitems==2 && imenu.autosel==1) {
		  simulate_keypress(KEY_ENTER << 8);
	       }
	       // printf("\nstill here\n");
	       restore_menuback();
               display_menu(index);
               menu_hlight(index,slc);
	    }
	    if( type=='F' )
	      {
		 strcpy(fname, commands[slc]);
		 file_select_ex("Select a Directory", fname, NULL, 80, 520, 350);
		 sprintf(commands[slc],"%s=%s",menu[slc].rom, fname);
		 write_option(slc);
		 sprintf(commands[slc],"%s",fname);
		 snprintf(menu[slc].name, 39, "%s                               ", fname);
		 restore_menuback();
		 display_menu(index);
		 menu_hlight(index,slc);
	      }
//	    if( type=='N' )
//	      setup_go();
	    if( type=='s' )
	      {
	       if(por==0) {
		  display_info(slc);
		  por=1;
	       } else {
		  write_option(slc);
/*		  printf("wrote options\n"); */
		  sprintf(newmenu, "%s/%s", dirname, lmenus[slc]);
/*		  printf("loading menu %s\n",newmenu); */
		  menuitems=load_menu(newmenu);
/*		  printf("loaded menu %s\n",lmenus[slc]); */
		  index=1;slc=1;
		  restore_menuback();
		  display_menu(index);
		  menu_hlight(index,slc);
	       }
	    }
		 
	    if( type=='i' ) {
	       if(por==0) {
		  display_info(slc);
		  por=1;
	       } else {
		  if(imenu.mode>0) {
		     sprintf(imenu.game,"%s%s",dirname,menu[slc].rom);
		     // launch code goes here
		     if(imenu.no_launch!=1) {
			//  Looks like this works for shutting down and restarting gfx mode
			// had to move this post process_cmd:
//			SDL_QuitSubSystem(SDL_INIT_VIDEO); // shutdown gfx
			module_exec();
//			SDL_InitSubSystem(SDL_INIT_VIDEO);// restart gfx
//			setgfxmode(); // restart gfx
			restore_screen(index,slc);
//			title(320,40,imenu.title);
			title(-1,-1,imenu.title);
			cdclock=1; starttime=time(NULL);
			imenu.no_launch=1;
//			por=0;
		     }
//		     poll_keyboard();
//		     while(keypressed()) {
//			keyp=readkey() >> 8;
//			if(keyp != 13) break;
//		     }
//		     keyp=0;
		       
//		     break;
		  } else {
		     printf("%s%s\n",dirname,menu[slc].rom);
		     break;
		  }
	       }
	    }
	    if( type=='a' ) {
	       if(imenu.mode>=1) {
		  // might need to do a basename here.
		  strcpy(imenu.sysbase,dirname);

		  // New profile support 
		  if ( menu[slc].rom[strlen(menu[slc].rom)-2] == '#' ) {
		     imenu.profile=menu[slc].rom[strlen(menu[slc].rom)-1] -48;
		     printf("found a profile: %d\n",imenu.profile);
		     hss_index(imenu.emulator,menu[slc].rom,0,'#');
		  } else  {
		     imenu.profile=0;
		     strcpy(imenu.emulator,menu[slc].rom);
		  }
//		  printf("imenu.sysbase: %s\n",imenu.sysbase);
//		  printf("imenu.emulator: %s\n",imenu.emulator);
//		  sprintf(imenu.emulator,"%s%s",dirname,roms[slc]);
		  imenu.mode++;
		  // Expand this!!!
//		  sprintf(imenu.menu,"%s%s.menu",dirname,roms[slc]);
		  sprintf(imenu.rc,"%s%c%setc%c%s.rc",basedir,mysep,dirname,mysep,imenu.emulator);
		  load_rc(imenu.rc);
		  sprintf(imenu.menu,"%s%c%s%s",basedir,mysep,dirname,menuname);
		  // HERE IS THE BUG
//		  sprintf(imenu.lastmenu,"%s.menu",menu[slc].rom);
		  strcpy(imenu.lastmenu,menuname);

		  menuitems=load_menu(imenu.menu);
		  index=1;slc=1;
		  if(menuitems==2 && imenu.autosel==1) {
		     slc=2;
		     simulate_keypress(KEY_ENTER << 8);
		  }
		  restore_menuback();
		  display_menu(index);
		  menu_hlight(index,slc);
	       }
	       // Old way
	       if(imenu.mode==0) {
		  printf("%s%s\n",dirname,menu[slc].rom);
		  break;
	       }
	    }
	 }
	 if(keyp==KEY_F12) {
	    if ( joy_enable == 1 ) 
	      joy_enable=0;
	    else
	      joy_enable=1;
	 }
	 if(keyp==KEY_ESC) {
	    printf("QUIT\n");
	    if(imenu.mode>0 && imenu.noexec==1)
	      mod_writebm(1);
	    break;
	 }

#ifdef USESDL
         if((keyp >=KEY_0 && keyp <= KEY_9 ) || (keyp >= KEY_A && keyp <= KEY_Z)) {
	    /* If you press a number or letter, it will be processed
	     * here */
	    if(keyp <= KEY_9 && keyp >= KEY_0) {
	       letsel=keyp;
	       /* number */
	    }
	    if(keyp <= KEY_Z && keyp >= KEY_A) {
	       letsel=keyp-32;
	       /* letter */
	    }
#else
         if(keyp > 0 && keyp < 37 ) {
	    /* If you press a number or letter, it will be processed
	     * here */
	    if(keyp < 37 && keyp > 26) {
	       letsel=keyp + 21;
	       /* number */
	    }
	    if(keyp < 27 && keyp > 0) {
	       letsel=keyp + 64;
	       /* letter */
	    }
#endif
	    menf=find_entry(letsel,slc);
	    if(menf > 0) {
	       /* redraw menu, this is the most dangerous part of 
		* the code */
	       cdclock=1; starttime=time(NULL);
	       if(por>0) rem_info_bar();
	       por=0;
	       menu_uhlight(index,slc);
	       if(menf>slc) {
		  index=index+(menf-slc);
	       } else {
		  index=menf;
	       }
	       slc=menf;
	       restore_menuback();
	       display_menu(index);
	       menu_hlight(index,slc);
	    }
	 } 	 
      } /* End if keypressed */
   } /* while */
   
  ef_shutdown();
}

END_OF_MAIN();
