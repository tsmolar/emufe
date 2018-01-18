#include <stdio.h>
#include <dirent.h>
#ifdef USESDL
#include <SDL.h>
#include "sdl_allegro.h"
#endif

#ifdef USEALLEGRO
#include <allegro.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <window.h>
#include "font.h"
#include "font_legacy.h"
#include <button.h>
#include "emufe.h"
#include "fileio.h"
#include "dfilepath.h"
#include "rcfile.h"

typedef struct charar_t {
   char str[85];
} charar_t;

typedef struct map_t {
   char var[40];
   char value[20];
   char p3[30];
   char p4[80];
} map_t;

#define SU_BUTTON 1
#define SU_CHECKBOX 2
#define SU_ENVTEXT 3
#define SU_SELECT 4
#define SU_EDIT 5

#define MAX_ENVC 50
#define P_CANCEL 0
#define P_ACTIVE 1
#define P_SAVE 2
#define P_WRITE 4

#define B_NULL 0
#define B_CANCEL 1
#define B_WRITE 2
#define B_NEXT 4
#define B_PREV 8
#define B_ADV 16


typedef struct widgetlist_t {
   int type;
   int x,y;
   int w,h;
   char text[40];
   char evalue[40];  // store env value
   int value;
   Widget *widget;
} widgetlist_t;

typedef struct setwin_t {
   int width,height,numlines,numwidget;
   int startx,starty;
   int active;
   int lastbutton;
   charar_t line[28];
   charar_t ddown[20];
   widgetlist_t widget[25];
} setwin_t;

extern char mysep;
setwin_t swin;
int envcidx,mapidx;
env_t envc[MAX_ENVC];
map_t emap[MAX_ENVC];

int wcb_buttoncb(Widget *w, int x, int y, int m) {
   printf("SETUP: wcb_buttoncb\n");
//   printf("CALLBACK!-> %s\n",w->text);
   swin.active=P_CANCEL;
   swin.lastbutton=B_CANCEL;
   if(strcmp(w->text,"ACCEPT")==0 || strcmp(w->text," SAVE ")==0)
      swin.lastbutton=B_WRITE;
   if(strcmp(w->text,"NEXT->")==0 || strcmp(w->text,"NEXT >")==0)
     swin.lastbutton=B_NEXT;
   if(strcmp(w->text,"<-BACK")==0 || strcmp(w->text,"< BACK")==0)
     swin.lastbutton=B_PREV;
   if(strcmp(w->text,"ADVANCED")==0 || strcmp(w->text,"Advanced")==0 )
     swin.lastbutton=B_ADV;
     
   if(swin.lastbutton==B_NEXT || swin.lastbutton==B_PREV ||swin.lastbutton==B_ADV || swin.lastbutton==B_WRITE)
     swin.active=(P_SAVE);
//   printf("DUN WIF CALLBACK\n");
//   pop_level();
}

int limit_str(char *sstr,const char *istr,int len) {
   int i;
   printf("SETUP: limit_str\n");

   for(i=0;i<len;i++)
     sstr[i]=istr[i];
   sstr[len]=0;
}

int setup_findbin(const char *fb) {
   char fpath[80],sdir[40],fspec[20];
   int n;
   FILE *fp;
   DIR *dp;
   struct dirent *ep;

   printf("SETUP: setup_findbin\n");

   setup_envget(sdir,fb);
   dfixsep2(fpath,sdir,1);
   sprintf(fspec,"%s%cfilespec",fpath,mysep);
   if(file_exists(fspec)) {
      fp = fopen(fspec,"rb");
      fgets(sdir,36,fp);
#ifdef WIN32
      // remove ^M linefeeds
      for(n=0;n<strlen(sdir);n++) {
	 if(sdir[n]==13)
	   sdir[n]=0;
      }
#endif
      fclose(fp);
//      sprintf(fspec,"%s%c%s",fpath,mysep,sdir);
   } else {
      strcpy(sdir,"*");
//      sprintf(fspec,"%s%c*",fpath,mysep);
   }
   hss_index(fspec,sdir,0,'*');
#ifdef DEBUG
   printf("fpath:(fb:%s) %s\n",fb,fspec);
#endif
   
   strcpy(swin.ddown[0].str,"-=- CLOSE -=-");
   n=1;
   dp = opendir (fpath);
   if (dp != NULL) {
      while (ep = readdir (dp)) {
	 if(strncmp(fspec,ep->d_name,strlen(fspec))==0) {
	      strcpy(swin.ddown[n].str,ep->d_name);
//	      printf ("dir: %s\n",swin.ddown[n].str);
	      n++;
	   }
      }
      
      (void) closedir (dp);
   } else
     perror("Could not open directory");
   return(n);
}

int wcb_dropdown(Widget *w, int x, int y, int m) {
   
      printf("SETUP: wcb_dropdown\n");

   alert_button=y;
   while(mouse_b!=0) poll_mouse();
   close_window();
}

