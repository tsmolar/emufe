// This will replace setup.c eventually
#include <dirent.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <SDL.h>
#include <SDL_image.h>

#include<stdio.h>

#include "emufe.h"
#include "rcfile.h"
#include "modules.h"
extern SDL_Window *sdlWindow;
extern SDL_Renderer *sdlRenderer;
extern int havejoy, UIclick, UseDfltEmu;
extern char mysep, fullscr;
extern prop_t rc;

SDL_Surface *csurf;

typedef struct cfg_cache_t {
   char imgname[30];
   int w, h;
   SDL_Texture *texture;
} cfg_cache_t;


typedef struct cfg_button_t {
   char buttonname[30];
   char var[16], strval[30];
   int x,y,w,h,type;
   int ival, changed;
//   char sval[60];
} cfg_button_t;

cfg_cache_t cfg_cache[12];
cfg_button_t cfg_button[24];
SDL_Rect cfgw;
char g_emubin[30], g_emudir[40], g_emunamebin[24], g_emuname[20];
char g_system[16], g_shremu[20];
int ButtonMax, numchanges;

#define CFG_BUTTON 1
#define CFG_RADIO 2
#define CFG_CHECK 3
#define CFG_SELECT 4
#define CFG_XSELECT 104
#define CFG_END 255

#define D_UP 1
#define D_DOWN 2
#define D_LEFT 3
#define D_RIGHT 4

int cfg_inbutton(int x, int y) {

   /* Detects if X,Y region is inside a button */
   int t, rv=-1;
   
   for(t=0;t<ButtonMax;t++) {
      if (x>=(cfgw.x + cfg_button[t].x) && x<=(cfgw.x + cfg_button[t].x+cfg_button[t].w)
	  && y>=(cfgw.y + cfg_button[t].y) && y<=(cfgw.y + cfg_button[t].y+cfg_button[t].h)) {
	 rv=t;
	 break;
      }      
   }
   return(rv);
} // cfg_inbutton()

int cfg_search_button(int t, int drctn) {
   
   /* Search for button in specified direction
    * This is used for directional navigating menus */
   int sx, sy, x, y;
   int rv=t, rw;

   // This is kind of a hack.  If we create more dialogs with the SELECT
   // type, we may need to address this
   if( cfg_button[t].type == CFG_SELECT )
      sx=cfg_button[t].x + cfgw.x + ((cfg_button[t].w / 8)*7);
   else
     sx=cfg_button[t].x + cfgw.x + (cfg_button[t].w / 2);
   sy=cfg_button[t].y + cfgw.y + (cfg_button[t].h / 2);
   
//   printf("square: %d x %d, %d, %d\n", cfgw.x, cfgw.y, cfgw.w, cfgw.h);
//   printf("button: %d x %d\n", cfg_button[t].x,cfg_button[t].y);
//   printf("max y: %d\n", sy+cfgw.h);
//   printf("sx/sy: %d x %d\n", sx, sy);
   
   if(drctn == D_DOWN) {
      for(y=sy+cfg_button[t].h; y<(cfgw.y+cfgw.h)*1.5; y=y+48){
//	 printf("checking coord:  %d,%d\n",sx, y);
	 for(x=0; x<192; x=x+32) {
	    rw=cfg_inbutton(sx+x,y);
	    if(rw>-1) {
	       rv=rw;
	       break;
	    }
	    rw=cfg_inbutton(sx-x,y);
	    if(rw>-1) {
	       rv=rw;
	       break;
	    }
	 }
	 if(rv!=t)
	   break;
      }      
   }
   
   if(drctn == D_UP) {
      for(y=sy-cfg_button[t].h; y>cfgw.y; y=y-48){
//	 printf("up checking coord:  %d,%d\n",sx, y);
	 for(x=0; x<192; x=x+32) {
	    rw=cfg_inbutton(sx+x,y);
	    if(rw>-1) {
	       rv=rw;
	       break;
	    }	 
	    rw=cfg_inbutton(sx-x,y);
	    if(rw>-1) {
	       rv=rw;
	       break;
	    }	 
	 }
	 if(rv!=t)
	   break;
      }
   }

   if(drctn == D_LEFT) {
      for(x=sx-cfg_button[t].w; x>cfgw.x; x=x-48){
//	 printf("checking coord:  %d,%d\n",sx, y);
	 for(y=0; y<192; y=y+32) {
	    rw=cfg_inbutton(x,sy+y);
	    if(rw>-1) {
	       rv=rw;
	       break;
	    }	 
	    rw=cfg_inbutton(x,sy-y);
	    if(rw>-1) {
	       rv=rw;
	       break;
	    }	 
	 }
	 if(rv!=t)
	   break;
      }      
   }

   if(drctn == D_RIGHT) {
      for(x=sx+cfg_button[t].w; x<(cfgw.x+cfgw.w)*1.5; x=x+48) {
	 for(y=0; y<192; y=y+32) {
	    rw=cfg_inbutton(x,sy-y);
	    if(rw>-1) {
	       rv=rw;
	       break;
	    }
	    rw=cfg_inbutton(x,sy+y);
	    if(rw>-1) {
	       rv=rw;
	       break;
	    }	 
	 }
	 if(rv!=t)
	   break;
      }
   }
   
   return(rv);
} // cfg_search_button()

