#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <SDL.h>
#include <SDL_image.h>

#ifdef HAVE_LIBSDL2_TTF
#include <SDL_ttf.h>
#endif

#include <dirent.h>
#include <time.h>

extern SDL_Window *sdlWindow;
extern SDL_Renderer *sdlRenderer;
// extern SDL_Surface *screen;
extern char mysep;
extern time_t starttime, endtime;

#define KEY_ENTER SDLK_RETURN

//#include <SDL_mixer.h>
#include <stdio.h>
//#include <dirent.h>
#include <unistd.h>
// #include "globaltypes.h"
#include "emufe.h"
// #include "conffile.h"

#define MAXGAMES 200

#define VERSION 2.0

// conf_t rc;
prop_t rc;
game_t game[MAXGAMES];
char DirImgDefault[128], ExeImgDefault[128], EmuImgDefault[128], UpDImgDefault[128];

#ifdef DEBUG
int himg[MAXGAMES];
#endif
char fullscr, lastaction;
int UIclick = 1;
int UseDfltEmu = 0;

SDL_Texture *bgtexture;
SDL_Rect bgsrc_r, bgdst_r;

SDL_Texture *icon[MAXGAMES];
// fix this
SDL_Texture *gridicon[8][50];

SDL_Texture *shadow, *cursor, *power, *joyicon, *gear;

int ssbg_set=0;
typedef struct img_cache_t {
   char imgname[30];
   int w, h;
   SDL_Texture *texture;
} img_cache_t;

typedef struct dflt_t {
   int entry;
   char name[26];
   char system[26];
} dflt_t;

dflt_t dflt;

img_cache_t img_cache[14];

SDL_Window *window;                    // Declare a pointer
SDL_Renderer *ren;
SDL_Surface *surf;

extern SDL_Surface *imgbx_bmp[12];
extern SDL_Surface *imgbx_mask[12];
extern SDL_Surface *imgbx_ovl[12];

SDL_Joystick* gGameController = NULL;

#ifdef HAVE_LIBSDL2_TTF
TTF_Font *font;
#endif

// audio
SDL_AudioSpec aSpec;
Uint32 wavlength;
Uint8 *wavbuffer;
SDL_AudioDeviceID deviceId;

//Analog joystick dead zone
//const int JOYSTICK_DEAD_ZONE = 8000;
//const int JOYSTICK_DEAD_ZONE = 0;
const int JOYSTICK_DEAD_ZONE = 32000;

#ifdef DEBUG
char s[100];
#endif

int startidx, startrow, row, havejoy, cursor_x=0, topcurs_idx=0;

void grid_flush_key() {
   SDL_FlushEvent(SDL_KEYDOWN);
   SDL_FlushEvent(SDL_KEYUP);
   SDL_FlushEvent(SDL_TEXTEDITING);
   SDL_FlushEvent(SDL_TEXTINPUT);
}

void grid_flush_joy() {
   SDL_FlushEvent(SDL_JOYAXISMOTION);
   SDL_FlushEvent(SDL_JOYHATMOTION);
   SDL_FlushEvent(SDL_JOYBUTTONDOWN);
   SDL_FlushEvent(SDL_JOYBUTTONUP);
   SDL_FlushEvent(SDL_CONTROLLERAXISMOTION);
   SDL_FlushEvent(SDL_CONTROLLERBUTTONDOWN);
   SDL_FlushEvent(SDL_CONTROLLERBUTTONUP);
}


void grid_ttf_print(int x, int y, char *text) {
   // note: Open_TTF sets the point size
   SDL_Rect src_r, dst_r, shd_r;
#ifdef HAVE_LIBSDL2_TTF

   SDL_Color bgcolor = { 0, 0, 0 };
   SDL_Color fgcolor = { 255, 255, 255 };

   SDL_Surface * surface = TTF_RenderText_Blended( font, text, bgcolor);
   SDL_Texture * txtur = SDL_CreateTextureFromSurface(sdlRenderer, surface);

   src_r.x=0; src_r.y=0; src_r.w=surface->w; src_r.h=surface->h;
   dst_r.x=x; dst_r.y=y; dst_r.w=src_r.w; dst_r.h=src_r.h;

   SDL_RenderCopy(sdlRenderer, txtur, &src_r, &dst_r);
   SDL_DestroyTexture(txtur);
   SDL_FreeSurface(surface);

   SDL_Surface * surface2 = TTF_RenderText_Blended( font, text, fgcolor);
   SDL_Texture * txtur2 = SDL_CreateTextureFromSurface(sdlRenderer, surface2);

   dst_r.x=x-4; dst_r.y=y-4; dst_r.w=src_r.w; dst_r.h=src_r.h;

   SDL_RenderCopy(sdlRenderer, txtur2, &src_r, &dst_r);
   SDL_DestroyTexture(txtur2);
   SDL_FreeSurface(surface2);
#endif
} // grid_ttf_print();


void grid_ttf_print_scale(int x, int y, char *text, int scale_x, int scale_y, int scale_p ) {
   // note scale_p =1 means scale positioning as well as size
   SDL_Rect src_r, dst_r, shd_r;
#ifdef HAVE_LIBSDL2_TTF

   SDL_Color bgcolor = { 0, 0, 0 };
   SDL_Color fgcolor = { 255, 255, 255 };

   SDL_Surface * surface = TTF_RenderText_Blended( font, text, bgcolor);
   SDL_Texture * txtur = SDL_CreateTextureFromSurface(sdlRenderer, surface);

   src_r.x=0; src_r.y=0; src_r.w=surface->w; src_r.h=surface->h;
   if (scale_p == 1) {
      dst_r.x=x*scale_x/100; dst_r.y=y*scale_y/100;
   } else {
      dst_r.x=x; dst_r.y=y;
   }

   dst_r.w=src_r.w*scale_x/100; dst_r.h=src_r.h*scale_y/100;

   SDL_RenderCopy(sdlRenderer, txtur, &src_r, &dst_r);
   SDL_DestroyTexture(txtur);
   SDL_FreeSurface(surface);

   SDL_Surface * surface2 = TTF_RenderText_Blended( font, text, fgcolor);
   SDL_Texture * txtur2 = SDL_CreateTextureFromSurface(sdlRenderer, surface2);

   if (scale_p == 1) {
     dst_r.x=x*scale_x/100-4; dst_r.y=y*scale_y/100-4;
   } else {
     dst_r.x=x-4; dst_r.y=y-4;
   }
   dst_r.w=src_r.w*scale_x/100; dst_r.h=src_r.h*scale_y/100;

   SDL_RenderCopy(sdlRenderer, txtur2, &src_r, &dst_r);
   SDL_DestroyTexture(txtur2);
   SDL_FreeSurface(surface2);
#endif
} // grid_ttf_print_scale();

void grid_getSystem(char *dname) {
   // this figures out the system based on dirname
   char emucd[160], lw[40], rw[220], line[256];
   FILE *fp;

   strcpy(dflt.system, ""); // set default
   
   // set defaults
   sprintf(emucd,"%s%cuser_config%ccfg1%cemucd.env",basedir,mysep,mysep,mysep);
   if(!file_exists(emucd)) {
      sprintf(emucd,"%s%cetc%cemucd.env",basedir,mysep,mysep);
   }
   
   if ((fp = fopen(emucd, "rb"))==NULL) {
      printf("Error opening rc file %s", emucd);
   } else {
      while(!feof(fp)) {
	 fgets(line, 255, fp);
	 if( line[0] != '#' ) {
	    line[strlen(line)-1]=0;  /* Strip LF */
            
	    hss_index(rw, line, 1, '=');
	    if(strncmp(rw, dirname, strlen(dirname)-1)==0) {
	       hss_index(lw, line, 0, '=');
	       hss_index(dflt.system, lw, 0, '_');
	    }
	 } // if (line[0] != '#' )
      } // while
      fclose(fp);
   }
} // grid_getSystem()

void grid_DefaultEmu(char *dname) {
   char emufile[240], line[256], w[40];
   FILE *fp;
//   if(UseDfltEmu>0) {
	
      grid_getSystem(dname);

      if(strcmp(dflt.name, "")==0) {
	 sprintf(emufile, "%s%cuser_config%ccfg1%ccfg%c%s.emu",basedir,mysep,mysep,mysep,mysep,dflt.system);

	 if(file_exists(emufile)) {
	    if ((fp = fopen(emufile, "rb"))==NULL) {
	       printf("Error opening file %s", emufile);
	    } else {
	       while(!feof(fp)) {   // read default emu file
		  fgets(line, 255, fp);
		  hss_index(w, line, 0, '|');
		  if(strcmp(w,"emulator")==0)
		    hss_index(dflt.name, line, 1, '|');
	       }
	       fclose(fp);
	    } // fopen
	 } // file_exists
      } 
   //   }
} // grid_DefaultEmu


void load_ribbon(int start)  {
   // initial load of icons
   int i;
   char pathname[140];

//   printf("picsdir: %s\n basedir: %s\n", picsdir, basedir);
   
   for(i=0; i<rc.numgames; i++) {
      SDL_DestroyTexture(icon[i]);
#ifdef DEBUG
      himg[i]=0;
#endif
   }

   for(i=0; i<8; i++) {
#ifdef DEBUG
//      printf("%d:%d   %s\n",i, i+start, game[i+start].icon);
      himg[i+start]=1;
#endif
      surf = IMG_Load(game[i+start].icon);
//      SDL_DestroyTexture(icon[i]);
      icon[i+start] = SDL_CreateTextureFromSurface(sdlRenderer, surf);
      SDL_FreeSurface(surf);
   }
   
   // Load Shadow Icon
//   surf = IMG_Load("/export/home/tsmolar/Devel/gamelauncher/src/images/shadow.png");
   sprintf(pathname, "%s%cpics%cshadow.png", basedir, mysep, mysep);
   surf = IMG_Load( pathname );

   SDL_DestroyTexture(shadow);
//   shadow = SDL_CreateTextureFromSurface(ren, surf);
   shadow = SDL_CreateTextureFromSurface(sdlRenderer, surf);
   SDL_FreeSurface(surf);

   // Load Cursor Icon
   sprintf(pathname, "%s%cpics%ccursor2.png", basedir, mysep, mysep);
   surf = IMG_Load( pathname );

//   SDL_DestroyTexture(cursor);
//   cursor = SDL_CreateTextureFromSurface(ren, surf);
   cursor = SDL_CreateTextureFromSurface(sdlRenderer, surf);
   SDL_FreeSurface(surf);
   
   // Load Power Button Icon
   sprintf(pathname, "%s%cpics%cpowerbutton.png", basedir, mysep, mysep);
   surf = IMG_Load( pathname );   

   SDL_DestroyTexture(power);
   power = SDL_CreateTextureFromSurface(sdlRenderer, surf);
   SDL_FreeSurface(surf);

   // Load Gear Icon
   sprintf(pathname, "%s%cpics%cgearicon.png", basedir, mysep, mysep);
   surf = IMG_Load( pathname );   

   SDL_DestroyTexture(gear);
   gear = SDL_CreateTextureFromSurface(sdlRenderer, surf);
   SDL_FreeSurface(surf);

   // Load Joystick Icon
   sprintf(pathname, "%s%cpics%cjoystick.png", basedir, mysep, mysep);
   surf = IMG_Load( pathname );
   
   SDL_DestroyTexture(joyicon);
   joyicon = SDL_CreateTextureFromSurface(sdlRenderer, surf);
   SDL_FreeSurface(surf);
   
} // load_ribbon(int start);