int wcb_openselect(Widget *w, int x, int y, int m) {
   int i,ni,dx,dw,dy,dh,li=-1,ci=-1,si=0;
   char wvar[90], wvar2[30], *homet;
   Widget *nw;
   
      printf("SETUP: wcb_openselect\n");
   //   printf("Now what?\n");
   for(i=0;i<16;i++) {
#ifdef DEBUG
      printf("wx:%d,  bx:%d,  wy:%d,  by:%d\n",swin.widget[i].x+14+(strlen(swin.widget[i].text)*8),(w->x1-swin.startx)-2,swin.widget[i].y,(w->y1-7)-swin.starty);
      printf("wt:%s\n",swin.widget[i].text);
#endif
      dx=swin.widget[i].x+14;
      dw=strlen(swin.widget[i].text)*8;
      dy=swin.widget[i].y;
      if(dx+dw == (w->x1-swin.startx)-2 
	 && dy==(w->y1-7)-swin.starty) {
//	 printf("setup_mapget(wvar,%s,3)\n",swin.widget[i].text);
	 setup_mapget(wvar,swin.widget[i].text,3);
	 si=i;
	 break;
      }
      
   }
//   printf("found a match: %s->%s\n",swin.widget[i].text,wvar);
   if(strcmp(wvar,"findbin")==0) {
      setup_mapget(wvar,swin.widget[i].text,4);
      ni=setup_findbin(wvar);
   }
   if(strcmp(wvar,"tmplist")==0) {
      strcpy(swin.ddown[0].str,"-");
#ifdef WIN32
      strcpy(wvar,"C:\\temp");
#else
      strcpy(wvar,"/tmp");
#endif
      strcpy(swin.ddown[1].str,wvar);
#ifdef WIN32
      strcpy(wvar,"C:\\emulator\\tmp");
#else
      homet=(char *)getenv("HOME");
      sprintf(wvar,"%s/.emulator/tmp",homet);
#endif
      strcpy(swin.ddown[2].str,wvar);
      ni=3;
   }
   if(strcmp(wvar,"fixedlist")==0) {
      setup_mapget(wvar,swin.widget[i].text,4);
      printf("WWH: %s\n",wvar);
      ni=hss_count(wvar,',')+1;
      strcpy(swin.ddown[0].str,"-");
      for(i=0;i<ni;i++) {
	 hss_index(wvar2,wvar,i,',');
	 strcpy(swin.ddown[i+1].str,wvar2);
      }
   }
   dx=dx+swin.startx-6;
   dy=dy+swin.starty+16;
   dw=dw+4;
   dh=(16*ni)+2;
#ifdef DEBUG
   printf("ni=%d\n",ni);
   printf("dx=%d\n",dx);
   printf("dy=%d\n",dy);
   printf("width=%d\n",dw);
   printf("height=%d\n",dh);
#endif
   new_window(dx,dy,dx+dw,dy+dh);
   rectfill(screen,dx,dy,dx+dw,dy+dh,makecol(224,224,224));
   rect(screen,dx,dy,dx+dw,dy+dh,makecol(0,0,0));
   for(i=0;i<ni;i++) {
//      printf("%d:%s\n",i,swin.ddown[i].str);
      nw=add_invisible_button(dx,dy+1+(i*16),dx+dw,dy+17+(i*16),&wcb_dropdown);
      if(i==0) wdg_bind_key(nw,KEY_ESC,-1,0);
      limit_str(wvar2,swin.ddown[i].str,dw/8);
//      fnt_print_string(screen,dx+2,dy+1+(i*16),swin.ddown[i].str,makecol(0,0,0),-1,-1);
      fnt_print_string(screen,dx+2,dy+1+(i*16),wvar2,makecol(0,0,0),-1,-1);
   }
   alert_button=0;
   while(alert_button==0) {
      // Hilight dropdown item under mouse
      if(mouse_x>dx && mouse_x<dx+dw && mouse_y>dy && mouse_y<(dy+dh)-4) {
	 ci=(mouse_y-dy-2)/16;
	 if(ci!=li) {
	    if(li>-1) {
	       rectfill(screen,dx+1,dy+1+(li*16),dx+dw-1,dy+17+(li*16),makecol(224,224,224));
	       limit_str(wvar2,swin.ddown[li].str,dw/8);
	       fnt_print_string(screen,dx+2,dy+1+(li*16),wvar2,makecol(0,0,0),-1,-1);	       
	    }
	    rectfill(screen,dx,dy+1+(ci*16),dx+dw,dy+17+(ci*16),makecol(0,0,0));
	    limit_str(wvar2,swin.ddown[ci].str,dw/8);
	    fnt_print_string(screen,dx+2,dy+1+(ci*16),wvar2,makecol(224,224,224),-1,-1);	       
	    li=ci;
	 }
      }
      s2a_flip(screen);
      event_loop(JUST_ONCE);
      kbd_loop(JUST_ONCE);
   }
//   printf("\nabr: %d\n",ci);
   if(ci>0) {
      strcpy(swin.widget[si].evalue,swin.ddown[ci].str);
      (*(swin.widget[si].widget->handler))(swin.widget[si].widget,-1,-1,-1);
   }
   
   
//   pop_level();
}

