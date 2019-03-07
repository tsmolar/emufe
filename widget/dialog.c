#include <stdio.h>
#include <string.h>
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
#include <button.h>

#define SU_NULL 0
#define SU_BUTTON 1
#define SU_BMPBUTTON 8
#define SU_CHECKBOX 2
#define SU_ENVTEXT 3
#define SU_SELECT 4
#define SU_PLUSMINUS 7
#define SU_EDIT 5
#define SU_BITMAP 6

#define MAX_ENVC 50
#define P_CANCEL 0
#define P_ACTIVE 1
#define P_SAVE 2
#define P_WRITE 4   // is this used?  (Apply button?  Save but don't close?)

#define B_NULL 0
#define B_CANCEL 1
#define B_WRITE 2
#define B_NEXT 4
#define B_PREV 8
#define B_ADV 16

// For storing internal dialog info
typedef struct dlg_t {
   char globalenv[120];
   char localenv[120];
   fnt_t* fontsave;
} dlg_t;

typedef struct env_t {
   char var[20];
   char value[80];
} env_t;

typedef struct charar_t {
   char str[85];
} charar_t;

typedef struct map_t {
   char var[40];
   char value[20];
   char p3[30];
   char p4[80];
} map_t;

typedef struct widgetlist_t {
   int type;
   int x,y;
   int w,h;
   char text[40];
   char evalue[40];  // store env value
   char etrig[40];   // env var to trigger a change here
   int value;
   Widget *widget;
} widgetlist_t;

typedef struct setwin_t {
   int width,height,numlines,numwidget;
   int startx,starty;
   int active;
   int lastbutton;
   int type;   // fixed or floating
   Widget *pw;
   BITMAP *bm;  // where to draw to
   charar_t line[80];
   charar_t ddown[80];
   widgetlist_t widget[40];
} setwin_t;

extern char mysep;
extern char datadir[200];
setwin_t swin;
int envcidx,mapidx;
env_t envc[MAX_ENVC];
map_t emap[MAX_ENVC];
dlg_t dinf;
char fdlg_lastoper[8];
int fdlg_active;

PALETTE dp;
// BITMAP* drawbmp;
BITMAP* dbitmap;

int dialog_buttoncb(Widget *w, int x, int y, int m) {
//  printf("CALLBACK!-> %s\n",w->text);
   swin.active=P_ACTIVE;
   swin.lastbutton=B_NULL;
   // This has to change
   if(strcmp(w->text,"ACCEPT")==0 || strcmp(w->text," SAVE ")==0)
      swin.lastbutton=B_WRITE;
   if(strcmp(w->text,"NEXT->")==0 || strcmp(w->text,"NEXT >")==0)
     swin.lastbutton=B_NEXT;
   if(strcmp(w->text,"<-BACK")==0 || strcmp(w->text,"< BACK")==0)
     swin.lastbutton=B_PREV;
   if(strcmp(w->text,"ADVANCED")==0 || strcmp(w->text,"Advanced")==0 )
     swin.lastbutton=B_ADV;

   if(strcmp(w->id,"BWRT")==0) swin.lastbutton=B_WRITE;
   if(strcmp(w->id,"BNXT")==0) swin.lastbutton=B_NEXT;
   if(strcmp(w->id,"BACK")==0) swin.lastbutton=B_PREV;
   if(strcmp(w->id,"BADV")==0) swin.lastbutton=B_ADV;
   if(strcmp(w->id,"BCAN")==0) swin.lastbutton=B_CANCEL;
   
   if(swin.lastbutton==B_NEXT || swin.lastbutton==B_PREV ||
      swin.lastbutton==B_ADV || swin.lastbutton==B_WRITE)
     swin.active=(P_SAVE);
   if(swin.lastbutton==B_CANCEL)
     swin.active=(P_CANCEL);
   fdlg_active=swin.active;
   strcpy(fdlg_lastoper,w->id);
//   printf("DUN WIF CALLBACK\n");
//   So we pop afterwards, do we need to close here?
//   if(swin.active==P_CANCEL || swin.active==P_SAVE) {
//      if(w->parent!=NULL)
//	wdg_window_close(w->parent);
//      else
//	pop_level();
//   }
   printf("still here\n");
} // dialog_buttoncb()


int dialog_checkcb(Widget *w, int x, int y, int m) {
   int i,tx,ty;
   char wvar[20];
   
//   printf("CHECK CALLBACK!\n");
   for(i=0;i<swin.numwidget;i++) {
      if(swin.widget[i].widget==w)
	break;
   }
   
   dialog_mapget(wvar,swin.widget[i].text,4);
   swin.widget[i].value=atoi(w->text);
   dss_index(swin.widget[i].evalue,wvar,swin.widget[i].value,',');
   
//   printf("pressed: %d-->%s\n",swin.widget[i].value,wvar);
//   printf("pressed: %s now %s\n",swin.widget[i].text,swin.widget[i].evalue);
} // dialog_buttoncb()