void power_button() {
   // need to scale this for resolution
   SDL_Rect src_r, dst_r;
   
//      rc.icon_power_x=24;  rc.icon_joy_x=96;  rc.icon_gear_x=400;
//      rc.topicons_y=24;  rc.topicons_w=48;  rc.topicons_h=48;

   src_r.x=0; src_r.y=0; src_r.w=100; src_r.h=100;
   dst_r.x=rc.icon_power_x; dst_r.y=rc.topicons_y; 
   dst_r.w=rc.topicons_w; dst_r.h=rc.topicons_h;

   SDL_RenderCopy(sdlRenderer, power, &src_r, &dst_r);
   
   if ( havejoy > 0 ) {
      src_r.x=0; src_r.y=0; src_r.w=100; src_r.h=100;
      dst_r.x=rc.icon_joy_x; dst_r.y=rc.topicons_y; 
      dst_r.w=rc.topicons_w; dst_r.h=rc.topicons_h;
      
      SDL_RenderCopy(sdlRenderer, joyicon, &src_r, &dst_r);
   }
   
   src_r.x=0; src_r.y=0; src_r.w=100; src_r.h=100;
   dst_r.x=rc.icon_gear_x; dst_r.y=rc.topicons_y; 
   dst_r.w=rc.topicons_w; dst_r.h=rc.topicons_h;

   SDL_RenderCopy(sdlRenderer, gear, &src_r, &dst_r);
      
} // power_button()


void grid_cleanup() {
   // Stuff to cleanup before switching to MODE_CLASSIC
   grid_clear();
   SDL_FreeSurface(surf);
} // grid_cleanup()


void grid_load_grid() {
//   int grid_w=6, grid_h=4;

   int ix, iy, curridx;

   rc.numrows = rc.numgames / rc.grid_w;
   if((rc.numgames % rc.grid_w) != 0)
     rc.numrows++;
#ifdef DEBUG
   printf("NUMROWS: %d\n", rc.numrows);
   printf("NUMGAMES: %d\n", rc.numgames);
   printf("grid_w: %d\n", rc.grid_w);
   printf("grid_h: %d\n", rc.grid_h);
#endif
//   for(iy=0;iy<rc.grid_h;iy++) {
   for(iy=0;iy<rc.numrows;iy++) {
      for(ix=0; ix<rc.grid_w; ix++) {
	 curridx=(iy * rc.grid_w) + ix;
#ifdef DEBUG
	 printf("%d:%s\n",curridx, game[curridx].icon);
	 printf("%d:%d: %s\n",curridx, game[curridx].showtext, game[curridx].title);
	 printf("%d:%d: %s\n",curridx, game[curridx].showtext, game[curridx].type);
	 printf("%d:%d: %s\n",curridx, game[curridx].showtext, game[curridx].exec);
	 printf("\n");
	 printf("222 %d:%s\n",curridx, game[curridx].icon);
#endif
	 surf = IMG_Load(game[curridx].icon);
	 gridicon[ix][iy] = SDL_CreateTextureFromSurface(sdlRenderer, surf);
         if(curridx == rc.numgames) break;
      }
      if(curridx == rc.numgames) break;
   }
} // grid_load_grid()

void grid_scroll_up() {
   // scroll grid up (if move down)
   SDL_Rect src_r, dst_r, shd_r;
   int ix, iy, curridx, i, Mstartrow, slc;
   int txtw, txth, txtc;

   Mstartrow=startrow;
   
   // 10 step
   for(i=0; i<10; i++) {
      // top line
      src_r.x=0; src_r.y=i*40; src_r.w=400; src_r.h=(10-i)*40;

      for(ix=0; ix < rc.grid_w; ix++) {
	 dst_r.x=(ix * rc.icon_w) + rc.grid_x; dst_r.y=(0 * rc.icon_h) + rc.grid_y; dst_r.w=rc.img_w; dst_r.h=(10-i) * (rc.img_h/10);
	 shd_r.x=(ix * rc.icon_w) + rc.grid_x + 8; shd_r.y=(0 * rc.icon_h) + rc.grid_y + 8; shd_r.w=rc.img_w; shd_r.h=(10-i) * (rc.img_h/10);

	 curridx=(0 * rc.grid_w) + ix + (Mstartrow * rc.grid_w);

	 if (curridx < rc.numgames) {
	    SDL_RenderCopy(sdlRenderer, shadow, &src_r, &shd_r);
	    SDL_RenderCopy(sdlRenderer, gridicon[ix][0+Mstartrow], &src_r, &dst_r);
	 }
      } // for ix

      
      // middle 
      src_r.x=0; src_r.y=0; src_r.w=400; src_r.h=400;
      
      for(iy=1; iy < (rc.grid_h); iy++) {
	 for(ix=0; ix < rc.grid_w; ix++) {
	    dst_r.x=(ix * rc.icon_w) + rc.grid_x; dst_r.y=(iy * rc.icon_h)-(i*rc.img_h/10) + rc.grid_y; dst_r.w=rc.img_w; dst_r.h=rc.img_h;
	    shd_r.x=(ix * rc.icon_w) + rc.grid_x + 8; shd_r.y=(iy * rc.icon_h)-(i*rc.img_h/10) + rc.grid_y + 8; shd_r.w=rc.img_w; shd_r.h=rc.img_h;
	    
	    curridx=(iy * rc.grid_w) + ix + (Mstartrow * rc.grid_w);
	    
	    if (curridx < rc.numgames) {
	       SDL_RenderCopy(sdlRenderer, shadow, &src_r, &shd_r);
	       //	 SDL_RenderCopy(ren, icon[curridx], &src_r, &dst_r);
	       SDL_RenderCopy(sdlRenderer, gridicon[ix][iy+Mstartrow], &src_r, &dst_r);
	    }
	 }  // for ix
      }  // for iy
	 
      // bottom line
      src_r.x=0; src_r.y=0; src_r.w=400; src_r.h=i*40;
      iy=rc.grid_h;
      for(ix=0; ix < rc.grid_w; ix++) {
	
	 dst_r.x=(ix * rc.icon_w) + rc.grid_x; dst_r.y=(iy * rc.icon_h)-(i*rc.img_h/10) + rc.grid_y; dst_r.w=rc.img_w; dst_r.h=i * (rc.img_h/10);
	 shd_r.x=(ix * rc.icon_w) + rc.grid_x + 8; shd_r.y=(iy * rc.icon_h)-(i*rc.img_h/10) + rc.grid_y + 8; shd_r.w=rc.img_w; shd_r.h=i * (rc.img_h/10);
	 
	 curridx=(iy * rc.grid_w) + ix + (Mstartrow * rc.grid_w);
	 
	 if (curridx < rc.numgames) {
	    SDL_RenderCopy(sdlRenderer, shadow, &src_r, &shd_r);
	    SDL_RenderCopy(sdlRenderer, gridicon[ix][iy+Mstartrow], &src_r, &dst_r);
	 }
      }
      power_button(); // grid_scroll_up()
      
//      slc=startidx+cursor_x+(startrow*rc.grid_w);
      slc=cursor_x+(startrow*rc.grid_w);

      grid_display_info( startidx + slc );

      TTF_SizeText(font, game[slc].stitle, &txtw, &txth);
      txtc=(((rc.icon_w * rc.grid_w) - txtw) / 2) + rc.grid_x;
      // perhaps we should load stitle when we load title?

//      grid_ttf_print(txtc, 21, game[slc].stitle);
      grid_ttf_print(txtc, rc.hdr_y, game[slc].stitle);
      
      SDL_RenderPresent(sdlRenderer);
      SDL_RenderClear(sdlRenderer);
      
// printf("DISPLAYING:4::::%s   %dx%d\n", rc.bgimage, bgdst_r.w, bgdst_r.h);
      
      SDL_RenderCopy(sdlRenderer, bgtexture, &bgsrc_r, &bgdst_r);

      SDL_Delay(20);  // Pause execution for 3000 milliseconds, for example
   }  // for i
      
}  // grid_scroll_up()