int wcb_checkcb(Widget *w, int x, int y, int m) {
   int i,tx,ty;
   char wvar[20];
   
      printf("SETUP: wcb_checkcb\n");
//   printf("CHECK CALLBACK!\n");
   for(i=0;i<swin.numwidget;i++) {
      if(swin.widget[i].widget==w)
	break;
   }
   
   setup_mapget(wvar,swin.widget[i].text,4);
   swin.widget[i].value=atoi(w->text);
   hss_index(swin.widget[i].evalue,wvar,swin.widget[i].value,',');
   
//   printf("pressed: %d-->%s\n",swin.widget[i].value,wvar);
//   printf("pressed: %s now %s\n",swin.widget[i].text,swin.widget[i].evalue);
}

int gfx_windecor(int x, int y, int width, int height,int base) {
   int bcr,bcg,bcb,wh,wl,ir,ig,ib;
   
   printf("SETUP: gfx_windecor\n");

   bcr=getr(base); bcg=getg(base); bcb=getg(base); 
   wl=makecol(bcr/4,bcg/4,bcb/4);
   ir=bcr+80; if(ir>255) ir=255;
   ig=bcg+80; if(ig>255) ig=255;
   ib=bcb+80; if(ib>255) ib=255;
   wh=makecol(ir,ig,ib);
//   rectfill(screen,x,y,x+width,y+height,base); // Paint bg
   rectfill(screen,x+4,y+4,x+width-4,y+14,makecol(32,32,120));
   hline(screen,x,y,x+width,wh);
   vline(screen,x,y,y+height,wh);
   hline(screen,x,y+height,x+width,wl);
   vline(screen,x+width,y,y+height,wl);
   // End Decor
}

int wcb_ab(Widget *w, int x, int y, int m) {
   // Alert Button Callback

   printf("SETUP: wcb_ab\n");

   alert_button=3;
   if(w->ksym==KEY_Y || w->ksym==KEY_ENTER)
     alert_button=1;
   if(w->ksym==KEY_N)
     alert_button=2;
   close_window();
}

int gfx_alert(char *text,int k1, char *b1,int k2, char *b2,int k3,char *b3) {
   // Taken from widget lib,  to be more customizable
   
   Widget* nw;
   int width,height,x,y,mbw,bw,toff,tyoff;
#ifdef GFXBITMAP
   bmpbtn_t mybutton;
#endif

   printf("SETUP: gfx_alert\n");

   mbw=40;
   alert_button=0;
   
   if(b1) {
      bw=calc_width(b1);
      if(bw > mbw) mbw=bw;
   }
   if(b2) {
      bw=calc_width(b2);
      if(bw > mbw) mbw=bw;
   }
   if(b3) {
      bw=calc_width(b3);
      if(bw > mbw) mbw=bw;
   }
   
   mbw+=2;
   width=calc_width(text);
   height=calc_height(text);
#ifdef GFXBITMAP
   width+=48;
   toff=40;
   tyoff=4;
   height+=40;
#else
   width+=16;
   toff=0;
   tyoff=0;
   height+=32;
#endif
   if (width < 3*mbw+6) width=3*mbw+6;
   height+=32; // For Banner
   x=(VIRTUAL_W-width)/2;
   y=(VIRTUAL_H-height)/2;
   new_window(x,y,x+width,y+height);
#ifdef GFXBITMAP
   gfx_windecor(x,y,width,height,makecol(182,170,170));
#else
   gfx_windecor(x,y,width,height,makecol(128,128,128));
#endif
   if(b1) {
#ifdef GFXBITMAP
     mybutton.btnupbmp=widgetbmp;
      mybutton.btndnbmp=widgetbmp;
      mybutton.btnhlbmp=NULL;
      if(strcmp(b1,"OK")==0) {
	 mybutton.up_x1=0;mybutton.up_y1=139;mybutton.up_x2=41;mybutton.up_y2=19;
	 mybutton.dn_x1=41;mybutton.dn_y1=139;mybutton.dn_x2=41;mybutton.dn_y2=19;
      } else {
	 mybutton.up_x1=0;mybutton.up_y1=120;mybutton.up_x2=41;mybutton.up_y2=19;
	 mybutton.dn_x1=41;mybutton.dn_y1=120;mybutton.dn_x2=41;mybutton.dn_y2=19;
      }
      nw=add_bmp_button(x+8,y+height-21,x+49,y+height-2,mybutton,&wcb_ab);
#else
      nw=add_button(x+8,y+height-20,x+mbw,y+height-4,b1,&wcb_ab);
#endif
      wdg_bind_key(nw,k1,-1,0);
   }
   if(b2) {
#ifdef GFXBITMAP
      mybutton.up_x1=82;mybutton.up_y1=120;mybutton.up_x2=41;mybutton.up_y2=19;
      mybutton.dn_x1=123;mybutton.dn_y1=120;mybutton.dn_x2=41;mybutton.dn_y2=19;
      nw=add_bmp_button(x+51,y+height-21,x+51+41,y+height-2,mybutton,&wcb_ab);
#else
      nw=add_button(x+mbw+2,y+height-20,x+2*mbw+2,y+height-4,b2,&wcb_ab);
#endif
      wdg_bind_key(nw,k2,-1,0);
   }
   if(b3) {
     nw=add_button(x+2*mbw+4,y+height-20,x+3*mbw+4,y+height-4,b3,&wcb_ab);
     wdg_bind_key(nw,k3,-1,0);
   }
   fnt_print_string(screen,x+8+toff,y+24+tyoff,text,makecol(0,0,0),-1,-1);
#ifdef GFXBITMAP
   masked_blit(widgetbmp,screen,64,0,x+8,y+17+tyoff,32,32);
#endif
   s2a_flip(screen);
   while(alert_button==0) {
      event_loop(JUST_ONCE);
      kbd_loop(JUST_ONCE);
   }
   return alert_button;
}