int cfg_getvalueidx(const char *vname) {
   int rv=-1, t;
   for(t=0;t<ButtonMax;t++) {
      if(strcmp(cfg_button[t].var, vname)==0) {
	 rv=t;
	 break;
      }
   }
   return(rv);
} // cfg_getvalueidx()

int cfg_getradioidx(const char *vstr, int nume) {
   // vstr is a comma separated list
   int t, z, rv=-1;
   char emnt[10];
   for(t=0; t<nume; t++) {
      hss_index(emnt, vstr, t, ',');
      z=cfg_getvalueidx(emnt);
      if(z>-1)
	if(cfg_button[z].ival>0 && cfg_button[z].ival<255)
	  rv=t;
   }
   return(rv);
} // cfg_getradioidx()

char cfg_getesetting(const char *vstr, char out0, char out1) {
   // This will reduce the amount of code in cfg_save() by mapping
   // 0 and 1 to actual values required
   char rv=0;
   int z;
   
   z=cfg_getvalueidx(vstr);
   if(z>-1)
     if(cfg_button[z].changed==1) {
	if(cfg_button[z].ival==0) {
	   rv=out0;
	} else {
	   rv=out1;
	}
     }
   
   return(rv);
}

void cfg_save() {
   FILE *fp, *fp2;
   char emucd[160], emucd2[160], line[256], lkey[20], st;
   int z;

   if(numchanges > 0) {
      sprintf(emucd,"%s%cuser_config%ccfg1%cemucd.env",basedir, mysep, mysep, mysep);
      if(!file_exists(emucd)) {  
	 sprintf(emucd,"%s%cetc%cemucd.env",basedir, mysep, mysep);
      }
      sprintf(emucd2, "cp %s /tmp/emucd.tmp", emucd);
      //   printf("cmd:%s\n", emucd2);
      system(emucd2);
      // end copy

      sprintf(emucd, "/tmp/emucd.tmp");
      sprintf(emucd2,"%s%cuser_config%ccfg1%cemucd.env",basedir, mysep, mysep, mysep);
   
      //   printf(" Infile:%s\n", emucd);
      //   printf("Outfile:%s\n", emucd2);
   
      if ((fp = fopen(emucd, "rb"))==NULL) {
	 printf("Error! Unable to open %s\n", emucd);
      } else {
	 fp2 = fopen(emucd2, "wb");
	 while(!feof(fp)) {
	    fgets(line,255,fp);
	    if(feof(fp)) break;
	    if( line[0] != '#' ){
	       hss_index(lkey, line, 0, '=');
	       
	       // UI Fullscreen
	       if((strncmp(lkey, "EMUFEfull", 9)) == 0) {
		  st=cfg_getesetting("UIFULLSCREEN", 'n', 'y');
		  if(st > 0)
		    sprintf(line, "EMUFEfull=%c\n", st);
	       }
	       
	       // UI Joystick
	       if((strncmp(lkey, "EMUFEjoy", 8)) == 0) {
		  st=cfg_getesetting("UIJOY", 'n', 'y');
		  if(st > 0)
		    sprintf(line, "EMUFEjoy=%c\n", st);
	       }
	       
	       // UI audio CLICKs
	       if((strncmp(lkey, "EMUFEclick", 10)) == 0) {
		  st=cfg_getesetting("UICLICK", 'n', 'y');
		  if(st > 0)
		    sprintf(line, "EMUFEclick=%c\n", st);
	       }
	       
	       // Add UIDFLTEMU
	       if((strncmp(lkey, "EMUFEusedflt", 12)) == 0) {
		  st=cfg_getesetting("UIDFLTEMU", 'n', 'y');
		  if(st > 0)
		    sprintf(line, "EMUFEusedflt=%c\n", st);
	       }
	       
	       // Fullscreen
	       if((strncmp(lkey, "FULLSCREEN", 10)) == 0) {
		  st=cfg_getesetting("FULLSCREEN", 'N', 'Y');
		  if(st > 0)
		    sprintf(line, "FULLSCREEN=%c\n", st);
	       }
	       
	       // Sound
	       if((strncmp(lkey, "SOUND", 5)) == 0) {
		  st=cfg_getesetting("SOUND", 'N', 'Y');
		  if(st > 0)
		    sprintf(line, "SOUND=%c\n", st);
	       }
	       
	       // joystick
	       if((strncmp(lkey, "JOYSTICK", 8)) == 0) {
		  st=cfg_getesetting("JOYSTICK", 'N', 'A');
		  if(st > 0)
		    sprintf(line, "JOYSTICK=%c\n", st);
	       }
	    
	       // fast cpu
	       if((strncmp(lkey, "FASTCPU", 7)) == 0) {
		  st=cfg_getesetting("FASTCPU", 'N', 'Y');
		  if(st > 0)
		    sprintf(line, "FASTCPU=%c\n", st);
	       }
	       
	       // MIDI
	       if((strncmp(lkey, "MIDI", 4)) == 0) {
		  st=cfg_getesetting("MIDI", 'N', 'Y');
		  if(st > 0)
		    sprintf(line, "MIDI=%c\n", st);
	       }
	       
	       // Scanlines
	       if((strncmp(lkey, "SCANLINE", 4)) == 0) {
		  z=cfg_getradioidx("NOCRT,BASIC,ADVANCED,CURVATURE", 4);
		  if(z>-1)
		    sprintf(line, "SCANLINE=%d\n", z);
	       }
	       
	       // Change EMU BIN
	       if((strncmp(lkey, g_emunamebin, strlen(g_emunamebin)))==0) {
		  z=cfg_getradioidx("S1,S2,S3,S4,S5", 5);
		  if(z>-1)
		    sprintf(line, "%s=%s\n", g_emunamebin, cfg_button[z+2].strval);
	       }
	    }
	    fprintf(fp2, "%s", line);
	 }
	 fclose(fp);
	 fclose(fp2);

         // save
	 z=cfg_getvalueidx("USEDEFAULT");	      
	 if(z>-1) {
	    if(cfg_button[z].changed==1) {
	       sprintf(emucd,"%s%cuser_config%ccfg1%ccfg%c%s.emu",basedir, mysep, mysep, mysep, mysep, g_system);
	       printf("Default file: %s\n", emucd);
	       if(cfg_button[z].ival==1) {
		  fp2 = fopen(emucd, "wb");
		  fprintf(fp2, "emulator|%s\n", g_emuname);
		  fclose(fp2);
	       } else {
		  sprintf(emucd2, "rm %s", emucd);
//		  printf("%s\n", emucd2);
		  system(emucd2);
	       }
	    }
	 }  // if (z>-1)
      }
   } else
     printf("No Changes To Save!\n");
   
} // cfg_save()