void grid_scroll_down() {
   // scroll grid down (if move up)
   SDL_Rect src_r, dst_r, shd_r;
   int ix, iy, curridx, i, Mstartrow, slc;
   int txtw, txth, txtc;
   
   Mstartrow=startrow-1;
   
   // 10 step
   //   for(i=0; i<10; i++) {
   for(i=10; i>0; i--) {
      // top line
      src_r.x=0; src_r.y=i*40; src_r.w=400; src_r.h=(10-i)*40;
      
      for(ix=0; ix < rc.grid_w; ix++) {
	 dst_r.x=(ix * rc.icon_w) + rc.grid_x; dst_r.y=(0 * rc.icon_h) + rc.grid_y; dst_r.w=rc.img_w; dst_r.h=(10-i) * (rc.img_h/10);
	 shd_r.x=(ix * rc.icon_w) + rc.grid_x + 8; shd_r.y=(0 * rc.icon_h) + rc.grid_y + 8; shd_r.w=rc.img_w; shd_r.h=(10-i) * (rc.img_h/10);
	 
	 curridx=(0 * rc.grid_w) + ix + (Mstartrow * rc.grid_w);
	 
	 if (curridx < rc.numgames) {
	    SDL_RenderCopy(sdlRenderer, shadow, &src_r, &shd_r);
	    SDL_RenderCopy(sdlRenderer, gridicon[ix][0+Mstartrow], &src_r, &dst_r);
	 }
      } // for ix

      
      // middle 
      src_r.x=0; src_r.y=0; src_r.w=400; src_r.h=400;
      
      for(iy=1; iy < (rc.grid_h); iy++) {
	 for(ix=0; ix < rc.grid_w; ix++) {
	    dst_r.x=(ix * rc.icon_w) + rc.grid_x; dst_r.y=(iy * rc.icon_h)-(i*rc.img_h/10) + rc.grid_y; dst_r.w=rc.img_w; dst_r.h=rc.img_h;
	    shd_r.x=(ix * rc.icon_w) + rc.grid_x + 8; shd_r.y=(iy * rc.icon_h)-(i*rc.img_h/10) + rc.grid_y + 8; shd_r.w=rc.img_w; shd_r.h=rc.img_h;
	    
	    curridx=(iy * rc.grid_w) + ix + (Mstartrow * rc.grid_w);
	    
	    if (curridx < rc.numgames) {
	       SDL_RenderCopy(sdlRenderer, shadow, &src_r, &shd_r);
	       //	 SDL_RenderCopy(ren, icon[curridx], &src_r, &dst_r);
	       SDL_RenderCopy(sdlRenderer, gridicon[ix][iy+Mstartrow], &src_r, &dst_r);
	    }
	 }  // for ix
      }  // for iy
	 
      // bottom line
      src_r.x=0; src_r.y=0; src_r.w=400; src_r.h=i*40;
      iy=rc.grid_h;
      for(ix=0; ix < rc.grid_w; ix++) {
	
//	 printf("strt  src:%d   dst:%d\n", (10-i)*40 ,i * (rc.img_h/10));
	 dst_r.x=(ix * rc.icon_w) + rc.grid_x; dst_r.y=(iy * rc.icon_h)-(i*rc.img_h/10) + rc.grid_y; dst_r.w=rc.img_w; dst_r.h=i * (rc.img_h/10);
	 shd_r.x=(ix * rc.icon_w) + rc.grid_x + 8; shd_r.y=(iy * rc.icon_h)-(i*rc.img_h/10) + rc.grid_y + 8; shd_r.w=rc.img_w; shd_r.h=i * (rc.img_h/10);
	 
	 curridx=(iy * rc.grid_w) + ix + (Mstartrow * rc.grid_w);
	 
	 if (curridx < rc.numgames) {
	    SDL_RenderCopy(sdlRenderer, shadow, &src_r, &shd_r);
	    SDL_RenderCopy(sdlRenderer, gridicon[ix][iy+Mstartrow], &src_r, &dst_r);
	 }
      }
      power_button(); // grid_scroll_down

//      slc=startidx+cursor_x+(startrow*rc.grid_w);
      slc=cursor_x+(startrow*rc.grid_w);

      grid_display_info( startidx + slc );
      
      TTF_SizeText(font, game[slc].stitle, &txtw, &txth);
      txtc=(((rc.icon_w * rc.grid_w) - txtw) / 2) + rc.grid_x;

      grid_ttf_print(txtc, rc.hdr_y, game[slc].stitle);
      
      
      SDL_RenderPresent(sdlRenderer);
      SDL_RenderClear(sdlRenderer);
      SDL_RenderCopy(sdlRenderer, bgtexture, &bgsrc_r, &bgdst_r);

      SDL_Delay(20);  // Pause execution for 3000 milliseconds, for example
   }  // for i
      
}  // grid_scroll_down()

void grid_clear() {
   int ix, iy;
   
   rc.numrows = rc.numgames / rc.grid_w;
   if((rc.numgames % rc.grid_w) != 0)
     rc.numrows++;
   
   for(iy=0;iy<rc.numrows;iy++) {
      for(ix=0; ix<rc.grid_w; ix++) {
	 SDL_DestroyTexture(gridicon[ix][iy]);
      } // for ix;
   } // for iy;
  	     
} // grid_clear()

void grid_show_games() {

   int grid_x=16, grid_y=80;
   SDL_Rect src_r, dst_r, shd_r;
   int ix, iy, curridx;
   int txtw, txth, txtc;
   char index_txt[10];
//   char *stitle;
   char stitle[80];
   
   src_r.x=0; src_r.y=0; src_r.w=400; src_r.h=400;

//   curridx = startidx;

   for(iy=0; iy < rc.grid_h; iy++) {
      for(ix=0; ix < rc.grid_w; ix++) {
	 dst_r.x=(ix * rc.icon_w) + rc.grid_x; dst_r.y=(iy * rc.icon_h) + rc.grid_y; dst_r.w=rc.img_w; dst_r.h=rc.img_h;
	 shd_r.x=(ix * rc.icon_w) + rc.grid_x + 8; shd_r.y=(iy * rc.icon_h) + rc.grid_y + 8; shd_r.w=rc.img_w; shd_r.h=rc.img_h;

	 curridx=(iy * rc.grid_w) + ix + (startrow * rc.grid_w);

	 if (curridx < rc.numgames) {
	    SDL_RenderCopy(sdlRenderer, shadow, &src_r, &shd_r);
	    SDL_RenderCopy(sdlRenderer, gridicon[ix][iy+startrow], &src_r, &dst_r);


	    // center icon text attempt
	    TTF_SizeText(font, game[curridx].stitle, &txtw, &txth);
//     printf("NAME=%s   txtw (raw) =%d\n", game[curridx].stitle, txtw);
	    txtw=txtw*75 / 100;
	    txtc=(rc.img_w - txtw ) /2;
//     printf("TXTW=%d (%d)  TXTC=%d\n", txtw, txtw*75/100, txtc);
//     printf("ICON_W=%d\n", rc.icon_w);
//     printf("IMG_W=%d\n\n", rc.img_w);
	    if(txtc<0) txtc=0;
	    
	    // changed [curridx+1] to [curridx]
	    if(game[curridx].showtext==1)
	      grid_ttf_print_scale(dst_r.x + txtc, dst_r.y+rc.icon_h-60, game[curridx].stitle, 70, 70, 0);
//	    grid_ttf_print_scale(dst_r.x + txtc, dst_r.y+rc.icon_h-50, game[curridx].stitle, 75, 75, 0);

//	    SDL_RenderCopy(ren, shadow, &src_r, &shd_r);
//	    SDL_RenderCopy(ren, gridicon[ix][iy+startrow], &src_r, &dst_r);
	 }
	 // Fix this:
	 //#ifdef DEBUG
	 //	 printf("%d:(%d) %s\n",curridx, ((i)*324)+24, game[curridx-1].title);
	 //#endif

//	 curridx++;
//	 if(curridx>(rc.numgames-1)) curridx=0;
      }
   }

#ifdef DEBUG
   printf("\n");
   printf("Index: %d\n",curridx);
   printf("Game: %s\n",game[curridx].title);
   printf("Exec: %s\n",game[curridx].exec);
#endif
   
   if(row==1) {
      // scale to resolution
      if (topcurs_idx == 1 )
	dst_r.x=rc.icon_gear_x;
      else
	dst_r.x=rc.icon_power_x;  
      
      dst_r.y=rc.topicons_y;
      dst_r.w=rc.topicons_w;  dst_r.h=rc.topicons_h;

      SDL_RenderCopy(sdlRenderer, cursor, &src_r, &dst_r);
   }
   if(row==2) {
      dst_r.x=rc.grid_x+(cursor_x*rc.icon_w); dst_r.y=rc.grid_y; dst_r.w=rc.img_w; dst_r.h=rc.img_h;
      SDL_RenderCopy(sdlRenderer, cursor, &src_r, &dst_r);
   }
   
   power_button(); // grid_show_games
   // need to center this
   curridx=cursor_x + ( rc.grid_w * startrow );

   grid_display_info( curridx + startidx );
   
   if(curridx<1)
     curridx=0;
//printf("curridx::%d  cursor_x::%d  startrow::%d\n", curridx, cursor_x, startrow);

   // the issue with centering is that the string is space-padded to the right
   // if we can remove that padding it should center better
   // search for C rtrim
   
//   trim(game[curridx].stitle, game[curridx].title);

   TTF_SizeText(font, game[curridx].stitle, &txtw, &txth);
   txtc=(((rc.icon_w * rc.grid_w) - txtw) / 2) + rc.grid_x;
   grid_ttf_print(txtc, rc.hdr_y, game[curridx].stitle);
   
   SDL_RenderPresent(sdlRenderer);
   SDL_RenderClear(sdlRenderer);

   SDL_RenderCopy(sdlRenderer, bgtexture, &bgsrc_r, &bgdst_r);

} // grid_show_games()

int grid_cursor_move(int curr_x, int curr_y, int dirct, int retv) {
   int retval, newx, newy;
   char index_txt[10];
   
   if(dirct==DIRCTN_UP) {
      newx=curr_x;
      newy=curr_y - 1;
   }
   
   if(dirct==DIRCTN_RIGHT) {
      newx=curr_x+1;
      newy=curr_y;
      // move this down
      if(newx == rc.grid_w && newy < rc.numrows-1) {
	 newx=0;
	 newy++;
      }
   }
   if(dirct==DIRCTN_DOWN) {
      newx=curr_x;
      newy=curr_y + 1;
   }

   if(dirct==DIRCTN_LEFT) {
      newx=curr_x-1;
      newy=curr_y;
      // move this down
      if(newx < 0) {
	 newx=rc.grid_w - 1;
	 newy--;
      }      
   }
   
   if(newy > rc.numrows-1) {
      newy = rc.numrows-1;
   }
   
   if((newy * rc.grid_w) + newx >= rc.numgames-1) {
      newx = (rc.numgames-1) % rc.grid_w;
      newy = (rc.numgames-1) / rc.grid_w;
   }
   
   if(newy < 0) {
      newy = 0;
      newx = curr_x;
   }
   
   if(retv==RTRN_X)
     retval=newx;

   if(retv==RTRN_Y)
     retval=newy;

//   printf("index: %d\n", newy * rc.grid_w + newx + 1 );
//   
//   prints debug info at the bottom of screen
//   grid_ttf_print(1180, 1380, menu[newy * rc.grid_w + newx + 1].name);
//   sprintf(index_txt, "  %d  ",newy * rc.grid_w + newx + 1); 
//   grid_ttf_print(0, 1380, index_txt);
   
   return retval;
} // grid_cursor_move()