int setup_parse_line(char *sline, int lno) {
   int i,wxs,bc=0;
   char wstr[20];
   
   printf("SETUP: setup_parse_line\n");

   for(i=0;i<strlen(sline);i++) {
      if(sline[i]=='|') sline[i]=' ';
      if(sline[i]==10) sline[i]=0;
      if(sline[i]==13) sline[i]=0;
      if(bc==1) {
//	 printf("ggg: %d\n",i-(wxs+1));
	 wstr[i-(wxs+1)]=sline[i];
	 if(wstr[i-(wxs+1)]=='_') wstr[i-(wxs+1)]=' ';
      }
     if((sline[i]=='%' || sline[i]=='@' || sline[i]=='$'
	 || sline[i]=='*' || sline[i]=='^') && bc==0) { 
	bc=1; wxs=i;
	swin.widget[swin.numwidget].y=((lno+1)*16);
	swin.widget[swin.numwidget].x=((i)*8);
	swin.widget[swin.numwidget].value=0;
	if(sline[i]=='%') swin.widget[swin.numwidget].type=SU_BUTTON;
	if(sline[i]=='@') swin.widget[swin.numwidget].type=SU_CHECKBOX;
	if(sline[i]=='$') swin.widget[swin.numwidget].type=SU_ENVTEXT;
	if(sline[i]=='*') swin.widget[swin.numwidget].type=SU_EDIT;
	if(sline[i]=='^') swin.widget[swin.numwidget].type=SU_SELECT;
      }
     if(sline[i]==' ' && bc==1) { 
	bc=0;
	wstr[i-(wxs+1)]=0;
#ifdef DEBUG
	printf("widget text: %s widget num:%d\n",wstr,swin.numwidget);
#endif
	strcpy(swin.widget[swin.numwidget].text,wstr); // button
	swin.numwidget++;
      }
      if(bc==1) sline[i]=' ';
    }
}

// setup essentially starts here

int setup_getlocalcfgname(char *cfgname) {
   char *homet;

   printf("SETUP: setup_getlocalcfgname\n");

#ifdef WIN32
   strcpy(cfgname,"C:\\emulator\\emucd.env");
#else
   homet=(char *)getenv("HOME");
   sprintf(cfgname,"%s/.emulator/emucd.env",homet);
#endif
}

int setup_getflashcfgname(char *cfgname) {
//   char *homet;

   printf("SETUP: setup_getflashcfgname\n");
   sprintf(cfgname,"%s%cuser_config%ccfg1%cemucd.env",basedir,mysep,mysep,mysep);
   printf("SETUP: cfgname=%s\n", cfgname);
}

int setup_getglobalcfgname(char *cfgname) {
   // this could be used to fix the crash bug when
   // emufe -n -i is used without CDROOT
   printf("SETUP: setup_getglobalcfgname\n");
   sprintf(cfgname,"%s%cetc%cemucd.env",basedir,mysep,mysep);
}

int setup_envcclr() {
   envcidx=0;
}

int setup_envprint() {
   int i;
   printf("----------------------------------\n");
   printf(" Printout of envchange\n");
   for (i=0;i<envcidx;i++) 
     printf("e:%s=%s\n",envc[i].var,envc[i].value);
	
   printf("----------------------------------\n");
}

int setup_envset(const char *var, const char *val) {
   int i,cc=0;
   //   printf("%s==%s\n",var,val);

   printf("SETUP: setup_envset:  var=%s  val=%s\n",var,val);
   // printf("SETUP: setup_envset:  envcidx=%d  MAX_ENVC=%d\n",envcidx,MAX_ENVC);

   for(i=0;i<envcidx;i++) {
      // printf(" envc[%d]: %s=%s\n",i,envc[i].var,envc[i].value);
      if(strcmp(envc[i].var,var)==0) {
	 strcpy(envc[i].value,val);
	 cc=1;
	 printf("set_env: replaced var\n");
      }
   }
   if(cc==0) {
      if(envcidx>=MAX_ENVC) {
	 printf("ERROR: Environment space exhausted, increase MAX_ENVC & recompile\n");
      } else {
	 strcpy(envc[envcidx].var,var);
	 strcpy(envc[envcidx].value,val);
	 // printf("set_env: inserted  var %s=%s at %d\n",envc[envcidx].var,envc[envcidx].value,envcidx);
	 envcidx++;
      }
   }
}

