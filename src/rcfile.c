#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "font.h"
#include "font_legacy.h"
#include "emufe.h"

extern char mysep;
extern char dirname[120];

extern int env_get(char *val, const char *var);
extern void dfixsep2(char *opath, char *ipath, int setfq);

int hss_count(const char *ostr, const char del) {
   int rv=1,i;
   for(i=0;i<strlen(ostr);i++) {
      if(ostr[i]==del) rv++;
   }
   return(rv);
}

int hss_index(char* rstr, char *ostr, int idx, char del) {
   // Alternative to strtok
   char *gidx;
   int t=0,i=0,si=0,ei=0,sl;

   gidx=ostr;

   do {
      if(gidx[t]==del || gidx[t]=='\n' || gidx[t]==0) {
	 i++;
	 if(i==idx) si=t+1;
	 if(i==idx+1) { ei=t; break; }
      }

      if(gidx[t]==0) break;
      t++;
   } while(1);

   sl=ei-si;
   if(sl<1) strcpy(rstr,"");
   else {
      gidx=gidx+si;
      for(i=0;i<sl;i++) {
	 rstr[i]=gidx[i];
      }
      rstr[i]=0;
   }
}

int load_defaults() {
    int i;
    strcpy(descdir, "./desc");
    strcpy(bgpic, "default");
    strcpy(txtbx[B_MENU].box, "default");
    strcpy(txtbx[B_BANR].box, "default");
    strcpy(txtbx[B_DESC].box, "default");
    strcpy(txtbx[B_SETUP].box, "default");
    strcpy(picbox, "default");
    strcpy(theme, "default");
    strcpy(rc.ttfont, "na");
    strcpy(tfont, "silkfont.fnt");
    strcpy(rc.resolution, "640x480");
    strcpy(rc.fontsize, "8x16");
    strcpy(gthemedir, "na");
    strcpy(tfontbmp, "na");
//    strcpy(imenu.kbname, "na");

    rc.mode=MODE_CLASSIC;
    rc.gridres=0;
    rc.startmode=rc.mode;
   
    rc.banr.sh.enable='N';
    rc.banr.bg.enable='N';
    cachefont=0;
    fullscr='a';
//    texthlr=texthlg=texthlb=0;
    descbgr=descbgg=descbgb=255;
    rc.banr.sh.r=rc.banr.sh.g=rc.banr.sh.b=0;
    rc.banr.bg.r=rc.banr.bg.g=rc.banr.bg.b=0;
    rc.banr.fg.r=rc.banr.fg.g=rc.banr.fg.b=255;
    imenu.col[0].bg.r=imenu.col[1].bg.g=imenu.col[1].bg.b=0;
    imenu.col[1].bg.r=imenu.col[1].bg.g=imenu.col[1].bg.b=128;
    imenu.col[1].sh.r=imenu.col[1].sh.g=imenu.col[1].sh.b=0;
    imenu.col[3].fg.r=imenu.col[3].fg.g=imenu.col[3].fg.b=128;
    rc.font_sox=rc.font_soy=1;
    // box settings -- new 3.0
    rc.pb_x=364;  rc.pb_y=96;  rc.pb_w=244;  rc.pb_h=152;
    rc.pb_x2=rc.pb_x+rc.pb_w;
    rc.pb_y2=rc.pb_y+rc.pb_h;

    rc.mb_x=32;  rc.mb_y=96;  rc.mb_w=316;  rc.mb_h=152;
    rc.mb_x2=rc.mb_x+rc.mb_w;
    rc.mb_y2=rc.mb_y+rc.mb_h;

    rc.db_x=32;  rc.db_y=264;  rc.db_w=576;  rc.db_h=184;
    rc.db_x2=rc.db_x+rc.db_w;
    rc.db_y2=rc.db_y+rc.db_h;
    // font
    rc.font_w=8; rc.font_h=16;
   for(i=0;i<12;i++) {
      imgbx[i].enabled=0;
      imgbx[i].mgn=0;
      strcpy(imgbx[i].imgname,"na");
      imgbx[i].r=imgbx[i].g=imgbx[i].b=128;
      imgbx[i].masktype=0;
       imgbx[i].ovpct=0;
      strcpy(imgbx[i].ovname,"na");
      strcpy(imgbx[i].mask,"na");
   }
   // Reset Text boxes
   for(i=0;i<4;i++) {
      strcpy(txtbx[i].font,"na");
      txtbx[i].fonttype=-1;
      txtbx[i].font_w=rc.font_w;
      txtbx[i].font_h=rc.font_h;
      txtbx[i].font_v=rc.font_h;
   }
   // Grid Defaults
   rc.g_grid_x=600;  rc.g_grid_y=88;  rc.g_grid_w=6;  rc.g_grid_h=4;
   rc.grid_x=rc.g_grid_x;  rc.grid_y=rc.g_grid_y;
   rc.grid_w=rc.g_grid_w;  rc.grid_h=rc.g_grid_h;
   rc.g_img_w=300;  rc.g_img_h=300;  rc.g_icon_w=320;  rc.g_icon_h=320;
   rc.img_w=rc.g_img_w;  rc.img_h=rc.g_img_h;
   rc.icon_w=rc.g_icon_w;  rc.icon_h=rc.g_icon_h;
   rc.hdr_x=1280; rc.hdr_y=21;
   rc.gridfontsize=40;

   // new icons
   // ideal 1080p settings
   rc.icon_power_x=24;  rc.icon_joy_x=96;  rc.icon_gear_x=2472;
   rc.topicons_y=24;  rc.topicons_w=64;  rc.topicons_h=64;

}