void show_games() {
   int i, curridx, gi, xc;
   SDL_Rect src_r, dst_r, shd_r;
   char tstr[200];

   src_r.x=0; src_r.y=0; src_r.w=400; src_r.h=400;

   curridx=startidx;

   for(i=0;i<8;i++) {
      if(i==0) gi=curridx;

      dst_r.x=((i)*324)+24; dst_r.y=100; dst_r.w=300; dst_r.h=300;
      shd_r.x=((i)*324)+32; shd_r.y=108; shd_r.w=300; shd_r.h=300;

      SDL_RenderCopy(sdlRenderer, shadow, &src_r, &shd_r);
      SDL_RenderCopy(sdlRenderer, icon[curridx], &src_r, &dst_r);

#ifdef DEBUG
      printf("%d:(%d) %s\n",curridx, ((i)*324)+24, game[curridx-1].title);
#endif

      curridx++;
      if(curridx>(rc.numgames-1)) curridx=0;
   }

#ifdef DEBUG
   printf("\n");
   printf("Index: %d\n",gi);
   printf("Game: %s\n",game[gi].title);
   printf("Exec: %s\n",game[gi].exec);
#endif

   if(row==1) {
      // does this get run?  Scale res if so
      dst_r.x=24; dst_r.y=24; dst_r.w=48; dst_r.h=48;
      SDL_RenderCopy(sdlRenderer, cursor, &src_r, &dst_r);
   }
   if(row==2) {
      dst_r.x=24; dst_r.y=100; dst_r.w=300; dst_r.h=300;
      SDL_RenderCopy(sdlRenderer, cursor, &src_r, &dst_r);
   }
   
   power_button(); // show_games

#ifdef HAVE_LIBSDL2_TTF
//   sprintf(tstr, "(%d) %s",startidx ,game[startidx].title); 
   sprintf(tstr, "%s", game[startidx].title); 
//   sprintf(tstr, "(%d) %s",startidx ,game[startidx].icon); 
   SDL_Color bgcolor = { 0, 0, 0 };
   SDL_Color fgcolor = { 255, 255, 255 };
//   SDL_Surface * surface = TTF_RenderText_Solid( font, tstr, color);
   SDL_Surface * surface = TTF_RenderText_Blended( font, tstr, bgcolor);
   SDL_Texture * txtur = SDL_CreateTextureFromSurface(sdlRenderer, surface);

   xc=(rc.res_x - surface->w)/2;
   
   src_r.x=0; src_r.y=0; src_r.w=surface->w; src_r.h=surface->h;
   dst_r.x=xc; dst_r.y=rc.res_y-100; dst_r.w=src_r.w; dst_r.h=src_r.h;

   SDL_RenderCopy(sdlRenderer, txtur, &src_r, &dst_r);
   SDL_DestroyTexture(txtur);
   SDL_FreeSurface(surface);

   SDL_Surface * surface2 = TTF_RenderText_Blended( font, tstr, fgcolor);
   SDL_Texture * txtur2 = SDL_CreateTextureFromSurface(sdlRenderer, surface2);
   dst_r.x=xc-4; dst_r.y=rc.res_y-104; dst_r.w=src_r.w; dst_r.h=src_r.h;
   SDL_RenderCopy(sdlRenderer, txtur2, &src_r, &dst_r);
   SDL_DestroyTexture(txtur2);
   SDL_FreeSurface(surface2);
#endif

   SDL_RenderPresent(sdlRenderer);
   SDL_RenderClear(sdlRenderer);
   
   SDL_RenderCopy(sdlRenderer, bgtexture, &bgsrc_r, &bgdst_r);

} // show_games();

void click() {
   if(UIclick == 1 ) {
      int audio = SDL_QueueAudio(deviceId, wavbuffer, wavlength);
      SDL_PauseAudioDevice(deviceId, 0);
   }
}

void exec(char* exe) {
   int ret;
   printf("Executing: %s\n", exe);
   
   ret = system(exe);
}

int init_screen() {
   window = SDL_CreateWindow(
			     "GameLauncher",    // window title
			     0,                   // initial x position
			     0,                   // initial y position
			     rc.res_x,          // width, in pixels
			     rc.res_y,          // height, in pixels
			     SDL_WINDOW_FULLSCREEN_DESKTOP     // flags - see below
			    );
//   printf("%d:%d\n",rc.res_x, rc.res_y);
   
   // SDL_WINDOW_FULLSCREEN_DESKTOP
   // Check that the window was successfully created
   if (window == NULL) {
      // In the case that the window could not be made...
      printf("Could not create window: %s\n", SDL_GetError());
      return 1;
   }

//   ren = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
   ren = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED );

   if (ren == NULL) {
      SDL_DestroyWindow(window);
      SDL_Quit();
      return 1;
   }

   // Does this go here?
   SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");  // smoother
   SDL_RenderSetLogicalSize(sdlRenderer, rc.res_x, rc.res_y);
} // init_screen()


int load_game(char *filen, int index) {
   
   // load game config files

   FILE *fp;
   char line[256];
   char key[50], value[255];
   int m, loaded;
 
   loaded=1;

   if ((fp = fopen(filen,"rb"))==NULL) {
      printf("Error opening rc file %s",filen);
      exit(1);
   }

   while(!feof(fp)) {
      fgets(line,255,fp);
      if( line[0] != '#' ) {
	 m=strlen(line);
	 /* Strip LF */
	 line[m-1]=0;

	 hss_index(key,line,0,'=');
	 hss_index(value,line,1,'=');

	 if(strcmp(key, "TITLE")==0) {
	    strcpy(game[index].title, value);
	 }

	 if(strcmp(key, "ICON")==0) {
//	    strcpy(game[index].icon, value);
	    sprintf(game[index].icon, "/export/home/tsmolar/Devel/gamelauncher/src/%s", value);
	 }

	 if(strcmp(key, "EXECUTABLE")==0) {
	    strcpy(game[index].exec, value);
	 }

	 if(strcmp(key, "JOYSTICK")==0)
	   if (value[0] == 'F' || value[0] == 'f')
	     if(rc.reqjoy == 1) loaded=0;

	 if(strcmp(key, "KEYBOARD")==0)
	   if (value[0] == 'F' || value[0] == 'f')
	     if(rc.reqkb == 1) loaded=0;

	 if(strcmp(key, "MOUSE")==0)
	   if (value[0] == 'F' || value[0] == 'f')
	     if(rc.reqmouse == 1) loaded=0;
      } // if (line...)
   } // while(feof)
   fclose(fp);

   //   printf(" filen:%s  loaded:%d \n", filen, loaded);
    
   return(loaded);
} // load_game()

   
int read_game_files(const char *dir) {  // DELETE THIS WHEN DONE!
 
   // Reads game files from specified directory.
   
   DIR *folder;
   struct dirent *entry;
   int index = 0;
   int rv;
   char ext[12], fullpath[200];
   const char sep = '/';
   
   folder = opendir(dir);
   if(folder == NULL) {
      perror("Unable to read directory");
      return(1);
   }
   
   while( (entry=readdir(folder)) ) {
      hss_index(ext, entry->d_name, 1, '.');
      if(strcmp(ext, "game")==0) {
	 sprintf(fullpath,"%s%c%s",dir, sep, entry->d_name);
	 
#ifdef DEBUG
	 printf("File %3d: %s\n", index, fullpath);
#endif

	 rv=load_game(fullpath, index);

	 if(rv==1)
	   index++;

      }
   }
 
   closedir(folder);
   rc.numgames = index;
 
   return(0);
} // read_game_files(const char *dir);


void load_screen(int start) {

   SDL_Rect src_r, dst_r;
   int i;
   
   surf = IMG_Load(rc.bgimage);
   bgsrc_r.x=0; bgsrc_r.y=0; bgsrc_r.w=surf->w; bgsrc_r.h=surf->h;
   bgdst_r.x=0; bgdst_r.y=0; bgdst_r.w=usex; bgdst_r.h=usey;

   bgtexture = SDL_CreateTextureFromSurface(sdlRenderer, surf);

   // try setting screenshot background images
   // if we set this more than once, and we scale, then the
   // backgrounds get corrupted.   ssbg_set tries to prevent this
   if (ssbg_set == 0 ) {
      for(i=4; i<7; i++) {
	 src_r.x=imgbx[i].x;  src_r.y=imgbx[i].y;
	 src_r.w=imgbx[i].w;  src_r.h=imgbx[i].h;
	 dst_r.x=0;  dst_r.y=0;
	 dst_r.w=imgbx[i].w;  dst_r.h=imgbx[i].h;
	 SDL_BlitSurface(surf, &src_r, imgbx_bmp[i], &dst_r);
//	 printf("settingBG:[%d] (%d,%d):%d:%d \n", i, imgbx[i].x, imgbx[i].y, imgbx[i].w, imgbx[i].h);
      }
      ssbg_set=1;
   }
   
   SDL_FreeSurface(surf);
   
   // The window is open: could enter program loop here (see SDL_PollEvent())
   //
   SDL_RenderClear(sdlRenderer);
//   SDL_RenderCopy(sdlRenderer, bgtexture, NULL, NULL);
   SDL_RenderCopy(sdlRenderer, bgtexture, &bgsrc_r, &bgdst_r);
   // SDL_RenderClear(ren);
   // SDL_RenderCopy(ren, bgtexture, NULL, NULL);
   SDL_RenderPresent(sdlRenderer);
   //  SDL_DestroyTexture(texture);
   //
   //   read_game_files("games");
   //   read_game_files("/export/home/tsmolar/Devel/gamelauncher/src/games");

   load_ribbon(start-1);

   if(rc.mode == MODE_RIBBON) {
      show_games();
   }

   if(rc.mode == MODE_GRID) {
      grid_load_grid();
      grid_show_games();
   }

} // load_screen()

void set_defaults() {
   sprintf(rc.bgimage, "%s/%s", picsdir, bgpic);
   strcpy(DirImgDefault, "folder.png"); 
   strcpy(ExeImgDefault, "exe.png"); 
   strcpy(EmuImgDefault, "emulator.png"); 
   strcpy(UpDImgDefault, "upfolder.png"); 
} // set_defaults()

int scale_calc(int scalefrom, int res, int value) {
   int scalefac;
   
   if(scalefrom==0) {
      printf("scale_calc: first parm cannot be 0 or this will crash\n");
      printf("  HINT: rc.gridres might not be set!!\n\n");
   }
   
   scalefac=(res*1000) / scalefrom;
   return (value * scalefac) / 1000;
} // scale_calc()

void fake_defaults() {
   int scalefrom, scalefac;
   // just a set of fake defaults that are used until we
   // implement loading everything through the conf file
   rc.res_x=1920;
   rc.res_y=1080;
//   rc.res_x=2560;
//   rc.res_y=1440;
//   rc.mode=MODE_GRID;
   strcpy(rc.font, "/export/home/tsmolar/Devel/gamelauncher/src/images/mumbsb.ttf");
   strcpy(rc.bgimage, "/export/home/tsmolar/Devel/gamelauncher/src/images/ataribg.png");
   strcpy(rc.click, "click.wav");
   rc.reqjoy=0;
   rc.reqkb=1;
   rc.reqmouse=0;
   rc.poweroff=0;
   
   // scale
   scalefrom=scale_calc(1440, 720, rc.img_w);
} // end fake_defaults