setup_mapget(char *ret, const char *var, int pno) {
   int i;
   for(i=0;i<mapidx;i++) {
#ifdef DEBUG
      printf("%s--%s--\n",var,emap[i].var); 
#endif
      
      printf("SETUP: setup_mapget\n");

      if(strncmp(emap[i].var,var,18)==0) {
	 if(pno==2) strcpy(ret,emap[i].value);
	 if(pno==3) strcpy(ret,emap[i].p3);
	 if(pno==4) strcpy(ret,emap[i].p4);
      }      
   }
}

int setup_mapset(const char *var, const char *val,const char *p3, const char *p4) {
   int i,cc=0;

   printf("SETUP: setup_mapset\n");

   for(i=0;i<mapidx;i++) {
      if(strcmp(emap[i].var,var)==0) {
	 strcpy(emap[i].value,val);
	 strcpy(emap[i].p3,p3);
	 strcpy(emap[i].p4,p4);
	 cc=1;
	 // printf("set_env: replaced\n");
      }
   }
   if(cc==0) {
      if(mapidx>=MAX_ENVC) {
	 printf("ERROR: Map space exhausted, increase MAX_ENVC & recompile\n");
      } else {
	 strcpy(emap[mapidx].var,var);
	 strcpy(emap[mapidx].value,val);
	 strcpy(emap[mapidx].p3,p3);
	 strcpy(emap[mapidx].p4,p4);
#ifdef DEBUG
	 printf("set_map: added #%d\n",mapidx);
	 printf("var:%s  val:%s\n",emap[mapidx].var,emap[mapidx].value);
	 printf("var:%s  val:%s\n",var,val);
#endif
	 mapidx++;
      }
   }
}

int setup_envgetch(char *val, const char *var) {
   // get changes only
   int i, rv=0;
//   setup_envprint();
   printf("checking changes::%s::\n",var);
   for(i=0;i<envcidx;i++) {
//      printf("::%d::%d\n",i,envcidx);
      if(strcmp(envc[i].var,var)==0) {
	 strcpy(val,envc[i].value);
	 rv=1;
	 break;
//	 return 0;
	  printf("set_env: %s replaced to %s\n",var,val);
      }
   }
   return rv;
}

int setup_envget(char* ival, const char *var) {
   int i;
   char *homet, emuenv[160],lc[246],tvl[160];
   FILE *fp;
   
//   printf("SETUP: setup_envget\n");
//	 printf("TS2addr:%p\n",tvl);

   if(setup_envgetch(ival, var)==1)
     return 0;
   
//   printf("checking changes\n");
//   for(i=0;i<envcidx;i++) {
//      if(strcmp(envc[i].var,var)==0) {
//	 strcpy(ival,envc[i].value);
//	 return 0;
//	 // printf("set_env: replaced\n");
//      }
//   }
//   setup_getlocalcfgname(lc);
   setup_getflashcfgname(lc);
   if(file_exists(lc)) 
     strcpy(emuenv,lc);
   else
     setup_getglobalcfgname(emuenv);
   printf("checking disk:%s\n",emuenv);
   fp=fopen(emuenv,"rb");
   while(!feof(fp)) {
      fgets(lc,240,fp);
//      printf("::%s\n",lc);
#ifdef WIN32
      // remove ^M linefeeds
      for(i=0;i<strlen(lc);i++) {
	 if(lc[i]==13)
	   lc[i]=0;
      }
#endif
//	 printf("TS3addr:%p\n",tvl);
      hss_index(tvl,lc,0,'=');
//      printf("%s\n",tvl);
//      printf("%s<->%s\n",tvl,var);
      if(strcmp(tvl,var)==0) {
//	       printf("matched %s<->%s\n",tvl,var);
//	 printf("%d:len:\n",strlen(lc));

//	 printf("TS3addr:%p\n",ival);
	 hss_index(ival,lc,1,'=');
//	 printf("ivalL:%s\n",ival);
//	 hss_index(ival,wwww,1,'"');
//	 sprintf(ival,"%s",wwww);
//	 printf("VALL:%s\n",ival);
	 return 0;
      }
//      printf("NEXT\n");
   }
   fclose(fp);
}

int setup_menv(char *val,const char *var) {
   int i;
   
   printf("SETUP: setup_menv\n");
   strcpy(val,"");
   for(i=0;i<mapidx;i++) {
//      printf("%d\n",i);
#ifdef DEBUG
      printf("coo:%s<-->%s\n",emap[i].var,emap[i].value);
#endif
      if(strcmp(emap[i].var,var)==0) {
	 strcpy(val,emap[i].value);
	 break;
      }
   }
}

int setup_menvget(char *val, const char *var) {
   int i;
   char ival[40];

   printf("SETUP: setup_menvget\n");
   setup_menv(ival,var);
//   printf("ival is %s\n",ival);

   if(strcmp(ival,"")!=0) {
      setup_envget(val,ival);
      if(val[0]=='\"') {
	 hss_index(ival,val,1,'\"');
	 strcpy(val,ival);
      }
   }
}