int dialog_dropdown(Widget *w, int x, int y, int m) {
   int i,cw;
   
   alert_button=y;
   while(mouse_b!=0) poll_mouse();
   if(swin.bm==screen)
     close_window();
} // dialog_dropdown()

int dialog_pmselect(Widget *w, int x, int y, int m) {
   // Callback for plus/minus buttons
   int i,dir=1,li;
   char wvar[90],evar[20];
   
   i=atoi(w->id);
   if(i<0) {
      dir=-1;
      i=i*-1;
   }
//   dialog_mapget(evar,swin.widget[i].text,2);
//   dialog_menvget(swin.widget[i].evalue,swin.widget[i].text);

   dialog_mapget(wvar,swin.widget[i].text,4);
   li=dss_getindex(wvar,swin.widget[i].evalue,',')+dir;
   printf("widgetselect1:%d wvar:%s\n",li,wvar);
   if(li<0) li=dss_count(wvar,',')-1;
   if(li>=dss_count(wvar,',')) li=0;
   dss_index(swin.widget[i].evalue,wvar,li,','); // get new value
   printf("widgetselect:%d dir:%d %s %s:%d\n",i,dir,wvar,swin.widget[i].evalue,li);
   swin.widget[i].value=li;
   dialog_mapget(evar,swin.widget[i].text,2);
   dialog_pvalchange(evar,swin.widget[i].evalue);
}

int dialog_openselect(Widget *w, int x, int y, int m) {
   int i,ni,dx,dw,dy,dh,li=-1,ci=-1,si=0;
   char wvar[90], wvar2[30], *homet;
   Widget *nw;
   
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
//	 printf("dialog_mapget(wvar,%s,3)\n",swin.widget[i].text);
	 dialog_mapget(wvar,swin.widget[i].text,3);
	 si=i;
	 break;
      }
      
   }
//   printf("found a match: %s->%s\n",swin.widget[i].text,wvar);
   //  Note: this was a special case for emufe that is unlikely to be needed
   //        for other applications.   It is kept here as an example of how
   //        to implement this kind of functionality, if needed.
   // 
   //   if(strcmp(wvar,"findbin")==0) {
   //      dialog_mapget(wvar,swin.widget[i].text,4);
   //   ni=setup_findbin(wvar);
   //}
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
      dialog_mapget(wvar,swin.widget[i].text,4);
      printf("WWH: %s\n",wvar);
      ni=dss_count(wvar,',')+1;
      strcpy(swin.ddown[0].str,"-");
      for(i=0;i<ni;i++) {
	 dss_index(wvar2,wvar,i,',');
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
   rectfill(swin.bm,dx,dy,dx+dw,dy+dh,makecol(224,224,224));
   rect(swin.bm,dx,dy,dx+dw,dy+dh,makecol(0,0,0));
   for(i=0;i<ni;i++) {
//      printf("%d:%s\n",i,swin.ddown[i].str);
      nw=add_invisible_button(dx,dy+1+(i*16),dx+dw,dy+17+(i*16),&dialog_dropdown);
      if(i==0) wdg_bind_key(nw,KEY_ESC,-1,0);
      dss_cutstr(wvar2,swin.ddown[i].str,0,dw/8);
//      fnt_print_string(swin.bm,dx+2,dy+1+(i*16),swin.ddown[i].str,makecol(0,0,0),-1,-1);
      fnt_print_string(swin.bm,dx+2,dy+1+(i*16),wvar2,makecol(0,0,0),-1,-1);
   }
   alert_button=0;
   while(alert_button==0) {
      // Hilight dropdown item under mouse
      if(mouse_x>dx && mouse_x<dx+dw && mouse_y>dy && mouse_y<(dy+dh)-4) {
	 ci=(mouse_y-dy-2)/16;
	 if(ci!=li) {
	    if(li>-1) {
	       rectfill(swin.bm,dx+1,dy+1+(li*16),dx+dw-1,dy+17+(li*16),makecol(224,224,224));
	       dss_cutstr(wvar2,swin.ddown[li].str,0,dw/8);
	       fnt_print_string(swin.bm,dx+2,dy+1+(li*16),wvar2,makecol(0,0,0),-1,-1);	       
	    }
	    rectfill(swin.bm,dx,dy+1+(ci*16),dx+dw,dy+17+(ci*16),makecol(0,0,0));
	    dss_cutstr(wvar2,swin.ddown[ci].str,0,dw/8);
	    fnt_print_string(swin.bm,dx+2,dy+1+(ci*16),wvar2,makecol(224,224,224),-1,-1);	       
	    li=ci;
	 }
      }
      rest(0);
      s2a_flip(screen);
      event_loop(JUST_ONCE);
      kbd_loop(JUST_ONCE);
   }
//   printf("\nabr: %d\n",ci);
   if(ci>0) {
      strcpy(swin.widget[si].evalue,swin.ddown[ci].str);
      printf("Changing %d to %s\n",si,swin.widget[si].evalue);      
      (*(swin.widget[si].widget->handler))(swin.widget[si].widget,-1,-1,-1);
      dialog_mapget(wvar2,swin.widget[si].text,2);
      dialog_pvalchange(wvar2,swin.widget[si].evalue);
   }
   
//   pop_level();
}