void cfg_apply() {
   int z;
   
   for(z=0;z<ButtonMax;z++) {
      if(strncmp(cfg_button[z].var, "UIFULLSCREEN", 12)==0) {
	 if(cfg_button[z].ival == 1)
	   fullscr='y';
	 else
	   fullscr='n';
      }
      
      if(strncmp(cfg_button[z].var, "UICLICK", 7)==0)
	UIclick=cfg_button[z].ival;
      
      if(strncmp(cfg_button[z].var, "UIJOY", 5)==0)
	joy_enable=cfg_button[z].ival;
      
      if(strncmp(cfg_button[z].var, "UIDFLTEMU", 9)==0)
	UseDfltEmu=cfg_button[z].ival;
   }   
   printf("Settings Applied!\n");
   
} // cfg_apply()


void cfg_setvalue(int t) {
   int z;
   if(cfg_button[t].type==CFG_CHECK) {
      if(cfg_button[t].ival==0)
	cfg_button[t].ival=1;
      else
	cfg_button[t].ival=0;
      cfg_button[t].changed=1;  // changed
      numchanges++;
   }
   
   if(cfg_button[t].type==CFG_RADIO) {
      for(z=0;z<ButtonMax;z++) {
	 if(cfg_button[z].type==CFG_RADIO) // clear all radio buttons
	   cfg_button[z].ival=0;
      }
      cfg_button[t].ival=1;  // set just the selected button
      cfg_refreshall();
      cfg_button[t].changed=1;  // changed
      numchanges++;
   }
   
   if(cfg_button[t].type==CFG_SELECT) {
      for(z=0;z<ButtonMax;z++) {
	 if(cfg_button[z].type==CFG_SELECT && cfg_button[z].ival<255) // clear all radio buttons
	   cfg_button[z].ival=0;
      }
      cfg_button[t].ival=1;  // set just the selected button
      cfg_refreshall();
      cfg_button[t].changed=1;  // changed
      numchanges++;
   }
   
   if(cfg_button[t].type==CFG_BUTTON) {
      if(strncmp(cfg_button[t].var, "APPLY", 5)==0) {
	 cfg_apply();
      }

      if(strncmp(cfg_button[t].var, "SAVE", 4)==0) {
	 cfg_apply();
	 cfg_save();
      }
   }
	
} // cfg_setvalue()