int load_setup_fl(char *fname) {
   FILE *fp;
   char lb[200],v1[40],v2[40],v3[30],v4[80];
   int sw=0,nl=0,i;

   printf("SETUP: load_setup_fl\n");

   if(fp=fopen(fname,"rb")) {
      while(!feof(fp)) {
	 fgets(lb,195,fp);
#ifdef WIN32
      // remove ^M linefeeds
      for(i=0;i<strlen(lb);i++) {
	 if(lb[i]==13)
	   lb[i]=0;
      }
#endif
	 if(feof(fp)) break;
//	 printf("%s",lb);
	 if(lb[0]=='|' && sw==1) {
	    strcpy(swin.line[nl].str,lb);
	    nl++;
	 }
	 if(sw==2) {
	    hss_index(v1,lb,0,'|');
	    hss_index(v2,lb,1,'|');
	    hss_index(v3,lb,2,'|');
//	    printf("v3:%s\nv4:%s\n",v3,v4);
	    hss_index(v4,lb,3,'|');
//	    printf("v1:%s\nv2:%s\n",v1,v2);
//	    printf("v3:%s\nv4:%s\n",v3,v4);
	    setup_mapset(v1,v2,v3,v4);
	 }
	 
	 if(strncmp(lb,"=VARIABLE",9)==0)
	   sw=2;
	 if(strncmp(lb,"=END VARIABLE",13)==0)
	   sw=0;
	 
	 if(lb[0]=='+' && lb[1]=='-') { 
	    if(sw==0) {
	       swin.width=(strlen(lb)*8);
	       sw=1;
	    } else {
	       sw=0;
	       swin.height=(nl+2)*16;
	    }
	 }
      }
      swin.numlines=nl;
      fclose(fp);
      return 1;
   } else {
      printf("Can't Open %s\n",fname);
      return 0;
   }
}

int setup_mystyle() {
   printf("SETUP: setup_mystyle\n");

   // Override some of the widget default styles
   activestyle.input.bg=makecol(240,240,240);
}