int hextod(char b, char l) {

   /* Yes this is crappy code and I'm sure there is a much easier way,
    * I just needed to write it fast! */

   int hival, loval;
   switch(b) {
      case 'A': hival=160; break;
      case 'a': hival=160; break;
      case 'B': hival=176; break;
      case 'b': hival=176; break;
      case 'C': hival=192; break;
      case 'c': hival=192; break;
      case 'D': hival=208; break;
      case 'd': hival=208; break;
      case 'E': hival=224; break;
      case 'e': hival=224; break;
      case 'f': hival=240; break;
      case 'F': hival=240; break;
      default: hival=(b-48)*16;
   }
   switch(l) {
      case 'a': loval=10; break;
      case 'A': loval=10; break;
      case 'b': loval=11; break;
      case 'B': loval=11; break;
      case 'c': loval=12; break;
      case 'C': loval=12; break;
      case 'd': loval=13; break;
      case 'D': loval=13; break;
      case 'e': loval=14; break;
      case 'E': loval=14; break;
      case 'f': loval=15; break;
      case 'F': loval=15; break;
      default: loval=(l-48);
   }
   return hival+loval;
}

int set_generic_rc() {
  /*   A generic rc would look like this:
   * 
   *  VERSION=3.5
   *  FONTDIR=../../fonts
   *  PICSDIR=./pics
   *  SHADOW=Y or N
   *  MENUNAME=<system>.menu
   *  DEFLTIMG=<system>.pcx
   *  THEME=../../etc/theme.rc
   */
   char tmpstr[300], t_system[20];

   sprintf(tmpstr,"%s%c%s%cpics",basedir,mysep,dirname,mysep);
   strcpy(picsdir,tmpstr);   // PICSDIR
//printf("-- sgr: Set picsdir to '%s'\n", picsdir);
   
   sprintf(tmpstr, "%s%cfonts", basedir, mysep);
   strcpy(fontdir,tmpstr);   // FONTDIR

   fshadow=1;   // SHADOW

   sprintf(tmpstr, "%s%cetc%ctheme.rc", basedir, mysep, mysep);
   strcpy(theme,tmpstr);   // THEME

   mod_getsystem(t_system, imenu.sysbase);
   sprintf(tmpstr,"%s.menu", t_system);
   strcpy(menuname,tmpstr);   // MENUNAME

   sprintf(tmpstr,"%s.pcx", t_system);
   strcpy(defimg,tmpstr);   // DEFLTIMG

//   sprintf(imenu.kbname,"%s_%s",imenu.emulator,t_system);
//   dfixsep2(fontdir,"../../fonts",1);
//   dfixsep2(picsdir,"./pics",1);  /* might have to stretch this out */
//   strcpy(menuname,"nes.menu");  /* fixme */
//   strcpy(defimg,"nes.pcx");  /* fixme */
//   dfixsep2(theme,tmpstr,1);
}