/* ***************************************************
 *       E N D   C A L L B A C K   S E C T I O N     *
 * ***************************************************/

void dialog_pvalchange(const char *evar,const char *newval) {
   int i;
   // This is to process changes to values that will cause bitmaps and
   // text fields to change.   
   for(i=0;i<swin.numwidget;i++) {
      if(swin.widget[i].type==SU_BITMAP && strcmp(swin.widget[i].etrig,evar)==0) 
	dialog_drawbmp(swin.bm,i,newval);
      if(swin.widget[i].type==SU_EDIT && strcmp(swin.widget[i].etrig,evar)==0)
	dialog_updent(i,newval);
   }
}

dialog_parse_line(char *sline, int lno) {
   int i,wxs,bc=0;
   char wstr[20];
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
	 || sline[i]=='*' || sline[i]=='^' || sline[i]=='~') && bc==0) { 
	bc=1; wxs=i;
	swin.widget[swin.numwidget].y=((lno+1)*16);
	swin.widget[swin.numwidget].x=((i)*8);
	swin.widget[swin.numwidget].value=0;
	if(sline[i]=='%') swin.widget[swin.numwidget].type=SU_BUTTON;
	if(sline[i]=='%' && sline[i+1]=='~') swin.widget[swin.numwidget].type=SU_BMPBUTTON;
	if(sline[i]=='@') swin.widget[swin.numwidget].type=SU_CHECKBOX;
	if(sline[i]=='$') swin.widget[swin.numwidget].type=SU_ENVTEXT;
	if(sline[i]=='*') swin.widget[swin.numwidget].type=SU_EDIT;
	if(sline[i]=='^') swin.widget[swin.numwidget].type=SU_SELECT;
	if(sline[i]=='^' && sline[i+1]=='+') swin.widget[swin.numwidget].type=SU_PLUSMINUS;
	if(sline[i]=='~') swin.widget[swin.numwidget].type=SU_BITMAP;
      }
     if(sline[i]==' ' && bc==1) { 
	bc=0;
	wstr[i-(wxs+1)]=0;
//#ifdef DEBUG
	printf("widget text: %s widget num:%d\n",wstr,swin.numwidget);
//#endif
	strcpy(swin.widget[swin.numwidget].text,wstr); // button
	// reserve an extra widget
	if(swin.widget[swin.numwidget].type==SU_PLUSMINUS) swin.numwidget++; 
	swin.numwidget++;
      }
      if(bc==1) sline[i]=' ';
   }
}

void dialog_windecor(BITMAP *wb, int x, int y, int width, int height,int base) {
   int bcr,bcg,bcb,wh,wl,ir,ig,ib;
   bcr=getr(base); bcg=getg(base); bcb=getg(base); 
   wl=makecol(bcr/4,bcg/4,bcb/4);
   ir=bcr+80; if(ir>255) ir=255;
   ig=bcg+80; if(ig>255) ig=255;
   ib=bcb+80; if(ib>255) ib=255;
   wh=makecol(ir,ig,ib);
//   rectfill(wb,x,y,x+width,y+height,base); // Paint bg
//   rectfill(wb,x+4,y+4,x+width-4,y+14,wh);
// windows title bar
   rectfill(wb,x+4,y+4,x+width-4,y+14,makecol(32,32,120));
//   rectfill(screen,x+4,y+4,x+width-4,y+14,makecol(32,32,120));
   hline(wb,x,y,x+width,wh);
   vline(wb,x,y,y+height,wh);
   hline(wb,x,y+height,x+width,wl);
   vline(wb,x+width,y,y+height,wl);
   printf("just decorated\n");
   // End Decor
} // dialog_windecor