void cfg_unhighlight(int t) {
   SDL_Rect src_r, dst_r;
   
   dst_r.x = scale_calc(2160, usey, cfg_button[t].x) + cfgw.x; 
   dst_r.y = scale_calc(2160, usey, cfg_button[t].y) + cfgw.y;

   if( cfg_button[t].type == CFG_RADIO || cfg_button[t].type == CFG_CHECK || cfg_button[t].type == CFG_BUTTON || cfg_button[t].type == CFG_SELECT ) {
      switch (cfg_button[t].type) {
       case CFG_BUTTON:
	 if ( strcmp(cfg_button[t].var,"EXIT") ==0 ) {
	   src_r.x=568; src_r.y=0; src_r.w=71; src_r.h=71;
	 }
	 if ( strcmp(cfg_button[t].var,"APPLY") ==0 ) {
	   src_r.x=0; src_r.y=71; src_r.w=351; src_r.h=141;
	 }
	 if ( strcmp(cfg_button[t].var,"SAVE") ==0 ) {
	   src_r.x=351; src_r.y=71; src_r.w=351; src_r.h=141;
	 }
	 break;
	 
       case CFG_RADIO:
	 src_r.x=426+(cfg_button[t].ival*71); src_r.y=0; src_r.w=71; src_r.h=71;
//	 printf("chg_radio: %s  int:%d   val:%d\n", cfg_button[t].var, 426+(cfg_button[t].ival*71), cfg_button[t].ival*71);
	 break;
	 
       case CFG_CHECK:
	 src_r.x=142+(cfg_button[t].ival*71); src_r.y=0; src_r.w=71; src_r.h=71;
//	 src_r.x=213; src_r.y=0; src_r.w=71; src_r.h=71;
	 break;
	 
       case CFG_SELECT:
	 if(cfg_button[t].ival == 1) {
	   src_r.x=517; src_r.y=22; src_r.w=31; src_r.h=26;
	 } else {
	   src_r.x=639; src_r.y=0; src_r.w=63; src_r.h=71;
	 }
	 break;
      }
      
      dst_r.w = scale_calc(2160, usey, cfg_button[t].w);
      dst_r.h = scale_calc(2160, usey, cfg_button[t].h);

      SDL_RenderCopy(sdlRenderer, cfg_cache[B_MBUTTON].texture, &src_r, &dst_r);

      if( cfg_button[t].type == CFG_SELECT )
	grid_ttf_print(dst_r.x+16, dst_r.y-4, cfg_button[t].strval);
      
      SDL_RenderPresent(sdlRenderer);
   }
} // cfg_unhighlight()


void cfg_highlight(int t) {
   SDL_Rect src_r, dst_r;
   
   dst_r.x = scale_calc(2160, usey, cfg_button[t].x) + cfgw.x; 
   dst_r.y = scale_calc(2160, usey, cfg_button[t].y) + cfgw.y;
   
   if( cfg_button[t].type == CFG_RADIO || cfg_button[t].type == CFG_CHECK || cfg_button[t].type == CFG_BUTTON || cfg_button[t].type == CFG_SELECT) {
      
      switch (cfg_button[t].type) {
       case CFG_BUTTON:
	 if ( strcmp(cfg_button[t].var,"EXIT") ==0 ) {   
	   src_r.x=0; src_r.y=0; src_r.w=71; src_r.h=71;
	 }
	 if ( strcmp(cfg_button[t].var,"APPLY") ==0 ) {
	    src_r.x=0; src_r.y=212; src_r.w=351; src_r.h=141;
	 }
	 if ( strcmp(cfg_button[t].var,"SAVE") ==0 ) {
	    src_r.x=351; src_r.y=212; src_r.w=351; src_r.h=141;
	 }
	 break;
	 
       case CFG_RADIO:
	 src_r.x=284+(cfg_button[t].ival*71); src_r.y=0; src_r.w=71; src_r.h=71;
	 break;

       case CFG_CHECK:
	 src_r.x=0+(cfg_button[t].ival*71); src_r.y=0; src_r.w=71; src_r.h=71;
	 break;

       case CFG_SELECT:
	 src_r.x=4; src_r.y=4; src_r.w=63; src_r.h=63;
	 break;
      }
      
      dst_r.w = scale_calc(2160, usey, cfg_button[t].w);
      dst_r.h = scale_calc(2160, usey, cfg_button[t].h);
      SDL_RenderCopy(sdlRenderer, cfg_cache[B_MBUTTON].texture, &src_r, &dst_r);

      if( cfg_button[t].type == CFG_SELECT )
	grid_ttf_print(dst_r.x+16, dst_r.y-4, cfg_button[t].strval);
      
      SDL_RenderPresent(sdlRenderer);
   }
} // cfg_highlight()

void cfg_refreshall() {
   // display current settings
   int t;

   for(t=0;t<ButtonMax;t++)
      cfg_unhighlight(t);   

} // cfg_refreshall()