int load_rc(char *filen) {
   /* The rc file should be in the same directory as emufe */
   FILE *fp;
   char line[256], tmpstr[120];
   char key[50], value[255];
   char h1, h2;
   int m, i;
   char *envfull, *envjoy, emuhome[220];

// printf("-- lrc:  started with file:'%s'\n", filen);
   if(imenu.mode>=1) {
      env_get(tmpstr,"EMUFEjoy");
      if ( tmpstr[0] == 'n') 
	joy_enable=0;
   } else {
      envjoy = getenv("EMUFEjoy");
      if ( envjoy != NULL && envjoy[0] == 'n')
	joy_enable=0;
   }
#ifdef DEBUG
   LOG(1, ("!!!RC loading: %s\n", filen));
#endif
   if ((fp = fopen(filen,"rb"))==NULL)  {
      printf("Error opening rc file %s",filen);
      exit(1);
   }

   rc.mode=rc.startmode;
   
   while(!feof(fp)) {
      fgets(line,255,fp);
      if( line[0] != '#' ) {
	 m=strlen(line);
	 /* Strip LF */ 
	 line[m-1]=0;

	 // The new way,  key and value need to be defined statically
	 // or have memory allocated before hand
	 // like "char key[55], value[200]
	 // This will allow blank lines in the rc file, the old method won't
	 hss_index(key,line,0,'=');
	 hss_index(value,line,1,'=');

// The old way, this requires key, value, and v to be initialized
// dynamically  like   "char *v, *key, *value;"
//	 v=(char *)strchr(line, '=');
//	 dvalue=(char *)strchr(v, v[1]);
//	 v[0]=0;
//	 dkey=line;
#ifdef DEBUG
	 LOG(3, ("* Processing Directive: %s\n", key));
#endif
	 if(strncmp(key, "FONTDIR", 7)==0) {
//	    strcpy(fontdir,value);
	    dfixsep2(fontdir,value,1);
#ifdef DEBUG
	    LOG(2, ("**fontdir is %s\n",fontdir));
#endif
	 }
	 if(strncmp(key, "TTFNAME", 7)==0) {
	    strcpy(rc.ttfont,value);
	 }
	 if(strncmp(key, "FONTNAME", 8)==0) {
	    strcpy(tfont,value);
	 }
	 if(strncmp(key, "FONTSIZE", 8)==0) {
	    strcpy(rc.fontsize,value);
	 }
	 if(strncmp(key, "FONTBMP", 7)==0) {
	    strcpy(tfontbmp,value);
	 }
	 if(strncmp(key, "CACHEFONT", 9)==0) {
	    if (value[0] == 'y' || value[0] == 'Y' )
	      cachefont=1;
	 }
	 if(strcmp(key, "FNTSHDOFFSET")==0) {
	    hss_index(tmpstr,value,0,',');
	    rc.font_sox=atoi(tmpstr);
	    hss_index(tmpstr,value,1,',');
	    rc.font_soy=atoi(tmpstr);
	 }


	 if(strncmp(key, "GTHEMEDIR", 9)==0) {
//	    emuhome=getenv("CDROOT");
//	    if(!emuhome) {
//		 emuhome=(char *)malloc(5);
//		 strcpy(emuhome,".");
//	      }
	    if(strcmp(cdroot,"") != 0)
	      strcpy(emuhome,cdroot);
	    else {
	       env_get(emuhome,"EMUHOME");
	       if(strcmp(emuhome,"") == 0)
		 strcpy(emuhome,".");
	    }
//	    sprintf(gthemedir,"%s/%s",emuhome,value);
	    sprintf(gthemedir,"%s%c%s",emuhome,mysep,value);
	 }
	 if(strncmp(key, "THEME", 5)==0) {
//	    strcpy(theme,value);
	    sprintf(tmpstr,"%s%s",dirname,value);
	    dfixsep2(theme,tmpstr,1);
#ifdef DEBUG
	    LOG(5, ("\nff:%s  %s\n",tmpstr,value));
	    LOG(5, ("ff2:%s\n",theme));
#endif
//	    abs_dirname(theme,value);
	    if(fileio_file_exists(theme)) {
	       load_rc(theme);
	    }
	 }
	 if(strncmp(key, "BACKGROUND", 10)==0) {
	    strcpy(bgpic,value);
	 }
	 if(strncmp(key, "TITLEBOX", 8)==0) {
	    strcpy(txtbx[B_BANR].box,value);
	 }
	 if(strncmp(key, "DESCBOX", 7)==0) {
	    strcpy(txtbx[B_DESC].box,value);
	 }
	 if(strncmp(key, "MENUBOX", 7)==0) {
	    strcpy(txtbx[B_MENU].box,value);
	 }
	 if(strncmp(key, "MENUFONT", 7)==0) {
	    strcpy(txtbx[B_MENU].font,value);
	 }
	 if(strncmp(key, "MENUFTYP", 7)==0) {
	    txtbx[B_MENU].fonttype=atoi(value);
	 }
	 if(strncmp(key, "MENUFSIZ", 7)==0) {
	    hss_index(tmpstr,value,0,'x');
	    txtbx[B_MENU].font_w=atoi(tmpstr);
	    hss_index(tmpstr,value,1,'x');
	    txtbx[B_MENU].font_h=atoi(tmpstr);
	    txtbx[B_MENU].font_v=txtbx[B_MENU].font_h+2;
	    LOG(1, ("GLOADED font: %s  type:%d  size: %d X %d\n", txtbx[B_MENU].font,txtbx[B_MENU].fonttype,txtbx[B_MENU].font_w,txtbx[B_MENU].font_h));
	 }
	 // Setup Dialog Box
	 if(strncmp(key, "SETUPFONT", 8)==0) {
	    strcpy(txtbx[B_SETUP].font,value);
	 }
	 if(strncmp(key, "SETUPFTYP", 8)==0) {
	    txtbx[B_SETUP].fonttype=atoi(value);
	 }
	 if(strncmp(key, "SETUPFSIZ", 8)==0) {
	    hss_index(tmpstr,value,0,'x');
	    txtbx[B_SETUP].font_w=atoi(tmpstr);
	    hss_index(tmpstr,value,1,'x');
	    txtbx[B_SETUP].font_h=atoi(tmpstr);
	    txtbx[B_SETUP].font_v=txtbx[B_SETUP].font_h;
	    LOG(1, ("DLOADED font: %s  type:%d  size: %d X %d\n", txtbx[B_SETUP].font,txtbx[B_SETUP].fonttype,txtbx[B_SETUP].font_w,txtbx[B_SETUP].font_h));
	 }

         if(strncmp(key, "MENUMODE", 8)==0) {
	    if(strncmp(value, "grid", 4)==0)
	      rc.mode=MODE_GRID;
	    if(strncmp(value, "ribbon", 6)==0)
	      rc.mode=MODE_RIBBON;
	    if(strncmp(value, "classic", 7)==0)
	      rc.mode=MODE_CLASSIC;
         }

	 if(strncmp(key, "RESOLUTION", 10)==0) {
	   hss_index(tmpstr,value,0,'x');
	   usex=atoi(tmpstr);
	   hss_index(tmpstr,value,1,'x');
	   usey=atoi(tmpstr);
	 }

	 // New Grid Parameters
	 if(strncmp(key, "GRIDRES", 7)==0) {
	    // This is the vertical res the grid parameters
	    // Are presented in, used to compute scaling
	    rc.gridres=atoi(value);
	 }
	 if(strncmp(key, "GRIDIMG", 7)==0) {
	    hss_index(tmpstr,value,0,'x');
	    rc.g_img_w = rc.img_w = atoi(tmpstr);
	    hss_index(tmpstr,value,1,'x');
	    rc.g_img_h = rc.img_h = atoi(tmpstr);
	 }

	 if(strncmp(key, "GRIDHDR", 7)==0) {
	    hss_index(tmpstr,value,0,'x');
	    rc.g_hdr_x = rc.hdr_x = atoi(tmpstr);
	    hss_index(tmpstr,value,1,'x');
	    rc.g_hdr_y = rc.hdr_y = atoi(tmpstr);
	   }
	 
	 if(strncmp(key, "GRIDICON", 8)==0) {
	    hss_index(tmpstr,value,0,'x');
	    rc.g_icon_w = rc.icon_w = atoi(tmpstr);
	    hss_index(tmpstr,value,1,'x');
	    rc.g_icon_h = rc.icon_h = atoi(tmpstr);
	 }

	 if(strncmp(key, "GRIDXY", 6)==0) {
	    hss_index(tmpstr,value,0,'x');
	    rc.g_grid_x = rc.grid_x = atoi(tmpstr);
	    hss_index(tmpstr,value,1,'x');
	    rc.g_grid_y = rc.grid_y = atoi(tmpstr);
	 }

	 if(strncmp(key, "GRIDWH", 6)==0) {
	    hss_index(tmpstr,value,0,'x');
	    rc.g_grid_w = rc.grid_w = atoi(tmpstr);
	    hss_index(tmpstr,value,1,'x');
	    rc.g_grid_h = rc.grid_h = atoi(tmpstr);
	 }

	 if(strncmp(key, "GRIDFONTSIZE", 12)==0) {
	    rc.gridfontsize=atoi(value);
	 }
	 // New grid Parameters 2024
	 if(strncmp(key, "TOPICONWH", 9)==0) {
	    hss_index(tmpstr,value,0,'x');
	    rc.topicons_w = atoi(tmpstr);
	    hss_index(tmpstr,value,1,'x');
	    rc.topicons_h = atoi(tmpstr);
	 }	 
	 
	 if(strncmp(key, "TOPICONY", 8)==0) {
	    rc.topicons_y=atoi(value);
	 }

	 if(strncmp(key, "ICON_POWER_X", 12)==0) {
	    rc.icon_power_x=atoi(value);
	 }

	 if(strncmp(key, "ICON_JOY_X", 10)==0) {
	    rc.icon_joy_x=atoi(value);
	 }
	 
	 if(strncmp(key, "ICON_GEAR_X", 11)==0) {
	    rc.icon_gear_x=atoi(value);
	 }
	 // End Grid Parameters
	 
	 if(strncmp(key, "MENUXY", 6)==0) {
	    hss_index(tmpstr,value,0,',');
	    rc.mb_x=atoi(tmpstr);
	    hss_index(tmpstr,value,1,',');
	    rc.mb_y=atoi(tmpstr);
	    hss_index(tmpstr,value,2,',');
	    rc.mb_w=atoi(tmpstr);
	    hss_index(tmpstr,value,3,',');
	    rc.mb_h=atoi(tmpstr);
	    rc.mb_x2=rc.mb_x+rc.mb_w;
	    rc.mb_y2=rc.mb_y+rc.mb_h;
	    LOG(2, ("MENUXY  :  %d,%d,%d,%d\n",rc.mb_x,rc.mb_y,rc.mb_w,rc.mb_h));
	 }
	 if(strncmp(key, "DESCXY", 6)==0) {
	   hss_index(tmpstr,value,0,',');
	   rc.db_x=atoi(tmpstr);
	   hss_index(tmpstr,value,1,',');
	   rc.db_y=atoi(tmpstr);
	   hss_index(tmpstr,value,2,',');
	   rc.db_w=atoi(tmpstr);
	   hss_index(tmpstr,value,3,',');
	   rc.db_h=atoi(tmpstr);
	   rc.db_x2=rc.db_x+rc.db_w;
	   rc.db_y2=rc.db_y+rc.db_h;
	   LOG(2, ("DESCXY  :  %d,%d,%d,%d : %d,%d\n",rc.db_x,rc.db_y,rc.db_w,rc.db_h,
		  rc.db_x2,rc.db_y2));
	 }
	 if(strncmp(key, "BANRXY", 6)==0) {
	   hss_index(tmpstr,value,0,',');
	   rc.bb_x=atoi(tmpstr);
	   hss_index(tmpstr,value,1,',');
	   rc.bb_y=atoi(tmpstr);
	   hss_index(tmpstr,value,2,',');
	   rc.bb_w=atoi(tmpstr);
	   hss_index(tmpstr,value,3,',');
	   rc.bb_h=atoi(tmpstr);
	   rc.bb_x2=rc.bb_x+rc.bb_w;
	   rc.bb_y2=rc.bb_y+rc.bb_h;
	 }
// PICBXY should be replaced with B_PICBOX_XY
	 if(strncmp(key, "PICBXY", 6)==0) {
	   hss_index(tmpstr,value,0,',');
	   rc.pb_x=atoi(tmpstr);
	   hss_index(tmpstr,value,1,',');
	   rc.pb_y=atoi(tmpstr);
	   hss_index(tmpstr,value,2,',');
	   rc.pb_w=atoi(tmpstr);
	   hss_index(tmpstr,value,3,',');
	   rc.pb_h=atoi(tmpstr);
	   rc.pb_x2=rc.pb_x+rc.pb_w;
	   rc.pb_y2=rc.pb_y+rc.pb_h;
	 }
// New, configurable picture boxes
	 if(strncmp(key, "B_BOXSCAN_XY", 12)==0) {
	   imgbx[B_BOXSCAN].enabled=1;
	   strcpy(imgbx[B_BOXSCAN].pfx,"bx_");
	   hss_index(tmpstr,value,0,',');
	   imgbx[B_BOXSCAN].x=atoi(tmpstr);
	   hss_index(tmpstr,value,1,',');
	   imgbx[B_BOXSCAN].y=atoi(tmpstr);
	   hss_index(tmpstr,value,2,',');
	   imgbx[B_BOXSCAN].w=atoi(tmpstr);
	   hss_index(tmpstr,value,3,',');
	   imgbx[B_BOXSCAN].h=atoi(tmpstr);
	   imgbx[B_BOXSCAN].x2=imgbx[B_BOXSCAN].x+imgbx[B_BOXSCAN].w;
	   imgbx[B_BOXSCAN].y2=imgbx[B_BOXSCAN].y+imgbx[B_BOXSCAN].h;
	 }
	 if(strncmp(key, "B_BOXSCAN_BG", 12)==0) {
	    imgbx[B_BOXSCAN].r=hextod(value[0],value[1]);
	    imgbx[B_BOXSCAN].g=hextod(value[2],value[3]);
	    imgbx[B_BOXSCAN].b=hextod(value[4],value[5]);
	 }
	 if(strncmp(key, "B_BOXSCAN_BM", 12)==0) {
	    strcpy(imgbx[B_BOXSCAN].imgname,value);
	 }
	 if(strcmp(key, "B_BOXSCAN_MK")==0) {
	    strcpy(imgbx[B_BOXSCAN].mask,value);
	 }
	 if(strncmp(key, "B_BOXSCAN_MG", 12)==0) {
	    // new, margins!
	    imgbx[B_BOXSCAN].mgn=atoi(value);
	    LOG(4, ("NWW: set margin to %d\n",imgbx[B_BOXSCAN].mgn));
	 }
	 if(strncmp(key, "B_BOXSCAN_MM", 12)==0) {
	    if(strcmp(value,"none")==0)
	      imgbx[B_BOXSCAN].masktype=0;
	    if(strcmp(value,"bitmap")==0)
	      imgbx[B_BOXSCAN].masktype=1;
	    if(strcmp(value,"bars")==0)
	      imgbx[B_BOXSCAN].masktype=2;
#ifdef DEBUG
	    LOG(4, ("dbg: Masktype:%s %d\n",value,imgbx[B_BOXSCAN].masktype));
#endif
	 }

	 if(strncmp(key, "B_PICBOX_XY", 11)==0) {
	   imgbx[B_PICBOX].enabled=1;
	   strcpy(imgbx[B_PICBOX].pfx,"");
	   hss_index(tmpstr,value,0,',');
	   imgbx[B_PICBOX].x=atoi(tmpstr);
	   hss_index(tmpstr,value,1,',');
	   imgbx[B_PICBOX].y=atoi(tmpstr);
	   hss_index(tmpstr,value,2,',');
	   imgbx[B_PICBOX].w=atoi(tmpstr);
	   hss_index(tmpstr,value,3,',');
	   imgbx[B_PICBOX].h=atoi(tmpstr);
	   imgbx[B_PICBOX].x2=imgbx[B_PICBOX].x+imgbx[B_PICBOX].w;
	   imgbx[B_PICBOX].y2=imgbx[B_PICBOX].y+imgbx[B_PICBOX].h;
	 }
	 if(strncmp(key, "B_PICBOX_BG", 11)==0) {
	    imgbx[B_PICBOX].r=hextod(value[0],value[1]);
	    imgbx[B_PICBOX].g=hextod(value[2],value[3]);
	    imgbx[B_PICBOX].b=hextod(value[4],value[5]);
	 }
	 if(strncmp(key, "B_PICBOX_BM", 11)==0) {
	    strcpy(imgbx[B_PICBOX].imgname,value);
	 }
	 if(strncmp(key, "B_KEYBOARD_XY", 13)==0) {
	   imgbx[B_KEYBOARD].enabled=1;
	   strcpy(imgbx[B_KEYBOARD].pfx,"kb_");
	   hss_index(tmpstr,value,0,',');
	   imgbx[B_KEYBOARD].x=atoi(tmpstr);
	   hss_index(tmpstr,value,1,',');
	   imgbx[B_KEYBOARD].y=atoi(tmpstr);
	   hss_index(tmpstr,value,2,',');
	   imgbx[B_KEYBOARD].w=atoi(tmpstr);
	   hss_index(tmpstr,value,3,',');
	   imgbx[B_KEYBOARD].h=atoi(tmpstr);
	   imgbx[B_KEYBOARD].x2=imgbx[B_KEYBOARD].x+imgbx[B_KEYBOARD].w;
	   imgbx[B_KEYBOARD].y2=imgbx[B_KEYBOARD].y+imgbx[B_KEYBOARD].h;
	 }
	 if(strcmp(key, "B_KEYBOARD_BG")==0) {
	    imgbx[B_KEYBOARD].r=hextod(value[0],value[1]);
	    imgbx[B_KEYBOARD].g=hextod(value[2],value[3]);
	    imgbx[B_KEYBOARD].b=hextod(value[4],value[5]);
	 }
	 if(strcmp(key, "B_KEYBOARD_BM")==0) {
	    strcpy(imgbx[B_KEYBOARD].imgname,value);
	 }
	 if(strcmp(key, "B_KEYBOARD_MK")==0) {
	    strcpy(imgbx[B_KEYBOARD].mask,value);
	 }
	 if(strcmp(key, "B_KEYBOARD_MM")==0) {
	    if(strcmp(value,"none")==0)
	      imgbx[B_KEYBOARD].masktype=0;
	    if(strcmp(value,"bitmap")==0)
	      imgbx[B_KEYBOARD].masktype=1;
	    if(strcmp(value,"bars")==0)
	      imgbx[B_KEYBOARD].masktype=2;
#ifdef DEBUG
	    LOG(4, ("dbg: Masktype:%s %d\n",value,imgbx[B_KEYBOARD].masktype));
#endif
	 }

	 if(strncmp(key, "B_SSHOT1_XY",12)==0) {
	   imgbx[B_SSHOT1].enabled=1;
	   strcpy(imgbx[B_SSHOT1].pfx,"s1_");
	   hss_index(tmpstr,value,0,',');
	   imgbx[B_SSHOT1].x=atoi(tmpstr);
	   hss_index(tmpstr,value,1,',');
	   imgbx[B_SSHOT1].y=atoi(tmpstr);
	   hss_index(tmpstr,value,2,',');
	   imgbx[B_SSHOT1].w=atoi(tmpstr);
	   hss_index(tmpstr,value,3,',');
	   imgbx[B_SSHOT1].h=atoi(tmpstr);
	   imgbx[B_SSHOT1].x2=imgbx[B_SSHOT1].x+imgbx[B_SSHOT1].w;
	   imgbx[B_SSHOT1].y2=imgbx[B_SSHOT1].y+imgbx[B_SSHOT1].h;
	 }
	 if(strncmp(key, "B_SSHOT1_BG", 12)==0) {
	    imgbx[B_SSHOT1].r=hextod(value[0],value[1]);
	    imgbx[B_SSHOT1].g=hextod(value[2],value[3]);
	    imgbx[B_SSHOT1].b=hextod(value[4],value[5]);
	 }
	 if(strncmp(key, "B_SSHOT1_BM", 12)==0) {
	    strcpy(imgbx[B_SSHOT1].imgname,value);
	 }
	 if(strncmp(key, "B_SSHOT1_OV", 12)==0) {
	    imgbx[B_SSHOT1].ovpct=50;
	    strcpy(imgbx[B_SSHOT1].ovname,value);
	 }
	 if(strncmp(key, "B_SSHOT1_MK", 12)==0) {
	    strcpy(imgbx[B_SSHOT1].mask,value);
	 }
	 if(strncmp(key, "B_SSHOT1_MM", 12)==0) {
	    if(strcmp(value,"none")==0)
	      imgbx[B_SSHOT1].masktype=0;
	    if(strcmp(value,"bitmap")==0)
	      imgbx[B_SSHOT1].masktype=1;
	    if(strcmp(value,"bars")==0)
	      imgbx[B_SSHOT1].masktype=2;
#ifdef DEBUG
	    LOG(4, ("dbg: Masktype:%s %d\n",value,imgbx[B_SSHOT1].masktype));
#endif
	 }
	 if(strncmp(key, "B_SSHOT2_XY", 12)==0) {
	   imgbx[B_SSHOT2].enabled=1;
	   strcpy(imgbx[B_SSHOT2].pfx,"s2_");
	   hss_index(tmpstr,value,0,',');
	   imgbx[B_SSHOT2].x=atoi(tmpstr);
	   hss_index(tmpstr,value,1,',');
	   imgbx[B_SSHOT2].y=atoi(tmpstr);
	   hss_index(tmpstr,value,2,',');
	   imgbx[B_SSHOT2].w=atoi(tmpstr);
	   hss_index(tmpstr,value,3,',');
	   imgbx[B_SSHOT2].h=atoi(tmpstr);
	   imgbx[B_SSHOT2].x2=imgbx[B_SSHOT2].x+imgbx[B_SSHOT2].w;
	   imgbx[B_SSHOT2].y2=imgbx[B_SSHOT2].y+imgbx[B_SSHOT2].h;
	 }
	 if(strncmp(key, "B_SSHOT2_BG", 12)==0) {
	    imgbx[B_SSHOT2].r=hextod(value[0],value[1]);
	    imgbx[B_SSHOT2].g=hextod(value[2],value[3]);
	    imgbx[B_SSHOT2].b=hextod(value[4],value[5]);
	 }
	 if(strncmp(key, "B_SSHOT2_BM", 12)==0) {
	    strcpy(imgbx[B_SSHOT2].imgname,value);
	 }
	 if(strncmp(key, "B_SSHOT2_OV", 12)==0) {
	    imgbx[B_SSHOT2].ovpct=50;
	    strcpy(imgbx[B_SSHOT2].ovname,value);
	 }
	 if(strncmp(key, "B_SSHOT2_MK", 12)==0) {
	    strcpy(imgbx[B_SSHOT2].mask,value);
	 }
	 if(strncmp(key, "B_SSHOT2_MM", 12)==0) {
	    if(strcmp(value,"none")==0)
	      imgbx[B_SSHOT2].masktype=0;
	    if(strcmp(value,"bitmap")==0)
	      imgbx[B_SSHOT2].masktype=1;
	    if(strcmp(value,"bars")==0)
	      imgbx[B_SSHOT2].masktype=2;
#ifdef DEBUG
	    LOG(4, ("dbg: Masktype:%s %d\n",value,imgbx[B_SSHOT2].masktype));
#endif
	 }
	 if(strncmp(key, "B_SSHOT3_XY", 12)==0) {
	   imgbx[B_SSHOT3].enabled=1;
	   strcpy(imgbx[B_SSHOT3].pfx,"s3_");
	   hss_index(tmpstr,value,0,',');
	   imgbx[B_SSHOT3].x=atoi(tmpstr);
	   hss_index(tmpstr,value,1,',');
	   imgbx[B_SSHOT3].y=atoi(tmpstr);
	   hss_index(tmpstr,value,2,',');
	   imgbx[B_SSHOT3].w=atoi(tmpstr);
	   hss_index(tmpstr,value,3,',');
	   imgbx[B_SSHOT3].h=atoi(tmpstr);
	   imgbx[B_SSHOT3].x2=imgbx[B_SSHOT3].x+imgbx[B_SSHOT3].w;
	   imgbx[B_SSHOT3].y2=imgbx[B_SSHOT3].y+imgbx[B_SSHOT3].h;
	 }
	 if(strncmp(key, "B_SSHOT3_BG", 12)==0) {
	    imgbx[B_SSHOT3].r=hextod(value[0],value[1]);
	    imgbx[B_SSHOT3].g=hextod(value[2],value[3]);
	    imgbx[B_SSHOT3].b=hextod(value[4],value[5]);
	 }
	 if(strncmp(key, "B_SSHOT3_BM", 12)==0) {
	    strcpy(imgbx[B_SSHOT3].imgname,value);
	 }
	 if(strncmp(key, "B_SSHOT3_OV", 12)==0) {
	    imgbx[B_SSHOT3].ovpct=50;
	    strcpy(imgbx[B_SSHOT3].ovname,value);
	 }
	 if(strncmp(key, "B_SSHOT3_MK", 12)==0) {
	    strcpy(imgbx[B_SSHOT3].mask,value);
	 }
	 if(strncmp(key, "B_SSHOT3_MM", 12)==0) {
	    if(strcmp(value,"none")==0)
	      imgbx[B_SSHOT3].masktype=0;
	    if(strcmp(value,"bitmap")==0)
	      imgbx[B_SSHOT3].masktype=1;
	    if(strcmp(value,"bars")==0)
	      imgbx[B_SSHOT3].masktype=2;
#ifdef DEBUG
	    LOG(4, ("dbg: Masktype:%s %d\n",value,imgbx[B_SSHOT3].masktype));
#endif
	 }

// End pic boxes
// PICBOX should be replaced with B_PICBOX_BM
	 if(strncmp(key, "PICBOX", 6)==0) {
	    strcpy(picbox,value);
	 }
	 if(strncmp(key, "PICSDIR", 7)==0) {
	    if(imenu.mode==2) {
	       sprintf(tmpstr,"%s%s",dirname,value);
	       dfixsep2(picsdir,tmpstr,1);
	    } else
	      dfixsep2(picsdir,value,1);
//	    strcpy(picsdir,value);
	 }
	 if(strncmp(key, "DESCDIR", 7)==0) {
	    strcpy(descdir,value);
//	    dfixsep2(descdir,value,0);
	 }
	 if(strncmp(key, "DESCFONT", 7)==0) {
	    strcpy(txtbx[B_DESC].font,value);
	 }
	 if(strncmp(key, "DESCFTYP", 7)==0) {
	    txtbx[B_DESC].fonttype=atoi(value);
	 }
	 if(strncmp(key, "DESCFSIZ", 7)==0) {
	    hss_index(tmpstr,value,0,'x');
	    txtbx[B_DESC].font_w=atoi(tmpstr);
	    hss_index(tmpstr,value,1,'x');
	    txtbx[B_DESC].font_h=atoi(tmpstr);
	    txtbx[B_DESC].font_v=txtbx[B_DESC].font_h;
	    LOG(1, ("DLOADED font: %s  type:%d  size: %d X %d\n", txtbx[B_DESC].font,txtbx[B_DESC].fonttype,txtbx[B_DESC].font_w,txtbx[B_DESC].font_h));
	 }


	 if(strncmp(key, "MENUC1BG", 8)==0) {
//	    textbgr=hextod(value[0],value[1]);
//	    textbgg=hextod(value[2],value[3]);
//	    textbgb=hextod(value[4],value[5]);
	    imenu.col[1].bg.r=hextod(value[0],value[1]);
	    imenu.col[1].bg.g=hextod(value[2],value[3]);
	    imenu.col[1].bg.b=hextod(value[4],value[5]);
	 }
	 if(strncmp(key, "DESCTX", 6)==0) {
	    // text description color
	    rc.txdesc_r=hextod(value[0],value[1]);
	    rc.txdesc_g=hextod(value[2],value[3]);
	    rc.txdesc_b=hextod(value[4],value[5]);
	 }
	 if(strncmp(key, "MENUC1FG", 8)==0) {
	    // main font color in menu window
	    imenu.col[1].fg.r=hextod(value[0],value[1]);
	    imenu.col[1].fg.g=hextod(value[2],value[3]);
	    imenu.col[1].fg.b=hextod(value[4],value[5]);
	 }
	 if(strncmp(key, "MENUHLFG", 8)==0) {
	    imenu.col[0].fg.r=hextod(value[0],value[1]);
	    imenu.col[0].fg.g=hextod(value[2],value[3]);
	    imenu.col[0].fg.b=hextod(value[4],value[5]);
	 }
	 if(strncmp(key, "MENUHLBG", 8)==0) {
	    imenu.col[0].bg.r=hextod(value[0],value[1]);
	    imenu.col[0].bg.g=hextod(value[2],value[3]);
	    imenu.col[0].bg.b=hextod(value[4],value[5]);
	 }
	 if(strncmp(key, "MENUC2FG", 8)==0) {
	    imenu.col[2].fg.r=hextod(value[0],value[1]);
	    imenu.col[2].fg.g=hextod(value[2],value[3]);
	    imenu.col[2].fg.b=hextod(value[4],value[5]);
	 }
	 if(strncmp(key, "MENUC3FG", 8)==0) {
	    imenu.col[3].fg.r=hextod(value[0],value[1]);
	    imenu.col[3].fg.g=hextod(value[2],value[3]);
	    imenu.col[3].fg.b=hextod(value[4],value[5]);
	 }
	 if(strncmp(key, "DESCBG", 6)==0) {
	    descbgr=hextod(value[0],value[1]);
	    descbgg=hextod(value[2],value[3]);
	    descbgb=hextod(value[4],value[5]);
	 }

	 if(strncmp(key, "BANRFG", 6)==0) {
	    rc.banr.fg.r=hextod(value[0],value[1]);
	    rc.banr.fg.g=hextod(value[2],value[3]);
	    rc.banr.fg.b=hextod(value[4],value[5]);
	 }
	 
	 if(strncmp(key, "BANRSH", 6)==0) {
	    rc.banr.sh.r=hextod(value[0],value[1]);
	    rc.banr.sh.g=hextod(value[2],value[3]);
	    rc.banr.sh.b=hextod(value[4],value[5]);
	    rc.banr.sh.enable='Y';
	 }
	 
	 if(strncmp(key, "BANRBG", 6)==0) {
	    rc.banr.bg.r=hextod(value[0],value[1]);
	    rc.banr.bg.g=hextod(value[2],value[3]);
	    rc.banr.bg.b=hextod(value[4],value[5]);
//	    printf("rc.banr.bg.=%d %d %d\n",rc.banr.bg.r, rc.banr.bg.g, rc.banr.bg.b); 
	 }
       	 if(strncmp(key, "BANRFONT", 7)==0) {
	    strcpy(txtbx[B_BANR].font,value);
//	    printf("GLOADED font: %s\n", txtbx[B_MENU].font);
	 }
	 if(strncmp(key, "BANRFTYP", 7)==0) {
	    txtbx[B_BANR].fonttype=atoi(value);
	 }
	 if(strncmp(key, "BANRFSIZ", 7)==0) {
	    hss_index(tmpstr,value,0,'x');
	    txtbx[B_BANR].font_w=atoi(tmpstr);
	    hss_index(tmpstr,value,1,'x');
	    txtbx[B_BANR].font_h=atoi(tmpstr);
	    txtbx[B_BANR].font_v=txtbx[B_BANR].font_h;
	    LOG(1, ("BLOADED font: %s  type:%d  size: %d X %d\n", txtbx[B_BANR].font,txtbx[B_BANR].fonttype,txtbx[B_BANR].font_w,txtbx[B_BANR].font_h));
	 }

	 if(strncmp(key, "MENUC1SH", 8)==0) {
//	    shadowr=hextod(value[0],value[1]);
//	    shadowg=hextod(value[2],value[3]);
//	    shadowb=hextod(value[4],value[5]);
	    imenu.col[1].sh.r=hextod(value[0],value[1]);
	    imenu.col[1].sh.g=hextod(value[2],value[3]);
	    imenu.col[1].sh.b=hextod(value[4],value[5]);
//	    printf("shadow=%d %d %d\n",imenu.col[1].bg.r, imenu.col[1].bg.g, imenu.col[1].bg.b);  
	 }
	 if(strncmp(key, "FULLSCREEN", 10)==0) {
	    fullscr=value[0];
	    if(imenu.mode>=1) {
	       env_get(tmpstr,"EMUFEfull");
	       if (tmpstr[0] == 'n' || tmpstr[0] == 'y' ) 
		 fullscr=tmpstr[0];
	    } else {
	       envfull = getenv("EMUFEfull");
	       if ( envfull != NULL && (envfull[0] == 'n' || envfull[0] == 'y' )) 
		 fullscr=envfull[0];
	    }

#ifdef DEBUG
	    LOG(3, ("Fullscreen mode: %c",fullscr));
#endif
	 }
	 // put systype into rc file unless we find a better place
	 if(strncmp(key, "SYSTYPE", 7)==0) {
	    if(strcmp(value,"generic")==0)
	      imenu.systype=SYS_GENERIC;
	    if(strcmp(value,"arcade")==0)
	      imenu.systype=SYS_ARCADE;
	    if(strcmp(value,"computer")==0)
	      imenu.systype=SYS_COMPUTER;
	 }
	 if(strncmp(key, "MENUNAME", 8)==0) {
	    strcpy(menuname,value);
	 }
	 if(strncmp(key, "DEFLTIMG", 8)==0) {
	    strcpy(defimg,value);
/*	    printf("default image is %s\n",defimg); */
	 }
	 if(strncmp(key, "SHADOW", 6)==0) {
	    if( value[0]=='y' || value[0]=='Y') {
	       fshadow=1;
	    } else {
	       fshadow=0;
	    }
	 }
//#ifdef DEBUG
//	 printf("%s\n",fontdir);
//#endif
      }
   }

   fclose(fp);
   if(strncmp(gthemedir, "na", 2) != 0)
     strcpy(fontdir, gthemedir);
   LOG(3, ("$$ fontdir is %s  g:%s\n",fontdir,gthemedir));
   
   if(rc.gridres != usey && rc.gridres > 0) {
//      printf("XX gridres:%d\n", rc.gridres);
//      printf("Resolution no match  res=%d  gridres=%d\n",usey, rc.gridres);
//      printf("before:%d\n", rc.img_w);
      rc.g_img_w = rc.img_w = scale_calc(rc.gridres, usey, rc.img_w);
//      printf("after:%d\n", rc.img_w);
      // this is getting scaled twice,
      // either prevent that or put a scaled flag in rc.
      rc.g_img_h = rc.img_h = scale_calc(rc.gridres, usey, rc.img_h);

      rc.g_icon_w = rc.icon_w = scale_calc(rc.gridres, usey, rc.icon_w);

      rc.g_icon_h = rc.icon_h = scale_calc(rc.gridres, usey, rc.icon_h);
      rc.g_grid_x = rc.grid_x = scale_calc(rc.gridres, usey, rc.grid_x);
      rc.g_grid_y = rc.grid_y = scale_calc(rc.gridres, usey, rc.grid_y);
      rc.g_hdr_x = rc.hdr_x = scale_calc(rc.gridres, usey, rc.hdr_x);
      rc.g_hdr_y = rc.hdr_y = scale_calc(rc.gridres, usey, rc.hdr_y);
      rc.gridfontsize = scale_calc(rc.gridres, usey, rc.gridfontsize);
      for(i=4;i<7;i++) {
	 if(imgbx[i].enabled==1) {
	    imgbx[i].x = scale_calc(rc.gridres, usey, imgbx[i].x);
	    imgbx[i].y = scale_calc(rc.gridres, usey, imgbx[i].y);
	    imgbx[i].w = scale_calc(rc.gridres, usey, imgbx[i].w);
	    imgbx[i].h = scale_calc(rc.gridres, usey, imgbx[i].h);
	 }
      }
      rc.icon_power_x = scale_calc(rc.gridres, usey, rc.icon_power_x);
      rc.icon_joy_x = scale_calc(rc.gridres, usey, rc.icon_joy_x);
      rc.icon_gear_x = scale_calc(rc.gridres, usey, rc.icon_gear_x);
      rc.topicons_y = scale_calc(rc.gridres, usey, rc.topicons_y);
      rc.topicons_w = scale_calc(rc.gridres, usey, rc.topicons_w);
      rc.topicons_h = scale_calc(rc.gridres, usey, rc.topicons_h);
      rc.gridres=0;
   }
}