int dialog_envget(char *val, const char *var) {
   // get in-memory env
   int i, rv=0;
//   printf("checking changes\n");
   for(i=0;i<envcidx;i++) {
      if(strcmp(envc[i].var,var)==0) {
	 strcpy(val,envc[i].value);
	 rv=1;
	 break;
      }
   }
   return rv;
}

int dialog_fenvget(char *val, const char *var) {
   int i;
   char *homet, envfile[160],lc[246],tvl[24];
   FILE *fp;
   if(dialog_envget(val, var)==1)
     return 0;

   strcpy(envfile,"");
   if(strcmp(dinf.globalenv,"")!=0)
     strcpy(envfile,dinf.globalenv);
   if(strcmp(dinf.localenv,"")!=0)
     strcpy(envfile,dinf.localenv);
   
   if(strcmp(envfile,"")!=0) {
      if (fp=fopen(envfile,"rb")) {
	 while(!feof(fp)) {
	    fgets(lc,240,fp);
#ifdef WIN32
	    // remove ^M linefeeds
	    for(i=0;i<strlen(lc);i++) {
	       if(lc[i]==13)
		 lc[i]=0;
	    }
#endif
	    dss_index(tvl,lc,0,'=');
	    if(strcmp(tvl,var)==0) {
	       dss_index(val,lc,1,'=');
	       return 0;
	    }
	 }
	 fclose(fp);
      } else 
	printf("Error:  Could not open %s\n",envfile);
   }
} // dialog_fenvget()

void dialog_envprint() {
   int i;
   printf("----------------------------------\n");
   printf(" Printout of envchange\n");
   for (i=0;i<envcidx;i++) 
     printf("e:%s=%s\n",envc[i].var,envc[i].value);
	
   printf("----------------------------------\n");
} // dialog_envpring()

dialog_envset(const char *var, const char *val) {
   int i,cc=0;
   //   printf("%s==%s\n",var,val);
   for(i=0;i<envcidx;i++) {
      if(strcmp(envc[i].var,var)==0) {
	 strcpy(envc[i].value,val);
	 cc=1;
	 // printf("set_env: replaced\n");
      }
   }
   if(cc==0) {
      if(envcidx>=MAX_ENVC) {
	 printf("ERROR: Environment space exhausted, increase MAX_ENVC & recompile\n");
      } else {
	 strcpy(envc[envcidx].var,var);
	 strcpy(envc[envcidx].value,val);
	 // printf("set_env: added #%d\n",envidx);
	 envcidx++;
      }
   }
} // dialog_envset()

void dialog_mapget(char *ret, const char *var, int pno) {
   int i;
   for(i=0;i<mapidx;i++) {
#ifdef DEBUG
      printf("%s--%s--\n",var,emap[i].var); 
#endif
//      printf("MAP VAR:%s\n",emap[i].var);
      if(strncmp(emap[i].var,var,18)==0) {
	 if(pno==2) strcpy(ret,emap[i].value);
	 if(pno==3) strcpy(ret,emap[i].p3);
	 if(pno==4) strcpy(ret,emap[i].p4);
      }      
   }
} // dialog_mapget()

dialog_mapset(const char *var, const char *val,const char *p3, const char *p4) {
   int i,cc=0;
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
} // dialog_mapset()

dialog_menv(char *val,const char *var) {
   int i;
   
   strcpy(val,"");
   for(i=0;i<mapidx;i++) {
//      printf("%d\n",i);
#ifdef DEBUG
      printf("coo:%s<-->%s\n",emap[i].var,emap[i].value);
 //     printf("does:>%s<=>%s<\n",emap[i].var,var);
 //     printf("lengths: %d,%d\n",strlen(emap[i].var),strlen(var));
#endif
      if(strcmp(emap[i].var,var)==0) {
	 strcpy(val,emap[i].value);
//	 printf("yes, it does:%s\n",val);
	 break;
      }
   }
}  // dialog_menv()

dialog_menvget(char *val, const char *var) {
   int i;
   char ival[40];

   dialog_menv(ival,var);
   printf("ival is %s\n",ival);

   if(strcmp(ival,"")!=0) {
      dialog_envget(val,ival);
      if(val[0]=='\"') {
	 dss_index(ival,val,1,'\"');
	 strcpy(val,ival);
      }
   }
} // dialog_menvget()