void cfg_presetival(char *vname, int v) {
   int t;
   for(t=0;t<ButtonMax;t++) {
     if(strcmp(cfg_button[t].var, vname)==0)
	cfg_button[t].ival=v;
   }
}  //cfg_setival()

void cfg_emuscandir(const char *emudir, const char *emubin) {
   struct dirent **namelist;
   FILE *fp;
   char fdir[200], sdir[16], fspec[15];
   int n, t;
   
   sprintf(fdir, "%s%s%cfilespec", basedir, emudir, mysep);
   
//   printf("DIR:%s\n", fdir);
   if(file_exists(fdir)) {
      fp = fopen(fdir,"rb");
      fgets(sdir,36,fp);
#ifdef WIN32
      // remove ^M linefeeds
      for(n=0;n<strlen(sdir);n++) {
	 if(sdir[n]==13)
	   sdir[n]=0;
      }
#endif
      fclose(fp);
   } else {
      strcpy(sdir, "*");
   }
   hss_index(fspec,sdir,0,'*');
   
//   printf("sdir:%s\n",sdir);
   sprintf(fdir, "%s%s", basedir, emudir);
   n = scandir(fdir, &namelist, NULL, alphasort);
   
   if (n == -1) {
      perror("scandir");
   }
   t=2;
   while (n--) {
      if(strncmp(fspec,namelist[n]->d_name,strlen(fspec))==0 && t<7) {
	 if(strcmp(namelist[n]->d_name,"..")!=0 && strcmp(namelist[n]->d_name,".")!=0) {
	    strcpy(cfg_button[t].strval, namelist[n]->d_name);
	    cfg_button[t].ival=0;
//	    printf("match:%s\n", namelist[n]->d_name);
	    if(strcmp(emubin, namelist[n]->d_name)==0)
	      cfg_button[t].ival=1;
	    t++;
	 }	 
      }
      free(namelist[n]);
   }
   free(namelist);
   
} // cfg_emuscandir