void grid_bg_box(const char *title) {
   SDL_Rect src_r, dst_r;
   char picname[350],picnoext[346];   
   
   sprintf(picnoext,"%s%cpics%cdialog%ckbjsbox", basedir, mysep, mysep, mysep);
   AddPicExt(picname,picnoext);

   if(strncmp(img_cache[B_KBJOY].imgname, "kbjoy", 5) ==0 ) {
      src_r.x=0; src_r.y=0; src_r.w=img_cache[B_KBJOY].w; src_r.h=img_cache[B_KBJOY].h;
   } else {
      surf = IMG_Load(picname);
      src_r.x=0; src_r.y=0; src_r.w=surf->w; src_r.h=surf->h;
      
      SDL_DestroyTexture(img_cache[B_KBJOY].texture);
      img_cache[B_KBJOY].w = surf->w;
      img_cache[B_KBJOY].h = surf->h;
      img_cache[B_KBJOY].texture = SDL_CreateTextureFromSurface(sdlRenderer, surf);
      strcpy(img_cache[B_KBJOY].imgname, "kbjoy");
      SDL_FreeSurface(surf);
   }
   dst_r.w = scale_calc(1440, usey, img_cache[B_KBJOY].w); dst_r.h = scale_calc(1440, usey, img_cache[B_KBJOY].h);
   // center this in the grid area
   //   dst_r.x = (bgdst_r.w - dst_r.w) / 2, dst_r.y = (bgdst_r.h - dst_r.h) / 2;

   dst_r.x = ((rc.icon_w * rc.grid_w) - dst_r.w) / 2 + rc.grid_x - 16;
   dst_r.y = ((rc.icon_h * rc.grid_h) - dst_r.h) / 2 + rc.grid_y;
   
   SDL_RenderCopy(sdlRenderer, img_cache[B_KBJOY].texture, &src_r, &dst_r);
   SDL_RenderPresent(sdlRenderer);

   grid_ttf_print(dst_r.x+32, dst_r.y+32, title);
   sprintf(picname, "Emulator: %s",imenu.emulator);
   dst_r.x = ((rc.icon_w * rc.grid_w)) /2 + rc.grid_x;
   grid_ttf_print(dst_r.x, dst_r.y+dst_r.h-64, picname);
} // grid_bg_box()

void grid_keyboard_box() {
   
   SDL_Rect src_r, dst_r;
   char picname[350],picnoext[346];   
   
   if(imgbx[B_KEYBOARD].enabled==1) {
      sprintf(picnoext,"%s%c%s%s",picsdir,mysep,imgbx[B_KEYBOARD].pfx, imenu.emulator);
      AddPicExt(picname,picnoext);

      if(strcmp(picname, "null")!=0) {
	 
	 if(strcmp(img_cache[B_KEYBOARD].imgname, imenu.emulator)==0) {
	    src_r.x=0; src_r.y=0; src_r.w=img_cache[B_KEYBOARD].w; src_r.h=img_cache[B_KEYBOARD].h;
	 } else {
	    surf = IMG_Load(picname);
	    src_r.x=0; src_r.y=0; src_r.w=surf->w; src_r.h=surf->h;
	    
	    SDL_DestroyTexture(img_cache[B_KEYBOARD].texture);
	    img_cache[B_KEYBOARD].w = surf->w;
	    img_cache[B_KEYBOARD].h = surf->h;
	    img_cache[B_KEYBOARD].texture = SDL_CreateTextureFromSurface(sdlRenderer, surf);
	    strcpy(img_cache[B_KEYBOARD].imgname, imenu.emulator);
	    SDL_FreeSurface(surf);
	 }
	 if (bgdst_r.h > 959) {
	    dst_r.w = scale_calc(1440, usey, img_cache[B_KEYBOARD].w*2); 
	    dst_r.h = scale_calc(1440, usey, img_cache[B_KEYBOARD].h*2);
	 } else {
	    dst_r.w = scale_calc(1440, usey, img_cache[B_KEYBOARD].w); 
	    dst_r.h = scale_calc(1440, usey, img_cache[B_KEYBOARD].h);
	 }

	 // center this in the grid area
//	 dst_r.x = (bgdst_r.w - dst_r.w) / 2, dst_r.y = (bgdst_r.h - dst_r.h) / 2;
//	 dst_r.x = ((rc.icon_w * rc.grid_w) - dst_r.w) / 2 + rc.grid_x;
	 dst_r.x = rc.grid_x;
	 dst_r.y = ((rc.icon_h * rc.grid_h) - dst_r.h) / 2 + rc.grid_y;

	 printf("KB:%d,%d\n", dst_r.w, dst_r.h);
	 SDL_RenderCopy(sdlRenderer, img_cache[B_KEYBOARD].texture, &src_r, &dst_r);
	 
	 SDL_RenderPresent(sdlRenderer);
      }      
   } else {
      printf("No keyboard today\n");	
   }
} // grid_keyboard_box()

void grid_joystick_box() {
   
   SDL_Rect src_r, dst_r;
   char picname[350],picnoext[346];   
   
   imgbx[B_JOYST].enabled=1;

   strcpy(imgbx[B_JOYST].pfx,"js_");
   if(imgbx[B_JOYST].enabled==1) {
      sprintf(picnoext,"%s%c%s%s",picsdir,mysep,imgbx[B_JOYST].pfx, imenu.emulator);
      AddPicExt(picname,picnoext);

      if(strcmp(picname, "null")!=0) {

//	 if(strcmp(img_cache[B_JOYST].imgname, imenu.emulator)==0) {
//	    src_r.x=0; src_r.y=0; src_r.w=img_cache[B_JOYST].w; src_r.h=img_cache[B_JOYST].h;
//	 } else {
	    surf = IMG_Load(picname);
	    src_r.x=0; src_r.y=0; src_r.w=surf->w; src_r.h=surf->h;
	   
	    SDL_DestroyTexture(img_cache[B_JOYST].texture);
	    img_cache[B_JOYST].w = surf->w;
	    img_cache[B_JOYST].h = surf->h;
	    img_cache[B_JOYST].texture = SDL_CreateTextureFromSurface(sdlRenderer, surf);
	    strcpy(img_cache[B_JOYST].imgname, imenu.emulator);
	    SDL_FreeSurface(surf);
//	 }

	 if ((rc.icon_h * rc.grid_h) > img_cache[B_JOYST].h*2) {
//	   dst_r.w = img_cache[B_JOYST].w*2; dst_r.h = img_cache[B_JOYST].h*2;
	    dst_r.w = scale_calc(1440, usey, img_cache[B_JOYST].w*2); 
	    dst_r.h = scale_calc(1440, usey, img_cache[B_JOYST].h*2);
	 } else {
//	    dst_r.w = img_cache[B_JOYST].w; dst_r.h = img_cache[B_JOYST].h;
	    dst_r.w = scale_calc(1440, usey, img_cache[B_JOYST].w); 
	    dst_r.h = scale_calc(1440, usey, img_cache[B_JOYST].h);
	 }
	 
	 // center this in the grid area
//	 dst_r.x = ((rc.icon_w * rc.grid_w) - dst_r.w) / 2 + rc.grid_x;
	 dst_r.x = ((rc.icon_w * rc.grid_w)) + rc.grid_x - dst_r.w -64;
	 dst_r.y = ((rc.icon_h * rc.grid_h) - dst_r.h) / 2 + rc.grid_y;

	 SDL_RenderCopy(sdlRenderer, img_cache[B_JOYST].texture, &src_r, &dst_r);
	 
	 SDL_RenderPresent(sdlRenderer);
      }      
   } else {
      printf("No joystick today\n");	
   }
} // grid_joystick_box()


// taken from emufe.c
int grid_imgbox(int i, char *imgdir, char *iname) {

   char picname[350], picnoext[346];
   SDL_Rect src_r, dst_r;
   SDL_Texture *img_texture;

   if(imgbx[i].enabled==1) {

      sprintf(picnoext,"%s%c%s%s",imgdir,mysep,imgbx[i].pfx,iname);
      AddPicExt(picname,picnoext);
      if(strcmp(picname, "null")!=0) {
	 
	 if(strcmp(img_cache[i].imgname, iname)==0) {
	    src_r.x=0; src_r.y=0; src_r.w=img_cache[i].w; src_r.h=img_cache[i].h;
	 } else {
	    surf = IMG_Load(picname);
//	    printf("HMM load: %s\n", picname);
	    src_r.x=0; src_r.y=0; src_r.w=surf->w; src_r.h=surf->h;
	 
	    SDL_DestroyTexture(img_cache[i].texture);
	    img_cache[i].w = surf->w;
	    img_cache[i].h = surf->h;
	    img_cache[i].texture = SDL_CreateTextureFromSurface(sdlRenderer, surf);

	    strcpy(img_cache[i].imgname, iname);

	    SDL_FreeSurface(surf);
	 }
	 
	 dst_r.x = imgbx[i].x, dst_r.y = imgbx[i].y, dst_r.w = imgbx[i].w, dst_r.h = imgbx[i].h;
	 SDL_RenderCopy(sdlRenderer, img_cache[i].texture, &src_r, &dst_r);

//	 SDL_DestroyTexture(img_texture);

	 // note this assumes that these textures got loaded
   //   }

	 if(imgbx[i].masktype==1) { // BITMAP MASK
	    src_r.x=0; src_r.y=0; src_r.w=imgbx_mask[i]->w; src_r.h=imgbx_mask[i]->h;
	    img_texture = SDL_CreateTextureFromSurface(sdlRenderer, imgbx_mask[i]);

	    SDL_RenderCopy(sdlRenderer, img_texture, &src_r, &dst_r);
	    SDL_DestroyTexture(img_texture);
	    // printf("mask:%d (%d,%d,%d,%d)\n",i, dst_r.x, dst_r.y, dst_r.w, dst_r.h);
	 }
	 
	 // draw overlay
	 if(imgbx[i].ovpct>0) {
	    src_r.x=0; src_r.y=0; src_r.w=imgbx_ovl[i]->w; src_r.h=imgbx_ovl[i]->h;
	    img_texture = SDL_CreateTextureFromSurface(sdlRenderer, imgbx_ovl[i]);
	    SDL_SetTextureAlphaMod(img_texture, (25500/(10000/imgbx[i].ovpct)));
	    SDL_RenderCopy(sdlRenderer, img_texture, &src_r, &dst_r);
	    SDL_SetTextureAlphaMod(img_texture, 255 );
	    SDL_DestroyTexture(img_texture);
	 }
      } else {
	 // Don't have this image
	 // Remove from cache
	 strcpy(img_cache[i].imgname, "xxNULLxx");  

	 if(imgbx_bmp[i]) {
	    src_r.x=0;  src_r.y=0;
	    src_r.w=imgbx_bmp[i]->w;  src_r.h=imgbx_bmp[i]->h;
//	    dst_r.x=imgbx[i].x+usex;  dst_r.y=imgbx[i].y+usey;
	    dst_r.x=imgbx[i].x;  dst_r.y=imgbx[i].y;
	    dst_r.w=imgbx[i].w;  dst_r.h=imgbx[i].h;
	    img_texture = SDL_CreateTextureFromSurface(sdlRenderer, imgbx_bmp[i]);

	    SDL_RenderCopy(sdlRenderer, img_texture, &src_r, &dst_r);

	 }
	 
      }
      
//      SDL_RenderPresent(sdlRenderer);   
   }
   
} // grid_imgbox