int dialog_load_file(char *fname) {  // from EXX:load_setup_fl
   FILE *fp;
   char lb[200],v1[40],v2[40],v3[30],v4[80];
   int sw=0,nl=0,i;
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
	    dss_index(v1,lb,0,'|');
	    dss_index(v2,lb,1,'|');
	    dss_index(v3,lb,2,'|');
//	    printf("v3:%s\nv4:%s\n",v3,v4);
	    dss_index(v4,lb,3,'|');
//	    printf("v1:%s\nv2:%s\n",v1,v2);
//	    printf("v3:%s\nv4:%s\n",v3,v4);
	    dialog_mapset(v1,v2,v3,v4);
	 }
	 
	 if(sw==3) {
	    printf("PROCKING BITMAP!!!\n");
	    dss_index(v1,lb,0,'|');
	    dss_index(v2,lb,1,'|');
	    if(strcmp(v1,"bitmap")==0) {
	       if(strcmp(v2,"file")==0) {
		  // Load it from a file
		  dss_index(v3,lb,2,'|');
		  sprintf(v4,"%s%cgfx%c%s",datadir,mysep,mysep,v3);
		  printf("need to load bitmap: %s\n",v4);
		  if(dbitmap!=NULL) destroy_bitmap(dbitmap);
		  dbitmap=load_bitmap(v4,dp);
	       }
	    } else {
	       dialog_envset(v1,v2);
	    }
	 }
	 
	 if(strncmp(lb,"=BITMAP",7)==0)
	   sw=3;
	 if(strncmp(lb,"=VARIABLE",9)==0)
	   sw=2;
	 if(strncmp(lb,"=END VARIABLE",13)==0 || strncmp(lb,"=END BITMAP",11)==0)
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
} // dialog_load_file()

dialog_mystyle() {
   // Override some of the widget default styles
   activestyle.input.bg=makecol(240,240,240);
} // dialog_mystyle()

dialog_updent(int wid,const char *newval) {
   char wtext[20],vr[20],vl[80],newst[40];
//   strcpy(wtext,swin.widget[wid].text);
//   printf("uwtext is now %s\n",wtext);
   strcpy(vr,swin.widget[wid].etrig);
   printf("vr is now %s\n",wtext);
   dialog_envget(vl,newval); // get geometry
   printf("vl: %s\n",vl);
   dss_index(newst,vl,4,',');
   strcpy(swin.widget[wid].widget->text,newst);
   (*(swin.widget[wid].widget->handler))(swin.widget[wid].widget,-1,-1,-1);
}

dialog_drawbmp(BITMAP *b, int wid, const char *newval) {
   char vl[80],vr[20],mt[20],geo[15],wx[7], wtext[20];
   int dx,dy,dw,dh, sx,sy,sw,sh;
   
   strcpy(wtext,swin.widget[wid].text);
   printf("wtext is now %s\n",wtext);
   // get absolute params
   
   if(b!=screen) printf("not drawing to screen\n");
   
   dx=swin.startx+swin.widget[wid].x;
   dy=swin.starty+swin.widget[wid].y;
   dw=swin.widget[wid].w;
   dh=swin.widget[wid].h;
   
   if(dbitmap!=NULL) {
      dialog_mapget(vr,wtext,2);
      dialog_mapget(mt,wtext,3);
      
//      dialog_mapget(geo,wtext,4);
//      dss_index(wx,geo,0,'x'); dw=atoi(wx);
//      dss_index(wx,geo,1,'x'); dh=atoi(wx);
      
//      dialog_envget(vl,vr); // get current val
      printf("dialog_drawbmp(): %s:%s:  %d,%d\n",wtext,mt,dw,dh);
      dialog_envget(vl,newval); // get geometry
      dss_index(wx,vl,0,','); sx=atoi(wx);
      dss_index(wx,vl,1,','); sy=atoi(wx);
      dss_index(wx,vl,2,','); sw=atoi(wx);
      dss_index(wx,vl,3,','); sh=atoi(wx);
      printf("dialog_drawbmp() 2: %s %s %d-%d-%d-%d\n",newval,vl,sx,sy,sw,sh);
      printf("blit(dbitmap,b,%d,%d,%d,%d,%d,%d\n",sx,sy,dx,dy,dw,dh);
      if(strcmp(mt,"CROP")==0)
	blit(dbitmap,b,sx,sy,dx,dy,dw,dh);
      if(strcmp(mt,"SCALE")==0) {
	printf("blitstretch(dbitmap,b,%d,%d,%d,%d,%d,%d,%d,%d\n",sx,sy,sw,sh,dx,dy,dw,dh);
	blitstretch(dbitmap,b,sx,sy,sw,sh,dx,dy,dw,dh);
      }
   }
}