void cfg_emuconfig(char *emuname) {
   FILE *fp;
   SDL_Rect src_r;
   char picname[350],picnoext[346];
   char emucd[160], line[256], w[40], defemu[20];
   int profile, t;

   strcpy(g_emuname, emuname);
   hss_index(g_shremu, emuname, 0, '#');
   hss_index(w, emuname, 1, '#');
   profile=atoi(w);
   sprintf(g_emunamebin, "%s_bin", g_shremu);
//   printf("parms:  emuname:%s\n", emuname);
//   printf("        g_shremu:%s\n", g_shremu);
//   printf("        w:%s      profile:%d\n", w, profile);
   sprintf(picnoext,"%s%cpics%cdialog%cemuconfig", basedir, mysep, mysep, mysep);
   AddPicExt(picname,picnoext);

   if(strcmp(picname, "null")!=0) {
      if(strcmp(cfg_cache[B_ECONFIG].imgname, "EmuConfig")==0) {
	 src_r.x=0; src_r.y=0; src_r.w=cfg_cache[B_ECONFIG].w; src_r.h=cfg_cache[B_ECONFIG].h;
      } else {
//	 printf("loading %s\n", picname);
	 csurf = IMG_Load(picname);
	 src_r.x=0; src_r.y=0; src_r.w=csurf->w; src_r.h=csurf->h;
	 
	 SDL_DestroyTexture(cfg_cache[B_ECONFIG].texture);
	 cfg_cache[B_ECONFIG].w = csurf->w;
	 cfg_cache[B_ECONFIG].h = csurf->h;
	 cfg_cache[B_ECONFIG].texture = SDL_CreateTextureFromSurface(sdlRenderer, csurf);
	 strcpy(cfg_cache[B_ECONFIG].imgname, "EmuConfig");
	 SDL_FreeSurface(csurf);
      }
      // center on screen
      cfgw.w = scale_calc(1440, usey, src_r.w);
      cfgw.h = scale_calc(1440, usey, src_r.h);
      cfgw.x=(usex-cfgw.w)/2;  cfgw.y=(usey-cfgw.h)/2;

      // center this in the grid area
      // dst_r.x = ((rc.icon_w * rc.grid_w) - dst_r.w) / 2 + rc.grid_x;
      // dst_r.y = ((rc.icon_h * rc.grid_h) - dst_r.h) / 2 + rc.grid_y;
      
      SDL_RenderCopy(sdlRenderer, cfg_cache[B_ECONFIG].texture, &src_r, &cfgw);      
      if(profile>0)
	sprintf(picname, "%s config (profile #%d)", g_shremu, profile); 
      else
	sprintf(picname, "%s config", emuname); 
      grid_ttf_print(cfgw.x+200, cfgw.y+44, picname);
      
      SDL_RenderPresent(sdlRenderer);
      
      // should we load cfg here?
      sprintf(picname,"%s%cpics%cdialog%cemuconfig.cfg", basedir, mysep, mysep, mysep);
      cfg_loadfile(picname);
      
   } else {
      printf("ERROR: Cannot Load: %s\n", picname);
   }

   sprintf(picnoext,"%s%cpics%cdialog%cbuttons", basedir, mysep, mysep, mysep);
   AddPicExt(picname,picnoext);
   
   if(strcmp(picname, "null")!=0) {
      if(strcmp(cfg_cache[B_MBUTTON].imgname, "Buttons")==0) {
	 src_r.x=0; src_r.y=0; src_r.w=cfg_cache[B_MBUTTON].w; src_r.h=cfg_cache[B_MBUTTON].h;
      } else {
//	 printf("loading %s\n", picname);
	 csurf = IMG_Load(picname);
	 src_r.x=0; src_r.y=0; src_r.w=csurf->w; src_r.h=csurf->h;
	 
	 SDL_DestroyTexture(cfg_cache[B_MBUTTON].texture);
	 cfg_cache[B_MBUTTON].w = csurf->w;
	 cfg_cache[B_MBUTTON].h = csurf->h;
	 cfg_cache[B_MBUTTON].texture = SDL_CreateTextureFromSurface(sdlRenderer, csurf);
	 strcpy(cfg_cache[B_MBUTTON].imgname, "Buttons");
	 SDL_FreeSurface(csurf);
      }
   }

   // set defaults
   sprintf(emucd,"%s%cuser_config%ccfg1%cemucd.env",basedir,mysep,mysep,mysep);

   // MAYBE WE SHOULD HAVE A FUNCTION
   // that returns the value of passed variables?  
   // It might be useful for other things
   if(!file_exists(emucd)) {
      sprintf(emucd,"%s%cetc%cemucd.env",basedir,mysep,mysep);
   }
//   printf("%s\n", emucd);
   if ((fp = fopen(emucd, "rb"))==NULL) {
      printf("Error opening rc file %s", emucd);
   } else {
      while(!feof(fp)){
	 fgets(line, 255, fp);
	 if( line[0] != '#' ){
	    line[strlen(line)-1]=0;  /* Strip LF */
	    sprintf(w,"%s_bin", g_shremu);
	    if(strncmp(line, w, strlen(w))==0)
	      hss_index(g_emubin, line, 1, '=');

	    sprintf(w,"%s_dir", g_shremu);
	    if(strncmp(line, w, strlen(w))==0)
	      hss_index(g_emudir, line, 1, '=');
	    
	    hss_index(w, line, 1, '=');   // get system name
	    if(strncmp(w, dirname, strlen(dirname)-1)==0) {
	       hss_index(w, line, 0, '=');
	       hss_index(g_system, w, 0, '_');
	    }
	    
	 }
      }
      fclose(fp);
//      printf("emudir:%s\n",g_emudir);
//      printf("emubin:%s\n",g_emubin);
//      printf("system:%s\n",g_system);
   }

   // load default emu file, if exists
   sprintf(emucd,"%s%cuser_config%ccfg1%ccfg%c%s.emu",basedir,mysep,mysep,mysep,mysep,g_system);
   strcpy(defemu,"NULL");
   if(file_exists(emucd)) {
      if ((fp = fopen(emucd, "rb"))==NULL) {
	 printf("Error opening file %s", emucd);
      } else {
	 while(!feof(fp)) {
	    fgets(line, 255, fp);
	    hss_index(w, line, 0, '|');
	    if(strcmp(w,"emulator")==0)
	      hss_index(defemu, line, 1, '|');
	 }
	 fclose(fp);
      }
   }
//   printf("default emulator=%s\n", defemu);
   if(strcmp(defemu, emuname)==0)
     cfg_button[1].ival=1;
   
   // add rest here
   // load default values for fileselect
   for(t=2; t<7; t++) {
      strcpy(cfg_button[t].strval, " ");
      cfg_button[t].ival=255;
   }
   
   cfg_emuscandir(g_emudir, g_emubin);   

   cfg_refreshall();
} // cfg_emuconfig