void grid_display_info( int mindex ) {
   int i;

   for(i=4;i<7;i++) {
     grid_imgbox(i, picsdir, menu[mindex].rom);
   }
}

void grid_init() {
   /* This initializes the new grid more */
   int rv, flags;

   for(rv=0;rv<12;rv++)
     strcpy(img_cache[rv].imgname, "---");
   
   flags=IMG_INIT_JPG|IMG_INIT_PNG;
   rv=IMG_Init(flags);

   fake_defaults();
   set_defaults();
   
#ifdef HAVE_LIBSDL2_TTF
   if (TTF_Init() != 0) {
      SDL_Quit();
      return 1;
   }
#endif
   row=2;
   
   // Unlike gamelauncher, we don't change directories, we just
   // read menus
   //   chdir(rc.basedir);

   // Probably can remove these, but check other settings
   //SDL_Event event;         // move to game_loop
   SDL_Texture *texture;    // use TXscreen
   SDL_Rect src_r, dst_r;   // move to game loop
   // end this

   // audio
//   sprintf(rc.click, "%s%cclick.wav", picsdir, mysep);
   sprintf(rc.click, "%s%cpics%cclick1.wav", basedir, mysep, mysep);
//printf("load rc.click:%s\n", rc.click);
   SDL_LoadWAV(rc.click, &aSpec, &wavbuffer, &wavlength);
   deviceId = SDL_OpenAudioDevice(NULL, 0, &aSpec, NULL, 0);

#ifdef HAVE_LIBSDL2_TTF
   sprintf(rc.font, "%s%c%s", picsdir, mysep, rc.ttfont);
   font=TTF_OpenFont(rc.font, rc.gridfontsize);
#endif
   // check these
   havejoy=0;  // keep

//   rv = init_screen();  // don't call this but might need to set things from it
   startidx=1;
//   load_screen(startidx);
   startrow=0;
   // remove if necessary

   // +------------------------------------------------------+
   // |              M A P P I N G     I N F O               |
   // +------------------------------------------------------+
   // | SDL Type        | Old grid        | New emufe        |  
   // +------------------------------------------------------+
   // | SDL_Window      | window          | sdlWindow        |
   // | SDL_Texture     | texture         | TXscreen         |
   // | SDL_Renderer    | ren             | sdlRenderer      |
   // | SDL_PixelFormat | ???             | sdlpixfmt        |
   // | SDL_Surface     | n/a             | screen           |
   // +------------------------------------------------------+

} // grid_init()

int grid_menu_parm(char *var, char *value) {
   // This allows grid icon parameters to be changed in menus
   // This would allow different shapes for say carts, CD-roms, etc
   char key[50], tmpstr[60];
   hss_index(key, var,0,' ');
//   printf("gmp: %s = %s\n", key, value);
//   printf("usey=%d:  rc.gridres=%d\n", usey, rc.gridres);
   
   if(strncmp(key, "ROMDIR", 6)==0) {
      strcpy(imenu.altromdir, value);
   } // alternate romdir for collections
   
   if(strncmp(key, "GRIDRES", 7)==0) {
      // This is the vertical res the grid parameters
      // Are presented in, used to compute scaling
      rc.gridres=atoi(value);
   }
   
   if(strncmp(key, "GRIDHDR", 7)==0) {
      hss_index(tmpstr,value,0,'x');
//      rc.img_w = atoi(tmpstr);
      rc.hdr_x = scale_calc(rc.gridres, usey, atoi(tmpstr));
      hss_index(tmpstr,value,1,'x');
//      rc.img_h = atoi(tmpstr);
      rc.hdr_y = scale_calc(rc.gridres, usey, atoi(tmpstr));
   }

   if(strncmp(key, "GRIDIMG", 7)==0) {
      hss_index(tmpstr,value,0,'x');
      rc.img_w = scale_calc(rc.gridres, usey, atoi(tmpstr));
      hss_index(tmpstr,value,1,'x');
      rc.img_h = scale_calc(rc.gridres, usey, atoi(tmpstr));
   }
   
   if(strncmp(key, "GRIDICON", 8)==0) {
      hss_index(tmpstr,value,0,'x');
//      rc.icon_w = atoi(tmpstr);
      rc.icon_w = scale_calc(rc.gridres, usey, atoi(tmpstr));
      hss_index(tmpstr,value,1,'x');
//      rc.icon_h = atoi(tmpstr);
      rc.icon_h = scale_calc(rc.gridres, usey, atoi(tmpstr));
   }
   
   if(strncmp(key, "GRIDXY", 6)==0) {
      hss_index(tmpstr,value,0,'x');
//      rc.grid_x = atoi(tmpstr);
      rc.grid_x = scale_calc(rc.gridres, usey, atoi(tmpstr));
      hss_index(tmpstr,value,1,'x');
//      rc.grid_y = atoi(tmpstr);
      rc.grid_y = scale_calc(rc.gridres, usey, atoi(tmpstr));
   }
   
   if(strncmp(key, "GRIDWH", 6)==0) {
      hss_index(tmpstr,value,0,'x');
      rc.grid_w = atoi(tmpstr);
//      rc.grid_w = scale_calc(rc.gridres, usey, atoi(tmpstr));
      hss_index(tmpstr,value,1,'x');
      rc.grid_h = atoi(tmpstr);
//      rc.grid_h = scale_calc(rc.gridres, usey, atoi(tmpstr));
   }   
}

int grid_load_menu(char *filen) {
   // wrap the load_menu()
   int i, menuitems;
   char tmpfilename[128];
   
   // reset default emulator
   if(UseDfltEmu>0) {
      dflt.entry=-1;
      strcpy(dflt.name, "");
      strcpy(dflt.system, "");
   }
   
   // reset the grid parameters
   rc.grid_x=rc.g_grid_x;  rc.grid_y=rc.g_grid_y;
   rc.grid_w=rc.g_grid_w;  rc.grid_h=rc.g_grid_h;
   rc.img_w=rc.g_img_w;  rc.img_h=rc.g_img_h;
   rc.icon_w=rc.g_icon_w;  rc.icon_h=rc.g_icon_h;

   /*
   printf("++ g_grid_x:%d  grid_x:%d\n", rc.g_grid_x, rc.grid_x);
   printf("++ g_grid_y:%d  grid_y:%d\n", rc.g_grid_y, rc.grid_y);
   printf("++ g_grid_w:%d  grid_w:%d\n", rc.g_grid_w, rc.grid_w);
   printf("++ g_grid_h:%d  grid_h:%d\n", rc.g_grid_h, rc.grid_h);
   printf("++ g_img_w:%d   img_w:%d\n", rc.g_img_w, rc.img_w);
   printf("++ g_img_h:%d   img_h:%d\n", rc.g_img_h, rc.img_h);
   printf("++ g_icon_w:%d  icon_w:%d\n", rc.g_icon_w, rc.icon_w);
   printf("++ g_icon_h:%d  icon_h:%d\n", rc.g_icon_h, rc.icon_h);
   */
 
   menuitems=load_menu(filen);
   rc.numgames=0;
   
   for(i=0; i<menuitems+1; i++) {
      game[rc.numgames].showtext=1;
      // uncomment to print out the contents of the menu
      // printf("idx:%d   type:%c  name:%s    rom:%s\n",i, menu[i].type, menu[i].name, menu[i].rom);
      strcpy(game[rc.numgames].title, menu[i].name);
      trim(game[rc.numgames].stitle, game[rc.numgames].title);
      strcpy(game[rc.numgames].exec, menu[i].rom);
//      printf("GLM: %c  %d\n", menu[i].type, menu[i].type);
      switch (menu[i].type) {
       case 'd':
	 sprintf(tmpfilename, "%s/gr_%s.jpg", picsdir, game[rc.numgames].exec);
	 
	 if(fileio_file_exists(tmpfilename)) {
            strcpy(game[rc.numgames].icon, tmpfilename);
	    game[rc.numgames].showtext=0;
	 } else {
	    sprintf(game[rc.numgames].icon, "%s%c%s", picsdir, mysep, DirImgDefault);
	 }

//	 sprintf(game[rc.numgames].icon, "/export/home/tsmolar/Devel/gamelauncher/src/images/folder.png");
	 strcpy(game[rc.numgames].type, "DIR");
	 rc.numgames++;
	 break;
       case 'a':
	 if(UseDfltEmu>0) {
	    grid_DefaultEmu(dirname);
	    if(strcmp(menu[i].rom, dflt.name)==0)
   	       dflt.entry=rc.numgames;
	    printf("Match? %d\n", dflt.entry);
	 }
	 sprintf(game[rc.numgames].icon, "%s%c%s", picsdir, mysep, EmuImgDefault);
//	 sprintf(game[rc.numgames].icon, "/export/home/tsmolar/Devel/gamelauncher/src/images/emulator.png");
	 strcpy(game[rc.numgames].type, "EMU");
	 rc.numgames++;
	 break;
       case 'i':
	 sprintf(tmpfilename, "%s/gr_%s.jpg", picsdir, game[rc.numgames].exec);

	 if(fileio_file_exists(tmpfilename)) {
	    // should this be rc.numgames -1 ???
	    strcpy(game[rc.numgames].icon, tmpfilename);
	    game[rc.numgames].showtext=0;
#ifdef DEBUG
	    printf("GRIDXY: %d\n", rc.numgames);
	    printf("  Name: %s\n", game[rc.numgames].title);
	    printf("  Show: %d\n", game[rc.numgames].showtext);
	    printf("  Show: %s\n", game[rc.numgames].exec);
#endif
	 } else {
//	    sprintf(game[rc.numgames].icon, "/export/home/tsmolar/Devel/gamelauncher/src/images/exe.png");
	    sprintf(game[rc.numgames].icon, "%s%c%s", picsdir, mysep, ExeImgDefault);
//	    printf("XXXzzz: %s\n", game[rc.numgames].icon);
	 }
	 
	 strcpy(game[rc.numgames].type, "ROM");
	 rc.numgames++;
	 break;
       case 'u':
	 sprintf(game[rc.numgames].icon, "%s%c%s", picsdir, mysep, UpDImgDefault);

	 strcpy(game[rc.numgames].type, "UPD");
	 rc.numgames++;
	 break;
       case 'e':
//	 printf("end of menu\n");
	 break;
       case 'v':
	 grid_menu_parm(menu[i].name, menu[i].rom);
	 break;
      } // end switch
   }

   //printf("rc.numgames=%d\n",rc.numgames);
   
   rc.gridres=0;
   return menuitems;
} // grid_load_menu();