int run_setup(char *n_setup) {
   char fname[120],envval[60],wst[20];
   int sx,sy,i,sxo,syo,l;
   Widget *nw;

//   fnt_print_string(screen,10,532,"run_setup()   ",makecol(255,255,25),makecol(50,30,0),-1);
   printf("in--setup: %s\n",n_setup);
   wdgst_default();
   setup_mystyle(); 
//   font_set_builtin();
//   fnt_setactive(cf8x16);
   
   setup_envget(fname,"VERSION");
#ifdef DEBUG
   printf("=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n");
   printf("testing env read\n");
   printf("VERSION is %s\n",fname);
   printf("=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n");
#endif
   
//   l=gfx_alert("Are You Sure You Want This?",KEY_ENTER,"OK",-1,NULL,-1,NULL);
   swin.width=swin.height=swin.numwidget=0;
//   sprintf(fname,"c:\\cygwin\\usr\\src\\emufe-2.5.3\\src\\%s.def",n_setup);
   sprintf(fname,"%s%cset-up%c%s.def",basedir,mysep,mysep,n_setup);
   printf("Gloading %s\n",fname);
   if(!load_setup_fl(fname))
     return 0;
   printf("SETUP: run_setup()  file loaded: %s\n", fname);
//   printf("Width:%d  Height:%d\n",swin.width,swin.height);

   if(swin.width>0 && swin.height>0) {
      widget_init();
      widget_clear_level();
      sx=(usex-swin.width)/2;
      sy=(usey-swin.height)/2;
      swin.startx=sx;
      swin.starty=sy;
      new_window(sx,sy,sx+swin.width,sy+swin.height);
      printf("New WINdow: %d,%d %d,%d\n", sx, sy, sx+swin.width, sy+swin.height); 
      gfx_windecor(sx,sy,swin.width,swin.height,makecol(182,170,170));
      
      for(i=0;i<swin.numlines;i++) {
	 setup_parse_line(swin.line[i].str,i);
	 fnt_print_string(screen,sx,sy+((i+1)*16),swin.line[i].str,makecol(240,240,240),-1,-1);
	 printf("-> %s\n",swin.line[i].str);
      }
      // render buttons
      for(i=0;i<swin.numwidget;i++) {
	 sxo=sx+swin.widget[i].x;
	 syo=sy+swin.widget[i].y;
	 if(swin.widget[i].type==SU_BUTTON) {
	    l=sxo+(strlen(swin.widget[i].text)*8)+8;
	    nw=add_button(sxo,syo,l,syo+20,swin.widget[i].text,&wcb_buttoncb);
	    if(strcmp(swin.widget[i].text,"ACCEPT")==0)
	      wdg_bind_key(nw,KEY_ENTER,-1,0);
	    if(strcmp(swin.widget[i].text,"CANCEL")==0)
	      wdg_bind_key(nw,KEY_ESC,-1,0);
	 }
//	 printf("widgeta %d  type: %d\n",i,swin.widget[i].type);
	 if(swin.widget[i].type==SU_CHECKBOX) {
//	    printf("chh: >%s<\n",swin.widget[i].text);
	    setup_menvget(swin.widget[i].evalue,swin.widget[i].text);
	    setup_mapget(fname,swin.widget[i].text,4);
	    hss_index(wst,fname,1,',');
//	    printf("chc: >%s<\n",fname);
	    if(strcmp(wst,swin.widget[i].evalue)==0)
	      nw=add_checkbox(sxo,syo+4,1,&wcb_checkcb);
	    else
	      nw=add_checkbox(sxo,syo+4,0,&wcb_checkcb);
	    swin.widget[i].widget=nw;
//	       (*(nw->handler))(nw,-1,-1,-1);
	    fnt_print_string(screen,sxo+8,syo,swin.widget[i].text,makecol(0,0,0),-1,-1);
	 }
	 if(swin.widget[i].type==SU_ENVTEXT) {
//	    printf("wth: >%s<\n",swin.widget[i].text);
	    setup_menvget(envval,swin.widget[i].text);
//	    printf("asd: >%s<\n",envval);
	    fnt_print_string(screen,sxo+8,syo,envval,makecol(0,0,0),-1,-1);
	 }
	 if(swin.widget[i].type==SU_SELECT || swin.widget[i].type==SU_EDIT) {
//	    printf("wth: >%s<\n",swin.widget[i].text);
	    strcpy(swin.widget[i].evalue,"n/a");
	    setup_menvget(swin.widget[i].evalue,swin.widget[i].text);
//	    printf("asd: >%s<\n",swin.widget[i].evalue);
//	    fnt_print_string(screen,sxo+8,syo,envval,makecol(0,0,0),-1,-1);
	    nw=add_input (sxo+8,syo,strlen(swin.widget[i].text),swin.widget[i].evalue);
	    swin.widget[i].widget=nw;
	    if(swin.widget[i].type==SU_SELECT) {
	       // need to bind this to the select, somehow
	       nw=add_arrow_button(sxo+22+(strlen(swin.widget[i].text)*8),syo+14,DOWN,&wcb_openselect);
	    }
	 }
      }
      swin.active=1;
      // Execute Window
#ifdef DEBUG
      printf("about to loop\n");
#endif
      while(swin.active==1) {
	 s2a_flip(screen);
	 event_loop(JUST_ONCE);
	 kbd_loop(JUST_ONCE);
      }
#ifdef DEBUG
      printf("About to POP\n");
#endif
//      close_window();
//     Another pop_level problem, what a surprise!
      pop_level();
      s2a_flip(screen);
#ifdef DEBUG
      printf("swin.active = %d\n",swin.active);
#endif
      if(swin.active & P_SAVE) { 
#ifdef DEBUG
	 printf("Saving...\n");
#endif
	 for(i=0;i<swin.numwidget;i++) {
	    if(swin.widget[i].type==SU_SELECT || swin.widget[i].type==SU_EDIT 
	       || swin.widget[i].type==SU_CHECKBOX) {
	       setup_menvget(envval,swin.widget[i].text);
	       if(strcmp(envval,swin.widget[i].evalue)!=0) {
		  setup_menv(envval,swin.widget[i].text);
#ifdef DEBUG
		  printf("CHAnge DeTeCtEd: %s<>%s\n",envval,swin.widget[i].evalue);
		  printf("%s=%s\n",envval,swin.widget[i].evalue);
#endif
		  setup_envset(envval,swin.widget[i].evalue);
	       }
	    }
	 }	
#ifdef DEBUG
	 setup_envprint();
#endif
      } // swin.active & P_SAVE
   }
   //   font_unset_builtin();
   fnt_setactive(DefaultFont);
}

int setup_spvar(const char *var) {
   char p1[22],p2[12];

   printf("SETUP: setup_spvar\n");

   hss_index(p2,var,1,'_');

   if(strcmp(p2,"dir")==0 || strcmp(p2,"bin")==0 || strcmp(p2,"desc")==0)
     return 0;

   if(setup_envgetch(p1,var)==1) 
	return 1;
//   printf("huh? getvar: %s\n",var);
   return 0;
}

int setup_advoptlst(char *vlist, const char *var) {
   int i;
   char p1[16],ipv[16];
//   printf("checking changes\n");

   printf("SETUP: setup_advoptlst\n");

   strcpy(vlist,"");
   for(i=0;i<envcidx;i++) {
      hss_index(ipv,var,0,'_');
      hss_index(p1,envc[i].var,0,'_');
      if(strcmp(p1,ipv)==0) {
	 strcat(vlist,envc[i].var);
	 strcat(vlist," ");
      }
   }
}

int setup_hderr(const char *msg1, const char *msg2,const char *msg3) {

   printf("SETUP: setup_hderr\n");

   setup_envcclr();
   setup_envset("instmsg1", msg1);
   setup_envset("instmsg2", msg2);
   setup_envset("instmsg3", msg3);
   run_setup("installhderr");
}