void cfg_maindialog() {
   SDL_Rect src_r;
   char picname[350],picnoext[346], emucd[200], enval[12];

   strcpy(g_emunamebin, "XnullX");
   sprintf(picnoext,"%s%cpics%cdialog%csettings", basedir, mysep, mysep, mysep);
   AddPicExt(picname,picnoext);
   
   if(strcmp(picname, "null")!=0) {
      if(strcmp(cfg_cache[B_MCONFIG].imgname, "MainConfig")==0) {
	 src_r.x=0; src_r.y=0; src_r.w=cfg_cache[B_MCONFIG].w; src_r.h=cfg_cache[B_MCONFIG].h;
      } else {
//	 printf("loading %s\n", picname);
	 csurf = IMG_Load(picname);
	 src_r.x=0; src_r.y=0; src_r.w=csurf->w; src_r.h=csurf->h;
	 
	 SDL_DestroyTexture(cfg_cache[B_MCONFIG].texture);
	 cfg_cache[B_MCONFIG].w = csurf->w;
	 cfg_cache[B_MCONFIG].h = csurf->h;
	 cfg_cache[B_MCONFIG].texture = SDL_CreateTextureFromSurface(sdlRenderer, csurf);
	 strcpy(cfg_cache[B_MCONFIG].imgname, "MainConfig");
	 SDL_FreeSurface(csurf);
      }

      // center on screen
      cfgw.w = scale_calc(1440, usey, src_r.w);
      cfgw.h = scale_calc(1440, usey, src_r.h);
//      dst_r.w=src_r.w; dst_r.h=src_r.h;
      cfgw.x=(usex-cfgw.w)/2;  cfgw.y=(usey-cfgw.h)/2;

      // center this in the grid area
      // dst_r.x = ((rc.icon_w * rc.grid_w) - dst_r.w) / 2 + rc.grid_x;
      // dst_r.y = ((rc.icon_h * rc.grid_h) - dst_r.h) / 2 + rc.grid_y;
      
      SDL_RenderCopy(sdlRenderer, cfg_cache[B_MCONFIG].texture, &src_r, &cfgw);
      
      SDL_RenderPresent(sdlRenderer);
      
      // should we load cfg here?
      sprintf(picname,"%s%cpics%cdialog%cmainconfig.cfg", basedir, mysep, mysep, mysep);
      cfg_loadfile(picname);
   } else {
      printf("ERROR: Cannot Load: %s\n", picname);
   }

   sprintf(picnoext,"%s%cpics%cdialog%cbuttons", basedir, mysep, mysep, mysep);
   AddPicExt(picname,picnoext);
   
   if(strcmp(picname, "null")!=0) {
      if(strcmp(cfg_cache[B_MBUTTON].imgname, "Buttons")==0) {
	 src_r.x=0; src_r.y=0; src_r.w=cfg_cache[B_MBUTTON].w; src_r.h=cfg_cache[B_MBUTTON].h;
      } else {
//	 printf("loading %s\n", picname);
	 csurf = IMG_Load(picname);
	 src_r.x=0; src_r.y=0; src_r.w=csurf->w; src_r.h=csurf->h;
	 
	 SDL_DestroyTexture(cfg_cache[B_MBUTTON].texture);
	 cfg_cache[B_MBUTTON].w = csurf->w;
	 cfg_cache[B_MBUTTON].h = csurf->h;
	 cfg_cache[B_MBUTTON].texture = SDL_CreateTextureFromSurface(sdlRenderer, csurf);
	 strcpy(cfg_cache[B_MBUTTON].imgname, "Buttons");
	 SDL_FreeSurface(csurf);
      }
   }
   // set defaults
   //   read emucd.env file 
   sprintf(emucd,"%s%cetc%cemucd.env",basedir,mysep,mysep);
   env_load(emucd);
   sprintf(emucd,"%s%cuser_config%ccfg1%cemucd.env",basedir,mysep,mysep,mysep);
   if(file_exists(emucd)) {
      env_load(emucd);
   }
   env_get(enval,"FULLSCREEN");
   if (enval[0] == 'Y' || enval[0] == 'y')  cfg_presetival("FULLSCREEN", 1);
   env_get(enval,"SOUND");
   if (enval[0] == 'Y' || enval[0] == 'y')  cfg_presetival("SOUND", 1);
   env_get(enval,"MIDI");
   if (enval[0] == 'Y' || enval[0] == 'y')  cfg_presetival("MIDI", 1);
   env_get(enval,"JOYSTICK");
   if (enval[0] == 'Y' || enval[0] == 'y' || enval[0] == 'a' || enval[0] == 'A')  
     cfg_presetival("JOYSTICK", 1);
   env_get(enval,"FASTCPU");
   if (enval[0] == 'Y' || enval[0] == 'y')  cfg_presetival("FASTCPU", 1);
   // Radio Buttons
   env_get(enval,"SCANLINE");
   switch(enval[0]) {
    case '0':
      cfg_presetival("NOCRT", 1);
      break;
    case '1':
      cfg_presetival("BASIC", 1);
      break;
    case '2':
      cfg_presetival("ADVANCED", 1);
      break;
    case '3':
      cfg_presetival("CURVATURE", 1);
      break;
   }
   // UI settings
   if (fullscr == 'y')  cfg_presetival("UIFULLSCREEN", 1);
   cfg_presetival("UICLICK", UIclick);
   cfg_presetival("UIJOY", joy_enable);
   cfg_presetival("UIDFLTEMU", UseDfltEmu);
   cfg_refreshall();

} // cfg_maindialog()