void grid_loop() {
   int rv, jsclear, tmx, tmy, i;
   int slc, index, mp, menuitems, nd_items;
   int quit = 0, noflush = 0, por = 0;
   int action=-1;
   char newmenu[500], usemenu[25], rcfile[30], letsel;
   SDL_Event event;

   // set the background image
   sprintf(rc.bgimage, "%s/%s", picsdir, bgpic);

   // deal with menu

   if(strlen(startdir)>0) {
      strcpy(dirname, startdir);
      sprintf(newmenu,"%s%c%sindex.menu",basedir,mysep, startdir);
   } else {
      if(strlen(dirname)==0) {
	 dirname[0]=0;
	 sprintf(newmenu,"%s%c%s",basedir,mysep,menuname);
      } else {
	 sprintf(newmenu,"%s%c%s%c%s",basedir,mysep,dirname,mysep,menuname);
      }
   }
//printf("GrId II:  newmenu:'%s'\n", newmenu);
#ifdef HAVE_LIBSDL2_TTF
   font=TTF_OpenFont(rc.font, rc.gridfontsize);
#endif

   slc=1; index=1; mp=0;

   menuitems=grid_load_menu(newmenu);
   load_screen(startidx);

   for(i=4;i<7;i++) 
     draw_imgbx(i);


   // when imenu.autosel is 1, don't skip the directory if it has
   // any 'i' items in the menu (causes crashes)
   nd_items=0;
   for(i=0;i<menuitems;i++) {
      if(menu[i].type=='i')
	nd_items++;
   }

#ifdef AMDGFX
//   SDL_RenderPresent(sdlRenderer);
   grid_show_games();
#endif

   while(quit == 0) {

      // detect if a joystick is connected during loop
      if(havejoy==0) {
	 if(SDL_NumJoysticks()>0) {
	    gGameController = SDL_JoystickOpen( 0 );
	    printf("\n\n\nJOYSTICK FOUND\n\n\n");
	    if( gGameController == NULL ) {
	       printf( "Warning: Unable to open game controller! SDL Error: %s\n", SDL_GetError() );
	    }
	    havejoy=1;
	 }
      }

      // delayed display
      if (cdclock == 1) {
	 endtime = time(NULL);
	 /* One Second Later */
	 if(endtime >= starttime) {
	    slc=startidx+cursor_x+(startrow*rc.grid_w);
	    if(menu[slc].type == 'i'  || menu[slc].type == 's') {
	       grid_display_info(slc);
	       por=1;
	    }
	    cdclock=0;  // will moving this out of the if fix the
	 }
      } // end cdclock == 1
      
      SDL_WaitEvent(&event);

      switch (event.type) {
	 
       case SDL_QUIT:
	 quit = 1;
	 break;
	 
       case SDL_KEYDOWN:
	 switch (event.key.keysym.sym) {
	  case SDLK_LEFT:
	    action=ACTION_LEFT;
	    break;

	  case SDLK_RIGHT:
	    action=ACTION_RIGHT;
	    break;

	  case SDLK_UP:
	    action=ACTION_UP;
	    break;

	  case SDLK_DOWN:
	    action=ACTION_DOWN;
	    break;

	  case SDLK_F1:
	    action=ACTION_F1;
	    break;

	  case SDLK_F2:
	    action=ACTION_F2;
	    break;

	  case SDLK_F3:
	    action=ACTION_CONFIG;
	    break;

	  case SDLK_F5:
	    load_ribbon(startidx);
	    break;

	  case SDLK_F10:
	    action=ACTION_F10;
	    break;

	  case SDLK_F11:
	    action=ACTION_F11;
	    break;

	  case SDLK_RETURN:
	    action=ACTION_SELECT;
	    break;

	  case SDLK_ESCAPE:
//	    quit = 1;
	    action=ACTION_ESCAPE;
	    break;

	  case SDLK_HOME:
	    action=ACTION_HOME;
	    break;
	    
	  case SDLK_END:
	    action=ACTION_END;
	    break;
	    
	  case SDLK_PAGEUP:
	    action=ACTION_PGUP;
	    break;
	    
	  case SDLK_PAGEDOWN:
	    action=ACTION_PGDN;
	    break;
	    
	 } // switch SDL_KEYDOWN
	 // Check for alphanumeric
         if (event.key.keysym.sym >= SDLK_0 && event.key.keysym.sym <= SDLK_9) {
	    action=ACTION_ALPHANUM;
	    letsel=event.key.keysym.sym;
	    printf("num char: %c\n", letsel);
	    // number
	 }
	 
         if (event.key.keysym.sym >= SDLK_a && event.key.keysym.sym <= SDLK_z) {
	    action=ACTION_ALPHANUM;
//	    action=ACTION_NULL;
	    letsel=event.key.keysym.sym-32;
	    printf("alpha char: %c\n", letsel);
	 }
	 
	 break;

	 // joysticks
       case SDL_JOYAXISMOTION:
	 if(event.jaxis.which == 0 ) {
	    if ( event.jaxis.axis == 0 || event.jaxis.axis == 3 ) { // X Axis
	       if( event.jaxis.value < -JOYSTICK_DEAD_ZONE ) {
		  // left
		  action=ACTION_LEFT;
		  SDL_Delay(100);
		  jsclear=1;
	       } else if ( event.jaxis.value > JOYSTICK_DEAD_ZONE ) {
		  // right
		  action=ACTION_RIGHT;
		  SDL_Delay(100);
		  jsclear=1;
	       } 
	    } // event.jaxis.axis

	    if ( event.jaxis.axis == 1 || event.jaxis.axis == 4 ) {
	       // up
	       if( event.jaxis.value < -JOYSTICK_DEAD_ZONE ) {
		  action=ACTION_UP;
		  SDL_Delay(100);
		  jsclear=1;
	       } else if ( event.jaxis.value > JOYSTICK_DEAD_ZONE ) {
		  action=ACTION_DOWN;
		  SDL_Delay(100);
                  jsclear=1;
	       }
	    } // event.jaxis.axis
	 }
	 if(jsclear==1) {
	    SDL_PumpEvents();
//	    SDL_FlushEvent(SDL_JOYAXISMOTION);
	    grid_flush_joy();
	    grid_flush_key();
	    jsclear=0;
	 }
	 break;
	 
	 // D-pad:
       case SDL_JOYHATMOTION:
	 if(event.jhat.which == 0 ) {
	    if(event.jhat.hat == 0 ) {
	       if(event.jhat.value == SDL_HAT_UP) {
		  action=ACTION_UP;
	       }

	       if(event.jhat.value == SDL_HAT_DOWN) {
		  action=ACTION_DOWN;
	       }

	       if(event.jhat.value == SDL_HAT_LEFT) {
		  action=ACTION_LEFT;
	       }

	       if(event.jhat.value == SDL_HAT_RIGHT) {
		  action=ACTION_RIGHT;
	       }
	    }
	 } // event.jhat.which
	 break;

	 // Joystick buttons
       case SDL_JOYBUTTONDOWN:
	 if( event.jbutton.which == 0 ) {
	    // PS4 (X)  XBOX (A) Button
	    if( event.jbutton.button == 0 ) {
	       if(event.jbutton.state == 1 ) {
		  action=ACTION_SELECT;
	       }  // event.jbutton.state == 1
	    } // event.jbutton.button
	    // PS4 (Triangle)  XBOX (Y) Button
	    if( event.jbutton.button == 4 ) {
	       if(event.jbutton.state == 1 ) {
		  action=ACTION_CONFIG;
	       }  // event.jbutton.state == 1
	    } // event.jbutton.button
	 } // event.jbutton.which
	 SDL_FlushEvent(SDL_JOYBUTTONDOWN);
	 break;
	 
      } // switch event.type()
      
      switch(action) {
       case ACTION_ESCAPE:
	 quit = 1;
	 break;
       case ACTION_LEFT:
	 if(row==1) {
	    if (topcurs_idx == 0) 
	      topcurs_idx = 1;
	    else
	      topcurs_idx = 0;
	    grid_show_games();
	    click();
	 }
	   
	 if(row==2) {
	    cdclock=1; starttime=time(NULL);
	    click();
	    tmx = grid_cursor_move(cursor_x, startrow, DIRCTN_LEFT, RTRN_X);
	    tmy = grid_cursor_move(cursor_x, startrow, DIRCTN_LEFT, RTRN_Y);
	    if(tmy < startrow)
	      grid_scroll_down();
	    cursor_x = tmx;
	    startrow = tmy;

	    grid_show_games();
	    imenu.no_launch=0;
	 }
	 break;
       case ACTION_RIGHT:
	 if(row==1) {
	    if (topcurs_idx == 0) 
	      topcurs_idx = 1;
	    else
	      topcurs_idx = 0;
	    click();
	    grid_show_games();
//	    power_button();
	 }
	 
	 if(row==2) {
	    cdclock=1; starttime=time(NULL);
	    click();
	    
	    tmx = grid_cursor_move(cursor_x, startrow, DIRCTN_RIGHT, RTRN_X);
	    tmy = grid_cursor_move(cursor_x, startrow, DIRCTN_RIGHT, RTRN_Y);		  

	    if(tmy > startrow)
	      grid_scroll_up(); 

	    cursor_x = tmx;
	    startrow = tmy;
	    
	    grid_show_games();

	    imenu.no_launch=0;
	 }
	 break;
       case ACTION_UP:
	 click();
	 if(startrow>0) {
	    cdclock=1; starttime=time(NULL);

	    tmx = grid_cursor_move(cursor_x, startrow, DIRCTN_UP, RTRN_X);
	    tmy = grid_cursor_move(cursor_x, startrow, DIRCTN_UP, RTRN_Y);
	    if(tmy < startrow)
	      grid_scroll_down(); 
	    cursor_x = tmx;
	    startrow = tmy;
	    imenu.no_launch=0;
	 } else
	   row=1;
	 grid_show_games();
	 break;

       case ACTION_DOWN:
	 click();
	 cdclock=1; starttime=time(NULL);
	 if(row==1)
	   startrow=0;
	 else {
	    tmx = grid_cursor_move(cursor_x, startrow, DIRCTN_DOWN, RTRN_X);
	    tmy = grid_cursor_move(cursor_x, startrow, DIRCTN_DOWN, RTRN_Y);
	    cursor_x = tmx;
	    if(tmy > startrow)
	      grid_scroll_up(); 
	    startrow = tmy;
	    imenu.no_launch=0;
	 }
	 row=2;
	 grid_show_games();
	 break;

       case ACTION_ALPHANUM:
	 // slc might be the issue here
	 i=find_entry(letsel,slc);
	 
//	 printf("  current slc = %d\n", slc);
//	 printf("  entry for: %c:%d\n", letsel, i);
//	 printf("  Row?  %d\n", (i-1) / rc.grid_w);
//	 printf("  Col?  %d\n", (i-1) % rc.grid_w);

	 if ( i>-1) {
	    click();
	    cursor_x = (i-1) % rc.grid_w;
	    startrow = (i-1) / rc.grid_w;
	    grid_show_games();
	    imenu.no_launch=0;
	    slc=i;
	 }
	 break;
	 
       case ACTION_PGDN:
	 if(row>1) {
	    click();
	    for(i=0;i<(rc.grid_h-1);i++) {
	       tmx = grid_cursor_move(cursor_x, startrow, DIRCTN_DOWN, RTRN_X);
	       tmy = grid_cursor_move(cursor_x, startrow, DIRCTN_DOWN, RTRN_Y);
	       cursor_x = tmx;
	       if(tmy > startrow)
		 grid_scroll_up();
	       startrow = tmy;
	    }
	    imenu.no_launch=0;
	    grid_show_games();
	 }
	 break;

       case ACTION_PGUP:
	 if(row>1) {
	    click();
	    for(i=0;i<(rc.grid_h-1);i++) {
	       tmx = grid_cursor_move(cursor_x, startrow, DIRCTN_UP, RTRN_X);
	       tmy = grid_cursor_move(cursor_x, startrow, DIRCTN_UP, RTRN_Y);
	       cursor_x = tmx;
	       if(tmy < startrow)
		 grid_scroll_down(); 
	       startrow = tmy;
	    }
	    imenu.no_launch=0;
	    grid_show_games();
	 }
	 break;

       case ACTION_CONFIG:
	 // TODO, only for emulator config, probably need to pass value
//	 cfg_emuconfig(imenu.emulator, imenu.profile);
	 if(menu[slc].type == 'a') {
	    cfg_emuconfig(menu[slc].rom);
	    cfg_loop();
	    grid_show_games();
	    grid_show_games();
	 }
	 break;
	 
       case ACTION_HOME:
	 click();
	 startrow=0;
	 grid_show_games();
	 break;
       
       case ACTION_END:
	 click();
	 for(i=0;i<rc.numrows;i++) {
	    tmx = grid_cursor_move(cursor_x, startrow, DIRCTN_DOWN, RTRN_X);
	    tmy = grid_cursor_move(cursor_x, startrow, DIRCTN_DOWN, RTRN_Y);
	    cursor_x = tmx;
	    startrow = tmy;	    
	 }
	 grid_show_games();
	 break;
	 
       case ACTION_F1:
	 grid_bg_box("Joystick and Keyboard Information");
	 grid_keyboard_box();
	 grid_joystick_box();
	 break;

       case ACTION_F2:
	 // change to desc_box
	 grid_joystick_box();
	 break;

       case ACTION_F10:
	 rc.mode=MODE_CLASSIC;
	 quit=1;
	 break;

       case ACTION_F11:
	 if(fullscr=='n') {
	    SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
	    fullscr='y';
	 } else {
	    SDL_SetWindowFullscreen(window ,0);
	    fullscr='n';
	 }
	 grid_show_games();
	 // restore screen
	 break;
	 
       case ACTION_SELECT:
	 if(row==1) {
	    // Allow for other icons than power
	    if (topcurs_idx == 0) {
	       quit = 1;
	       if(rc.poweroff > 0 )
		 quit = 2;
	    } else {
	       click();
	       cfg_maindialog();
//	       cfg_loadfile("/data/emulator/pics/dialog/mainconfig.cfg");
	       cfg_loop();
	       grid_show_games();
	       grid_show_games();
	    }
	 }

	 if(row==2) {
	    lastaction=menu[slc].type;
	    slc=startidx+cursor_x+(startrow*rc.grid_w);
//	    printf("slc=%d\n", slc);

	    if(menu[slc].type == 'd' || menu[slc].type == 'u') {
	       strcpy(usemenu, "index.menu");
	       if(menu[slc].type == 'u') {
//		  printf("-=-=-=-=-=-=-=- UpDiR -=-=-=-=-=-=-\n");
//		  printf("    dirname: %s\n", dirname);
//		  printf("    imenu.mode: %d\n", imenu.mode);
		  
		  if(strcmp(imenu.sysbase,dirname)==0 && imenu.mode>1) {
		     imenu.mode--;
		     if(imenu.mode==1) {
//			printf("XxYxXx-- etc%ctheme.rc\n",mysep);
			sprintf(imenu.rc, "etc%ctheme.rc", mysep);
			
			if(fileio_file_exists(imenu.rc)) {
			   load_rc(imenu.rc);
			   if(rc.mode != MODE_GRID) {
			      quit=1;
			   }
			} else {
			   printf("I can't load: %s\n", imenu.rc);
			}
		     }
		  } else {
		     updir(dirname);
		     
		     if(strcmp(imenu.sysbase,dirname)==0 && imenu.mode>1) {
			strcpy(usemenu,imenu.lastmenu);
		     }
		  }
		  sprintf(newmenu,"%s%c%s%s",basedir,mysep,dirname,usemenu);
		  
//		  printf("          usemenu:%s\n", usemenu);
//		  printf("          newmenu:%s\n", newmenu);
//		  printf("   imenu.lastmenu:%s\n", imenu.lastmenu);
//		  printf("    dirname: %s\n", dirname);
//		  printf("      quit:%d\n", quit);
//		  printf("        menuitems:%d\n", menuitems);
//		  printf("    imenu,autosel:%d\n", imenu.autosel);
//		  printf("-=-=-=-=-=-=-=- UpDiR -=-=-=-=-=-=-\n");

		  if(menuitems==2 && imenu.autosel==1 && nd_items==0) {
		     if(strlen(dirname)>0)
		       updir(dirname);
		     sprintf(newmenu,"%s%c%s%s",basedir,mysep,dirname,usemenu);
		     strcpy(menuname, "index.menu");
		  }

//printf("STARTDIR TRACKER: (grid) '%s'\n", startdir);
//printf("   ----------->: dirname '%s'\n", dirname);
	       }
	       if(menu[slc].type == 'd') {		  
		  sprintf(dirname,"%s%s%c",dirname,menu[slc].rom,mysep);
	       }
	       //		  sprintf(newmenu,"%s%c%sindex.menu",basedir,mysep,dirname);
	       sprintf(newmenu,"%s%c%s%s", basedir, mysep, dirname, usemenu);
	       
	       if(rc.mode == MODE_CLASSIC) {
		  printf("SwItChInG To MODE_CLASSIC\n");
		  strcpy(passdir, dirname); // experimental (broken?)
		  break;
	       }
	       
	       slc=1; index=1; mp=0;
	       
	       grid_clear();
	       
	       menuitems=grid_load_menu(newmenu);
	       
	       grid_load_grid();
	       
	       startrow=0;
	       cursor_x=0;
	       
	       if(UseDfltEmu > 0) {
		  if(dflt.entry>-1) {
		     // is it enough to just highlight the default 
		     // or should we automatically select?
		     printf("Default Emulator Enabled, Automatically selected:%d\n", dflt.entry);
		     if(lastaction=='d') {
			slc=dflt.entry+1;
			cursor_x=dflt.entry % rc.grid_w;
			startrow=dflt.entry / rc.grid_w;
			simulate_keypress(KEY_ENTER << 8);
		     }
		  }
	       }
	       grid_show_games();
	    }

	    if(menu[slc].type == 'i') {
	       if(imenu.mode>0) {
		  // Here's where we can set the collection rom base?
		  // example, dirname = Coleco/Colecovision/roms
		  // altdirname = basedir + new rom
		  if(strcmp(imenu.altromdir, "NULL")==0) {
		     sprintf(imenu.game,"%s%s",dirname,menu[slc].rom);
		  } else {
		     sprintf(imenu.game, "%s%s%c%s", imenu.sysbase, imenu.altromdir, mysep, menu[slc].rom);
		  }
 		  // launch code goes here
		  if(imenu.no_launch!=1) {
		     //  Looks like this works for shutting down and restarting gfx mode
		     // had to move this post process_cmd:
		     module_exec();
		     jsclear=1;
		     grid_flush_joy();
		     grid_flush_key();
		     grid_flush_joy();
		     grid_flush_key();
		     
		     // let's try this:
			
		     SDL_DestroyTexture(bgtexture);
		     load_screen(1);
		     
		     imenu.no_launch=1;
		  }			
	       } else {
		  printf("%s%s\n",dirname,menu[slc].rom);
		  break;
	       }
	       // }
	    } // if(menu[slc].type == 'i')
	    
	    if(menu[slc].type == 'a') {
	       // This is when an emulator is selected
	       // 
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
//		  printf("imenu.system: %s\n",imenu.system);
//		  printf("imenu.emulator: %s\n",imenu.emulator);
//		  printf("imenu.profile: %d\n",imenu.profile);
	
		  imenu.mode++;
		  
		  // Expand this!!!
		  // sprintf(imenu.menu,"%s%s.menu",dirname,roms[slc]);
		  // 
		  // per emulator rc file load happens here
		  sprintf(imenu.rc,"%s%c%setc%c%s.rc",basedir,mysep,dirname,mysep,imenu.emulator);
		  
		  // New for 2019, RC files not needed for simple console systems
		  // We might want to always run set_generic_rc and overlay with rc file
		  // 
		  imenu.systype=SYS_GENERIC;  // set to default
		  if(fileio_file_exists(imenu.rc)) {
		     load_rc(imenu.rc);
		  } else {
		     LOG(3,("Warning, no .rc file, using default settings\n"));
		     set_generic_rc();
		     //              sprintf(imenu.kbname,"%s",imenu.emulator);
		  }
		  // load keyboard picture here

		  sprintf(imenu.menu,"%s%c%s%s",basedir,mysep,dirname,menuname);
		  // HERE IS THE BUG
		  //              sprintf(imenu.lastmenu,"%s.menu",menu[slc].rom);
		  strcpy(imenu.lastmenu,menuname);

		  grid_clear();

		  menuitems=grid_load_menu(imenu.menu);
		  index=1;slc=1;

		  grid_load_grid();

		  startrow=0; cursor_x=0;
		  grid_show_games();

		  // reenable for auto-select (need KEY_ENTER defined)
		  if(menuitems==2 && imenu.autosel==1) {
		     // printf("PRT: ok\n");
		     slc=2;
		     cursor_x=1;
		     simulate_keypress(KEY_ENTER << 8);
		     //	noflush=1;
		     //			simulate_keypress(SDLK_RETURN << 8);
		  }
		  
	       }
	       if(imenu.mode==0) {
//		  printf("%s%s\n",dirname,menu[slc].rom);
		  break;
	       }
	    }
	 }
	    
	 if(noflush==0)
	   SDL_FlushEvent(SDL_KEYDOWN);
	 else
	   noflush=0;
	 // Note: Need to clear keyboard buffer
	 break;
	 
      } // switch(action)
      action=ACTION_NULL;
   }
   
#ifdef HAVE_LIBSDL2_TTF
   TTF_CloseFont(font);
#endif

} // grid_loop()