Widget* dialog_exec(char *n_setup, int wintype) {
   char fname[120],envval[60],wst[20];
   int sx,sy,i,sxo,syo,l;
   int x2,y2,w2,h2;
   Widget* nw;
   PALETTE p;
   BITMAP *b;
   bmpbtn_t mb;

   swin.type=wintype;
   swin.pw=NULL;
   swin.bm=screen;
//   useb=screen;
   wdgst_default();
   dialog_mystyle();
   dinf.fontsave=fnt_getactive();
   fnt_setactive(cf8x16);
//   font_set_builtin();
   
   dialog_envget(fname,"VERSION");
#ifdef DEBUG
   printf("=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n");
   printf("testing env read\n");
   printf("VERSION is %s\n",fname);
   printf("=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n");
#endif
   
//   l=gfx_alert("Are You Sure You Want This?",KEY_ENTER,"OK",-1,NULL,-1,NULL);
   swin.width=swin.height=swin.numwidget=0;
//   sprintf(fname,"c:\\cygwin\\usr\\src\\emufe-2.5.3\\src\\%s.def",n_setup);
   sprintf(fname,"%s%cdialog%c%s.def",datadir,mysep,mysep,n_setup);
   printf("Gloading %s\n",fname);
   dialog_envprint();
   if(!dialog_load_file(fname))
     return 0;
//   printf("Width:%d  Height:%d\n",swin.width,swin.height);
   if(swin.width>0 && swin.height>0) {
      // Don't init unless it hasn't been initted already
//      widget_init();
//      widget_clear_level();
      if(wintype==WDG_WIN_FIXED) {
	 sx=(VIRTUAL_W-swin.width)/2;
	 sy=(VIRTUAL_H-swin.height)/2;
	 swin.startx=sx;
	 swin.starty=sy;
	 new_window(sx,sy,sx+swin.width,sy+swin.height);
      }
      if(wintype==WDG_WIN_FLOAT) {
	 sx=sy=swin.startx=swin.starty=0;
	 swin.pw=widget_newwindow(sx,sy,swin.width,swin.height);
//	 useb=swin.pw->extra;
	 swin.bm=swin.pw->extra;
//	 swin.pw->extra=NULL; //test
      }
      dialog_windecor(swin.bm,sx,sy,swin.width,swin.height,makecol(182,170,170));
      
      for(i=0;i<swin.numlines;i++) {
	 dialog_parse_line(swin.line[i].str,i);
	 fnt_print_string(swin.bm,sx,sy+((i+1)*16),swin.line[i].str,makecol(240,240,240),-1,-1);
      }
      // render buttons
      for(i=0;i<swin.numwidget;i++) {
	 sxo=sx+swin.widget[i].x;
	 syo=sy+swin.widget[i].y;
	 if(swin.widget[i].type==SU_BUTTON) {
	    l=sxo+(strlen(swin.widget[i].text)*8)+8;
//	    nw=add_button(sxo,syo,l,syo+20,swin.widget[i].text,&dialog_buttoncb);
//	    printf("ADDIN Button:  %dx%d\n",sxo,syo);
	    nw=wdg_button_txt_add(swin.pw,sxo,syo,l,syo+20,swin.widget[i].text,&dialog_buttoncb);
//	    dialog_menvget(nw->id,swin.widget[i].text);
	    dialog_menv(nw->id,swin.widget[i].text);
printf("ERRG: %s %s\n",swin.widget[i].text,nw->id);
//	    strcpy(nw->id,"TEST");
	    if(strcmp(swin.widget[i].text,"ACCEPT")==0)
	      wdg_bind_key(nw,KEY_ENTER,-1,0);
	    if(strcmp(swin.widget[i].text,"CANCEL")==0)
	      wdg_bind_key(nw,KEY_ESC,-1,0);
	 }
	 if(swin.widget[i].type==SU_BMPBUTTON) {
	    l=sxo+(strlen(swin.widget[i].text)*8)+8;
	    strcpy(envval,swin.widget[i].text);
	    dss_index(swin.widget[i].text,envval,1,'~');
	    // get bmp geometry
	    dialog_envget(envval,swin.widget[i].text);
	    printf("XxX: %s | %s\n",envval,swin.widget[i].text);
	    dss_index(wst,envval,0,','); x2=atoi(wst);
	    dss_index(wst,envval,1,','); y2=atoi(wst);
	    dss_index(wst,envval,2,','); w2=atoi(wst);
	    dss_index(wst,envval,3,','); h2=atoi(wst);
	    // setup button structure
	    mb.btnupbmp=mb.btndnbmp=dbitmap;
	    mb.btnhlbmp=NULL;
	    mb.up_x1=x2; mb.up_y1=y2; mb.up_x2=w2; mb.up_y2=h2;
	    mb.dn_x1=x2; mb.dn_y1=y2; mb.dn_x2=w2; mb.dn_y2=h2;
	    mb.hl_x1=0; mb.hl_y1=0; mb.hl_x2=0; mb.hl_y2=0;
	    
	    nw=wdg_button_bmp_add(swin.pw,sxo,syo,l,syo+20,mb,&dialog_buttoncb);
	    nw->text=(char *)malloc(strlen(swin.widget[i].text)+4);
	    strcpy(nw->text,swin.widget[i].text);
	    dialog_menv(nw->id,swin.widget[i].text);
//printf("bERRG: %s %s\n",swin.widget[i].text,nw->id);
//	    dialog_drawbmp(swin.bm,i,envval);
//	    printf("asd: >%s<\n",envval);
	 }
//	 printf("widgeta %d  type: %d\n",i,swin.widget[i].type);
	 if(swin.widget[i].type==SU_CHECKBOX) {
	    dialog_menvget(swin.widget[i].evalue,swin.widget[i].text);
	    dialog_mapget(fname,swin.widget[i].text,4);
	    dss_index(wst,fname,1,',');
	    dialog_envprint();
	    printf("chh: >%s<\n",swin.widget[i].text);
	    printf("chi: >%s must equal %s<\n",wst,swin.widget[i].evalue);
//	    printf("chc: >%s<\n",fname);
	    if(strcmp(wst,swin.widget[i].evalue)==0)
	      nw=wdg_checkbox_add(swin.pw,sxo,syo+4,1,&dialog_checkcb);
	    else
	      nw=wdg_checkbox_add(swin.pw,sxo,syo+4,0,&dialog_checkcb);
	    swin.widget[i].widget=nw;
//	       (*(nw->handler))(nw,-1,-1,-1);
	    fnt_print_string(swin.bm,sxo+8,syo,swin.widget[i].text,makecol(0,0,0),-1,-1);
	 }
	 if(swin.widget[i].type==SU_ENVTEXT) {
//	    printf("wth: >%s<\n",swin.widget[i].text);
	    dialog_menvget(envval,swin.widget[i].text);
//	    printf("asd: >%s<\n",envval);
	    fnt_print_string(swin.bm,sxo+8,syo,envval,makecol(0,0,0),-1,-1);
	 }
	 if(swin.widget[i].type==SU_PLUSMINUS) {
	    //	    strcpy(swin.widget[i].evalue,"n/a");
	    //	    dialog_menvget(swin.widget[i].evalue,swin.widget[i].text);
	    nw=wdg_button_txt_add(swin.pw,sxo,syo+1,sxo+13,syo+16,"-",&dialog_pmselect);
	    sprintf(nw->id,"-%d",i);
	    swin.widget[i].widget=nw;
	    
	    dialog_menvget(swin.widget[i].evalue,swin.widget[i].text);
	    printf("construct pm button: %s - %s\n",swin.widget[i].evalue,swin.widget[i].text);
	    
	    dialog_mapget(envval,swin.widget[i].text,3);
	    l=strlen(envval)*8+18;
//	    nw=wdg_button_txt_add(swin.pw,sxo+l,syo+1,sxo+l+13,syo+16,"+",&dialog_pmselect);
	    if(envval[0]=='~') { 
	       swin.widget[i+1].type=SU_BITMAP;
	       l=strlen(envval)*8+18;
	    }
	    if(envval[0]=='_') { 
	       swin.widget[i+1].type=SU_NULL;
	       l=18;
	    }
	    nw=wdg_button_txt_add(swin.pw,sxo+l,syo+1,sxo+l+13,syo+16,"+",&dialog_pmselect);
	    sprintf(nw->id,"+%d",i);
	    swin.widget[i+1].y=swin.widget[i].value=0;
	    swin.widget[i+1].y=swin.widget[i].y;
	    swin.widget[i+1].x=swin.widget[i].x+16;
	    dss_cutstr(swin.widget[i+1].text,envval,1,strlen(envval)-1);
	 }
	 if(swin.widget[i].type==SU_BITMAP) {
	    printf("wth: >%s<\n",swin.widget[i].text);
	    dialog_mapget(envval,swin.widget[i].text,2);
	    strcpy(swin.widget[i].etrig,envval);
	    dialog_mapget(envval,swin.widget[i].text,4);
	    dss_index(wst,envval,0,'x'); swin.widget[i].w=atoi(wst);
	    dss_index(wst,envval,1,'x'); swin.widget[i].h=atoi(wst);

	    printf("geo: %s %d,%d,%d,%d\n",envval,swin.widget[i].x,swin.widget[i].y,swin.widget[i].w,swin.widget[i].h);
	    dialog_menvget(envval,swin.widget[i].text);
	    dialog_drawbmp(swin.bm,i,envval);
	    printf("asd: >%s<\n",envval);
//            b=load_bitmap(envval,p);
//	    blit(b,swin.bm,0,0,sxo,syo,64,64);
//	    destroy_bitmap(b);
	 }
	 
	 if(swin.widget[i].type==SU_SELECT || swin.widget[i].type==SU_EDIT) {
//	    printf("wth: >%s<\n",swin.widget[i].text);
	    strcpy(swin.widget[i].evalue,"n/a");
	    dialog_menvget(swin.widget[i].evalue,swin.widget[i].text);
	    dialog_mapget(wst,swin.widget[i].text,3);
	    if(wst[0]=='@') {
	       dss_index(envval,wst,1,'@');
	       strcpy(swin.widget[i].etrig,envval);
	    }
//	    printf("asd: >%s<\n",swin.widget[i].evalue);
//	    fnt_print_string(swin.bm,sxo+8,syo,envval,makecol(0,0,0),-1,-1);
//	    nw=add_input (sxo+8,syo,strlen(swin.widget[i].text),swin.widget[i].evalue);
	    nw=wdg_input_add(swin.pw,sxo+8,syo,strlen(swin.widget[i].text),swin.widget[i].evalue);
	    swin.widget[i].widget=nw;
	    if(swin.widget[i].type==SU_SELECT) {
	       // need to bind this to the select, somehow
//	       nw=add_arrow_button(sxo+22+(strlen(swin.widget[i].text)*8),syo+14,DOWN,&dialog_openselect);
	       nw=wdg_button_arrow_add(swin.pw,sxo+22+(strlen(swin.widget[i].text)*8),syo+14,DOWN,&dialog_openselect);
	    }
	 }
      }
      swin.active=1;
      fdlg_active=1;
      strcpy(fdlg_lastoper,"NULL");

//      wdg_window_move(swin.pw,200,100);
//      wdg_window_activate(swin.pw);
      // Execute Window
#ifdef DEBUG
      printf("about to loop\n");
#endif
      if(wintype==WDG_WIN_FIXED) {
	 while(swin.active==1) {
	    rest(0);
	    s2a_flip(screen);
	    event_loop(JUST_ONCE);
	    kbd_loop(JUST_ONCE);
	 }
#ifdef DEBUG
	 printf("About to POP\n");
#endif
	 //     Another pop_level problem, what a surprise!
	 //      printf("About to POP\n");
	 pop_level();
      }
      
#ifdef DEBUG
      printf("swin.active = %d\n",swin.active);
#endif

      if(swin.active & P_SAVE) { 
#ifdef DEBUG
	 printf("Saving...\n");
#endif
	 for(i=0;i<swin.numwidget;i++) {
	    if(swin.widget[i].type==SU_SELECT || swin.widget[i].type==SU_EDIT 
	       || swin.widget[i].type==SU_CHECKBOX || swin.widget[i].type==SU_PLUSMINUS) {
	       dialog_menvget(envval,swin.widget[i].text);
	       if(strcmp(envval,swin.widget[i].evalue)!=0) {
		  dialog_menv(envval,swin.widget[i].text);
#ifdef DEBUG
		  printf("CHAnge DeTeCtEd: %s<>%s\n",envval,swin.widget[i].evalue);
		  printf("%s=%s\n",envval,swin.widget[i].evalue);
#endif
		  dialog_envset(envval,swin.widget[i].evalue);
	       }
	    }
	 }	
#ifdef DEBUG
	 dialog_envprint();
#endif
      } // swin.active & P_SAVE
   }
   fnt_setactive(dinf.fontsave);
   return swin.pw;
} 

void dialog_maplocalcfgname(const char *cfgname) {
   /* If you want to use a config file to store the config settings
    * (in the /share/game directory, most likely),  then call this with the
    * full path to the file.   It will then be 'registered' to be used with
    * the dialog features */
   strcpy(dinf.localenv,cfgname);
}

void dialog_mapglobalcfgname(const char *cfgname) {
   /* similar to map global, but can be used to set a local file 
    * (usually in $HOME/.game/something.conf).   This will override
    * the global, if present */
   strcpy(dinf.globalenv,cfgname);
}

dialog_init() {
   strcpy(dinf.localenv,"");
   strcpy(dinf.globalenv,"");
   fdlg_active=0;
   strcpy(fdlg_lastoper,"NULL");
   dbitmap=NULL;
}