int setup_hd(const char *glo, const char *loc, const char *type) {
   printf("SETUP: setup_hd\n");

   setup_envcclr();
//   printf("CXHE\n");
   if(strcmp(type,"hdi")==0) {
      setup_envset("hddesc", "Hard Disk Image Installation");
      setup_envset("instmsg", "  This will install the following HD image");
      setup_envset("hdloc", loc);
   }
   if(strcmp(type,"hdd")==0) {
      setup_envset("hddesc", " Hard Disk Dir Installation");
      setup_envset("instmsg", "This will install the following HD directory");
      setup_envset("hdloc", loc);
   }
   
   run_setup("installhd");
   if(swin.lastbutton != B_CANCEL) 
     return 1;
   else
     return 0;
}

int setup_go() {
   // Entry point
   char lc[120], gc[160],val[120],dlist[220],var[30],v1[15],v2[15],v3[90];
   int didx=0,i,sflag, iflag=0;
   FILE *fp, *fpw;

//#ifdef SDL2
//   SA_AUTOUPDATE=0;
//#endif
   printf("SETUP: setup_go\n");
//   setup_getlocalcfgname(lc);
   setup_getflashcfgname(lc);
   if(file_exists(lc)) 
     setup_envset("int_insttype","Update Configuration");
   else
     setup_envset("int_insttype","(Re)Install Configuration");
     run_setup("start");
   if(swin.lastbutton==B_CANCEL)
     return 0;
   setup_envget(val,"int_insttype");
   if(strcmp(val,"(Re)Install Configuration")==0) {
      iflag=1;
   }
   
   printf("int_insttype:%s\n",val);
   setup_envget(gc,"syslist");
   hss_index(val,gc,1,'"');
    sprintf(dlist,"main mess %s end",val);
   printf("dlist:%s\n",dlist);
   do {
//      swin.lastbutton=B_NULL;
      hss_index(val,dlist,didx,' ');
      printf("I AM GOING TO: %s\n",val);
      run_setup(val);
      s2a_flip(screen);
      if(swin.lastbutton==B_ADV) {
	 sprintf(var,"%s_adv",val);
	 run_setup(var);
	 swin.lastbutton=B_NULL;
      }
      if(swin.lastbutton==B_NEXT) didx++;
      if(swin.lastbutton==B_PREV) didx--;
   } while (swin.lastbutton != B_CANCEL && swin.lastbutton != B_WRITE);
   if(swin.lastbutton==B_WRITE) {
#ifdef DEBUG
      printf("The following changes were made:\n");
      setup_envprint();
#endif
      if(iflag==1) {
	 setup_getglobalcfgname(gc);
//	 printf("DOIT: Installing %s to %s\n",gc,lc);
	 fileio_dirname(v3,lc);
//	 printf("DOIT: mkdir %s\n",v3);
	 fileio_mkdir_p(v3);
	 fileio_cp(gc,lc);
      }
#ifdef DEBUG
      printf("opening %s\n",lc);
      printf("writing to %s\n",gc);
#endif
      sprintf(gc,"%s2",lc);
      if(fp=fopen(lc,"rb")) {
	 fpw=fopen(gc,"wb");
	 while(!feof(fp)) {
	    fgets(dlist,195,fp);
	    if(feof(fp)) break;
	    if(dlist[0]!='#') {
	       hss_index(var,dlist,0,'=');
	       // need to handle the 'advanced' options
	       hss_index(v1,var,1,'_');
	       // This was originally 'bin', change it back
	       // if problems result
	       if(strcmp(v1,"desc")==0) {
//		  hss_index(v2,var,0,'_');
		  setup_advoptlst(val,var);
		  if(hss_count(val,' ')>1) {
#ifdef DEBUG
		     printf("Advanced options: %s\n",val);
#endif
		     for(i=0;i<(hss_count(val,' ')-1);i++) {
			hss_index(v2,val,i,' ');
			setup_envgetch(v1,v2);
			if(strcmp(v1,"off")!=0 && strcmp(v1,"n/a")!=0) {
			   sprintf(v3,"%s=%s\n",v2,v1);   
			   fputs(v3,fpw);
#ifdef DEBUG
			   printf("%s",v3);
#endif
			} // if
		     } //for
		  } // if hss_count
	       } // if bin
	       if(strcmp(v1,"")!=0)
		 sflag=setup_spvar(var);
	       else
		 sflag=0; 
#ifdef DEBUG
	       printf("SFLAG:%d  %s\n",sflag,var);
#endif
	       if(setup_envgetch(val,var)==1)
		 sprintf(dlist,"%s=%s\n",var,val);
	    }
//	    printf("%s",dlist);
	    if(sflag==0)
	      fputs(dlist,fpw);
	 }
	 fclose(fpw);
	 fclose(fp);
//	 printf("MMV: %s,%s\n",gc,lc);
	 fileio_mv(gc,lc);
      }
   }
//#ifdef SDL2
//   SA_AUTOUPDATE=1;
//#endif
}

int setup_test() {
   // something is screwy
}