void cfg_loadfile(char *filen) {
   FILE *fp;
   char line[256], f1[20], f2[6], btype[20];
   int m, t=0, x, y, w, h, type;

//   printf("cfg_loadfile(%s)\n", filen);
   ButtonMax=0;
   if ((fp = fopen(filen,"rb"))==NULL) {
      printf("Error opening rc file %s",filen);
      exit(1);
   }
   
   while(!feof(fp)) {
      fgets(line,255,fp);
      if( line[0] != '#' ) {
	 m=strlen(line);
	 line[m-1]=0;  /* Strip LF */
      }
      
      cfg_button[t].ival=0;  // reset value
      cfg_button[t].changed=0;  // reset value
      hss_index(btype,line,2,':');
      hss_index(f1,line,0,':');  // field 0
      hss_index(f2,f1,0,',');
      cfg_button[t].x=atoi(f2);
      hss_index(f2,f1,1,',');
      cfg_button[t].y=atoi(f2);

      hss_index(f1,line,1,':'); // field 1
      hss_index(f2,f1,0,',');
      cfg_button[t].w=atoi(f2);
      hss_index(f2,f1,1,',');
      cfg_button[t].h=atoi(f2);
      hss_index(f1,line,3,':');
      strcpy(cfg_button[t].var, f1);
      
//      printf("btype:%s\n",btype);
      
      if(strncmp(btype, "BUTTON", 6)==0) {
	 cfg_button[t].type=CFG_BUTTON;
	 t++;
      }

      if(strncmp(btype, "RADIO", 5)==0) {
	 cfg_button[t].type=CFG_RADIO;
	 t++;
      }

      if(strncmp(btype, "CHECK", 5)==0) {
	 cfg_button[t].type=CFG_CHECK;
	 t++;
      }

      if(strncmp(btype, "SELECT", 6)==0) {
	 cfg_button[t].type=CFG_SELECT;
	 t++;
      }
	     
   } // end while
   cfg_button[t].type=CFG_END;
   ButtonMax=t;
   numchanges=0;
   
   fclose(fp);
//   cfg_dumpbutton();
} // cfg_load()

void cfg_dumpbutton() {
   int i;
   
   for (i=0;i<24;i++) {
      if(cfg_button[i].type == CFG_END) break;
      printf("[%d]- %d x %d | %d , %d | %d | %s\n", i, cfg_button[i].x, cfg_button[i].y,
	     cfg_button[i].w, cfg_button[i].h, cfg_button[i].type, cfg_button[i].var);
   }
   
} // cfg_dumpbutton  may not need this

void cfg_loop() {
   int quit=0;
   int action=-1;
   int i=0;
   SDL_Event event;
   
   while(quit == 0) {

      SDL_WaitEvent(&event);
      switch (event.type) {
	 // keyboard
       case SDL_KEYDOWN:
	 switch (event.key.keysym.sym) {
	  case SDLK_TAB:
	    action=ACTION_TAB;
	    break;

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
	    
	  case SDLK_RETURN:
	    action=ACTION_SELECT;
	    break;
	      
	  case SDLK_ESCAPE:
	    action=ACTION_ESCAPE;
	    break;
	 } // switch SDL_KEYDOWN
	 
	 // Joysticks
	 // missing for now :)
	 
	 // D-pad;
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
	    if( event.jbutton.button == 1 ) {
	       if(event.jbutton.state == 1 ) {
		  action=ACTION_ESCAPE;
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

       case ACTION_TAB:
	 cfg_unhighlight(i);
	 i++;
	 if(cfg_button[i].type == 255)
	   i=0;
	 cfg_highlight(i);
	 break;

       case ACTION_SELECT:
	 if(cfg_button[i].ival<255) {
	    cfg_unhighlight(i);
	    cfg_setvalue(i);
	    if(cfg_button[i].type==CFG_BUTTON)
	      quit = 1;
	    cfg_highlight(i);
	 }
	 break;
	 
       case ACTION_UP:
	 cfg_unhighlight(i);
	 i=cfg_search_button(i, D_UP);
	 cfg_highlight(i);
	 break;
      
       case ACTION_DOWN:
	 cfg_unhighlight(i);
	 i=cfg_search_button(i, D_DOWN);
	 cfg_highlight(i);
	 break;

       case ACTION_LEFT:
	 cfg_unhighlight(i);
	 i=cfg_search_button(i, D_LEFT);
	 cfg_highlight(i);
	 break;
      
       case ACTION_RIGHT:
	 cfg_unhighlight(i);
	 i=cfg_search_button(i, D_RIGHT);
	 cfg_highlight(i);
	 break;
      }
      action=ACTION_NULL;
   } // while(quit==0)
} // cfg_loop()
