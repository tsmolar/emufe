#include <stdio.h>

#ifdef USESDL
#include <SDL.h>
#include "sdl_allegro.h"
#endif

#ifdef USEALLEGRO
#include <allegro.h>
#endif


#ifndef WIN32
#include <stdlib.h>
#endif
#include <sys/stat.h>
#include "emufe.h"
#ifdef WIN32
#include <process.h>
#endif

#define NO_OVERWRITE 1
#define OVERWRITE 0

typedef struct emuopts_t {
   char cmd_patt[512];
   char cmd_line[1536];
   char bintype[10];
   char fqrom[120];
   char uqrom[25]; // unqualified ROM
   char tmpdir[80];
   char diskloc[80]; // Where disks were last copied/unzipped to
   char cplocal;
   char cfgdir[80];
   char localcfg;
   char usecfg;
   char cfgfile[60];
   char osname[20];
   int exec;
} emuopts_t;

typedef struct romarc_t {
   char name[100]; // name, full path
   char type[6];
   int inserted;
} romarc_t;

// moved to emufe.h
//typedef struct env_t {
//   char var[20];
//   char value[80];
//} env_t;

typedef struct cmdpatt_t {
   char optname[40];
   char envpat[70];
   char cmdopt[200];
} cmdpatt_t;

#define ENV_EQ 0
#define ENV_NE 1
#define ENV_LT 2
#define ENV_GT 3

#define MAX_ENV 40
#define MAX_CMD 65

romarc_t arc[8];
emuopts_t emuopt;
cmdpatt_t cmdtbl[MAX_CMD];
env_t enviro[MAX_ENV];
int cmdtidx, envidx;

extern char mysep;

int file_exists(const char *filename) {
   FILE *file;
   if (file = fopen(filename, "r")) {
      fclose(file);
      return 1;
   }
   
   return 0;
}

int mod_readbm(char *bmfile) {
   char bf[220], p1[20], p2[200];
   FILE *fp;
   int i;
#ifdef DEBUG
   printf("Reading Bookmark: %s\n",bmfile);
#endif
   if(fp=fopen(bmfile,"rb")) {
      while(!feof(fp)) {
	 fgets(bf,218,fp);
#ifdef WIN32
	 // remove ^M linefeeds
	 for(i=0;i<strlen(bf);i++) {
	    if(bf[i]==13)
	      bf[i]=0;
	 }
#endif
	 hss_index(p1,bf,0,'|');
	 hss_index(p2,bf,1,'|');
	 if(strcmp(p1,"diskloc")==0) strcpy(emuopt.diskloc,p2);
	 if(strcmp(p1,"system")==0) strcpy(imenu.system,p2);
	 if(strcmp(p1,"sysbase")==0) strcpy(imenu.sysbase,p2);
	 if(strcmp(p1,"emulator")==0) strcpy(imenu.emulator,p2);
	 if(strcmp(p1,"mode")==0) imenu.mode=atoi(p2);
	 if(strcmp(p1,"game")==0) strcpy(imenu.game,p2);
	 if(strcmp(p1,"rc")==0) strcpy(imenu.rc,p2);
	 if(strcmp(p1,"menu")==0) strcpy(imenu.menu,p2);
	 if(strcmp(p1,"lastmenu")==0) strcpy(imenu.lastmenu,p2);
      }
      fclose(fp);
   }
}

int mod_writebm(int mode) {
   char bmfile[220];
   FILE *fp;
   if(getenv("EMUBOOKMARK"))
     strcpy(bmfile,getenv("EMUBOOKMARK"));
   else
     sprintf(bmfile,"%s%cemubookmark",emuopt.tmpdir,mysep);
#ifdef DEBUG
   printf("BOOKMARKF:%s\n",bmfile);
#endif
   if(mode==1) strcpy(emuopt.cmd_line,"QUIT");
   if(fp=fopen(bmfile,"wb")) {
      sprintf(bmfile,"diskloc|%s\n",emuopt.diskloc);
      fputs(bmfile,fp);
      sprintf(bmfile,"system|%s\n",imenu.system);
      fputs(bmfile,fp);
      sprintf(bmfile,"sysbase|%s\n",imenu.sysbase);
      fputs(bmfile,fp);
      sprintf(bmfile,"emulator|%s\n",imenu.emulator);
      fputs(bmfile,fp);
      sprintf(bmfile,"mode|%d\n",imenu.mode);
      fputs(bmfile,fp);
      sprintf(bmfile,"game|%s\n",imenu.game);
      fputs(bmfile,fp);
      sprintf(bmfile,"rc|%s\n",imenu.rc);
      fputs(bmfile,fp);
      sprintf(bmfile,"menu|%s\n",imenu.menu);
      fputs(bmfile,fp);
      sprintf(bmfile,"lastmenu|%s\n",imenu.lastmenu);
      fputs(bmfile,fp);
      sprintf(bmfile,"exec|%s\n",emuopt.cmd_line);
      fputs(bmfile,fp);
      fclose(fp);
   }
#ifdef DEBUG
   printf("END BOOKMARKF:\n");
#endif
}


int mod_colidx(char* rstr, char *ostr, int idx, char del) {
   // Alternative to strtok
   // Experimental version to allow an arbitrary number of chars
   // between fields
   char *gidx;
   int t=0,i=0,si=0,ei=0,sl;

   gidx=ostr;
   do {
      if(gidx[t]==del || gidx[t]=='\n' || gidx[t]==0) {
	    i++;
	    if(i==idx+1) { ei=t; break; }
	    while(gidx[t+1]==del) t++;
	    if(i==idx) si=t+1;
      } 
//	 printf("%c",gidx[t]);
      
      if(gidx[t]==0) break;
      t++;
   } while(1);
//   printf("\n");
// printf("si:%d  ei:%d\n",si,ei);
   
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

int zip_bintype(char *btype, const char *cext) {
   int i,rv=0;
   for(i=0;i<cmdtidx;i++) {
      if(strcmp(cmdtbl[i].optname,"ext")==0) {
	 if(strcmp(cext,cmdtbl[i].cmdopt)==0) { 
	    strcpy(btype,cmdtbl[i].envpat);
	    break;
	 }
      }
   }
   return(rv);
}

int zip_isbin(char *cext) {
   int i,rv=0;
   for(i=0;i<cmdtidx;i++) {
      if(strcmp(cmdtbl[i].optname,"ext")==0) {
	 if(strcmp(cext,cmdtbl[i].cmdopt)==0) { 
	    rv=1;
	    break;
	 }
      }
   }
   return(rv);
}

int cplocaldisk() {
   char localpath[120], based[30];
   emu_basename(based,emuopt.fqrom);
   sprintf(localpath,"%s%c%s",emuopt.diskloc,mysep,based);
//   printf("NOTE: CpLoCaL src:%s  dest:%s\n",emuopt.fqrom,localpath);
   fileio_cp(emuopt.fqrom,localpath);
   strcpy(emuopt.fqrom,localpath);
}

int customcfg() {
   char icfgfile[120], ocfgfile[120],lbi[200],lbo[200];
   int n;
   FILE *fi, *fo;

//   n=hss_count(emuopt.cfgfile,mysep);
 //  printf("num seprs: %d\n",n);
//   basename(lbo,emuopt.cfgfile);
   //   if(strcmp(lbo,emuopt.cfgfile)==0)
//  printf("WHATFIVES? %s\n",emuopt.cfgfile);
   if(hss_count(emuopt.cfgfile,mysep)==1) {
//      printf("BBBBBAS: %s: in etc dir\n",emuopt.cfgfile);
//      sprintf(icfgfile,"%s%c%setc%c%s",basedir,mysep,imenu.sysbase,mysep,emuopt.cfgfile);
//  I'm not sure why we build the cfg file here, rather than doing the scanvar
//      sprintf(ocfgfile,"%s%c%s",emuopt.diskloc,mysep,emuopt.cfgfile);
      strcpy(ocfgfile,"");
      cmd_scanvar(ocfgfile,"%CFGFILE%");
      sprintf(icfgfile,"%s%c%setc%c%s",basedir,mysep,imenu.sysbase,mysep,emuopt.osname);
      if(fileio_dir_exists(icfgfile))
	sprintf(icfgfile,"%s%c%setc%c%s%c%s",basedir,mysep,imenu.sysbase,mysep,emuopt.osname,mysep,emuopt.cfgfile);
      else
	sprintf(icfgfile,"%s%c%setc%c%s",basedir,mysep,imenu.sysbase,mysep,emuopt.cfgfile);
   } else {
//      printf("BBgBBAS: %s: NOT! in etc dir\n",emuopt.cfgfile);
      strcpy(icfgfile,emuopt.cfgfile);
//      cmd_getvar(lbo,"CFGFILE");
      strcpy(ocfgfile,"");
      cmd_scanvar(ocfgfile,"%CFGFILE%");
//      printf("CHRAEI: %s\n",lbo);
//      sprintf(ocfgfile,"%s%c%s",emuopt.diskloc,mysep,lbo);
//      strcpy(ocfgfile,lbo);
   }
   // Make sure the parent directory exists
   fileio_dirname(lbo,ocfgfile);
   if(!fileio_dir_exists(lbo)) {
//      printf("RUHROH!  Dir doesn't exist!  %s:%s\n",lbo,ocfgfile);
      fileio_mkdir_p(lbo);
   }

#ifdef DEBUG
   printf("CUSTOM CFG\n----------\n");
   printf(" In File: %s\n",icfgfile);
   printf("Out File: %s\n",ocfgfile);
#endif
   if(fi=fopen(icfgfile,"rb")) {
      fo=fopen(ocfgfile,"wb");
      while(!feof(fi)) {
	 fgets(lbi,196,fi);
//	 printf("%s",lbi);
	 if(feof(fi)) break;
	 strcpy(lbo,"");
	 cmd_scanvar(lbo,lbi);
	 fprintf(fo,"%s",lbo);
      }
      fclose(fo);
      fclose(fi);
   }
//   mkdir(emuopt.diskloc); // do a check
}

int p_customcfg() {
   // persistant, custom cfg, for emulators that alter ini frequently
   char icfgfile[120], ocfgfile[120],lbi[200],lbo[200], var[40],context[40];
   int n;
   FILE *fi, *fo;

   
   strcpy(icfgfile,"");
   cmd_scanvar(icfgfile,"%CFGFILE%");
//   sprintf(icfgfile,"%s%c%s",emuopt.diskloc,mysep,emuopt.cfgfile);
   if(!file_exists(icfgfile)) {
//     printf("CFG NO EXISTO\n");
      if(hss_count(emuopt.cfgfile,mysep)==1) {
//	 printf("P_cFG: %s: in an etc dir\n",emuopt.cfgfile);
	 sprintf(ocfgfile,"%s%c%setc%c%s",basedir,mysep,imenu.sysbase,mysep,emuopt.osname);
	 if(fileio_dir_exists(ocfgfile))
	   sprintf(ocfgfile,"%s%c%setc%c%s%c%s",basedir,mysep,imenu.sysbase,mysep,emuopt.osname,mysep,emuopt.cfgfile);
	 else
	   sprintf(ocfgfile,"%s%c%setc%c%s",basedir,mysep,imenu.sysbase,mysep,emuopt.cfgfile);
      } else {
//	 printf("P_cFG: %s: NOT! in etc dir\n",emuopt.cfgfile);
	 strcpy(ocfgfile,emuopt.cfgfile);
      }
#ifdef DEBUG
      printf("cp %s %s\n",ocfgfile,icfgfile);
#endif
      fileio_cp(ocfgfile,icfgfile);
   }
   
//   sprintf(icfgfile,"%s%c%s",emuopt.diskloc,mysep,emuopt.cfgfile);
//   sprintf(ocfgfile,"%s%c%s.new",emuopt.diskloc,mysep,emuopt.cfgfile);
   sprintf(ocfgfile,"%s.new",icfgfile);
#ifdef DEBUG
   printf("CUSTOM CFG\n----------\n");
   printf(" In File: %s\n",icfgfile);
   printf("Out File: %s\n",ocfgfile);
#endif
   if(fi=fopen(icfgfile,"rb")) {
      fo=fopen(ocfgfile,"wb");
      while(!feof(fi)) {
	 fgets(lbi,196,fi);
	 if(feof(fi)) break;
//	 printf("%s\n",lbi);
	 if(lbi[0]=='[') {
	    hss_index(lbo,lbi,1,'[');
	    hss_index(context,lbo,0,']');
//	    printf("new context  is %s\n",context);
	    sprintf(lbo,"CTXT=%s",context);
	    env_set(lbo);
	 } else {
	    hss_index(lbo,lbi,0,'=');
//	    printf("%s\n",lbo);
	    sprintf(var,"%%%s%%",lbo);
	    strcpy(lbo,"");
//	    printf("pcfgvar=%s\n",var);
	    cmd_scanvar(lbo,var);
	    if(strcmp(lbo,"")!=0) {
	       hss_index(var,lbi,0,'=');
	       sprintf(lbi,"%s=%s\n",var,lbo);
	    }
//	    printf("pcfgvar=%s\n",lbo);
	 }
	 fprintf(fo,"%s",lbi);
      }
      fclose(fo);
      fclose(fi);
      fileio_cp(ocfgfile,icfgfile);
#ifdef WIN32
      sprintf(lbi,"del %s",ocfgfile);
#else
      sprintf(lbi,"rm -f %s",ocfgfile);
#endif
#ifdef DEBUG
      printf("e=xecing %s\n",lbi);
#endif
      system(lbi);
   }
//   mkdir(emuopt.diskloc); // do a check
}

int dealwithzip(int keep) {
   char zipexe[50], zipcmd[160],tmpfl[80],tmpfl2[120],lb[120],ext[6];
   int romcnt=0,i;
   FILE *fp;
   
//   printf("Dealing with zip\n");
   dfixsep2(zipexe,"tools/miniunz",1);
#ifdef DEBUG
   printf("ZIP exe=%s\n",zipexe);
#endif
   // check file exists
//   sprintf(zipcmd,"%s -l %s",zipexe,emuopt.fqrom);
//   strcpy(emuopt.diskloc,emuopt.tmpdir); // TEMP, change

   if(!fileio_dir_exists(emuopt.tmpdir))
     fileio_mkdir_p(emuopt.tmpdir);
   sprintf(tmpfl,"%s%ctmpziplst",emuopt.tmpdir,mysep);
   sprintf(zipcmd,"%s -l %s > %s",zipexe,emuopt.fqrom,tmpfl);
#ifdef DEBUG
   printf("WIN32 ZIP cmd=%s\n",zipcmd);
#endif
   system(zipcmd);
   // read disk list
   fp=fopen(tmpfl,"rb");
   for(i=0;i<8;i++) arc[i].inserted=0;
   while(!feof(fp)) {
      fgets(lb,118,fp);
#ifdef WIN32
      // remove ^M linefeeds
      for(i=0;i<strlen(lb);i++) {
	 if(lb[i]==13)
	   lb[i]=0;
      }
#endif
      if(feof(fp)) break;
      if(lb[0]==' ')
       mod_colidx(tmpfl,lb,8,' ');
      else
       mod_colidx(tmpfl,lb,7,' ');
      hss_index(ext,tmpfl,1,'.');
//      printf("ChEhK: %s  %s \n",tmpfl,ext);
      if(zip_isbin(ext)) {
	 zip_bintype(arc[romcnt].type,ext);
//	 strcpy(arc[romcnt].name,tmpfl);
	 sprintf(arc[romcnt].name,"%s%c%s",emuopt.diskloc,mysep,tmpfl);
	 arc[romcnt].inserted=1;
//	 printf("%s -> %s\n",arc[romcnt].name,arc[romcnt].type);
	 sprintf(tmpfl,"HAVEDISK%d=Y",romcnt+1);
	 env_set(tmpfl);
	 romcnt++;
	 if(romcnt>7) break;
      }
   }
   fclose(fp);
//   env_print();
   // need to set to conf dir in some cases
   chdir(emuopt.diskloc);
   for(i=0;i<romcnt;i++) {
      if(keep == OVERWRITE) {
	 sprintf(tmpfl2,"rm -f %s", arc[i].name);
	 printf("DELETING:: %s\n",tmpfl2);
	 system(tmpfl2);
      }
      if(file_exists(arc[i].name)) {
#ifdef DEBUG
	 printf("EXISTS: %s\n",arc[i].name);
#endif
      } else {
	 emu_basename(tmpfl,arc[i].name);
#ifdef WIN32XXX
	 // Test spawn command to keep dos windows to a minimum
	 // Still seems to open just as many windows
//	 printf("WIN32 ZIP exe=%s\n",zipexe);
	 spawnl(_P_WAIT,zipexe,"-o",emuopt.fqrom,tmpfl,0);
#else
	 sprintf(zipcmd,"%s -o %s %s",zipexe,emuopt.fqrom,tmpfl);
# ifdef DEBUG
	 printf("ZIP exe=%s\n",zipexe);
	 printf("ZIP cmd=%s\n",zipcmd);
# endif
	 system(zipcmd);
#endif
      }
   }
   // might need to do a chmod
   return(romcnt);
}

int mod_optype(const char *stmt) {
   int optype=-1, i;
   for(i=0;i<strlen(stmt);i++) {
      if(stmt[i]=='=') { optype=ENV_EQ; break; }
      if(stmt[i]=='!') { optype=ENV_NE; break; }
      if(stmt[i]=='<') { optype=ENV_LT; break; }
      if(stmt[i]=='>') { optype=ENV_GT; break; }
   }
   return(optype);
}

int env_get(char *val, const char *var) {
   int i;
   // Special Cases
   if(strcmp(var,"Ebintype")==0) strcpy(val,emuopt.bintype);
   // Still process, so that special cases can be overridden
   for(i=0;i<envidx;i++) {
      if(strcmp(enviro[i].var,var)==0) {
	 strcpy(val,enviro[i].value);
	 break;
      }
   }
}

int env_cmp(char *senv) {
   char var[40], val[80];
   int i,rv=0,optype;
   optype=mod_optype(senv);
   if(optype==ENV_EQ) {
      hss_index(var,senv,0,'=');
      hss_index(val,senv,1,'=');
   }
   if(optype==ENV_NE) {
      hss_index(var,senv,0,'!');
      hss_index(val,senv,1,'=');
   }
   if(optype==ENV_LT) { hss_index(var,senv,0,'<'); hss_index(val,senv,1,'<');}
   if(optype==ENV_GT) { hss_index(var,senv,0,'>'); hss_index(val,senv,1,'>');}
//   printf("%s==%s\n",var,val);
   for(i=0;i<envidx;i++) {
      if(optype==ENV_EQ && strcmp(enviro[i].var,var)==0 && strcmp(enviro[i].value,val)==0) {
	 rv=1;
	 break;
      }
      if(optype==ENV_NE && strcmp(enviro[i].var,var)==0 && strcmp(enviro[i].value,val)!=0) {
	 rv=1;
	 break;
      }
      if(optype==ENV_LT && strcmp(enviro[i].var,var)==0 && (atoi(enviro[i].value) < atoi(val))) {
	 rv=1;
	 break;
      }
      if(optype==ENV_GT && strcmp(enviro[i].var,var)==0 && (atoi(enviro[i].value) > atoi(val))) {
	 rv=1;
	 break;
      }
   }
   return(rv);
}

int cmd_gethdx(char *myhd, char *type, char *loc) {
   int i;
   strcpy(myhd,"");
   for(i=0;i<cmdtidx;i++) {
      if(strcmp(cmdtbl[i].optname,type)==0) {
	 if(strcmp(cmdtbl[i].envpat,loc)==0) 
	   cmd_scanvar(myhd,cmdtbl[i].cmdopt);
      }
   }
}

int cmd_insthdi(){
   char localhd[80], globalhd[80], dirval[80];
   int r;

//   printf("INSTALL HDI!!!!\n");
   cmd_gethdx(globalhd,"hdi", "global");
   cmd_gethdx(localhd,"hdi", "local");
#ifdef DEBUG
   printf("HDI*: local:%s\n",localhd);
   printf("HDI*: global:%s\n",globalhd);
#endif
   if(file_exists(globalhd)) {
      r=setup_hd(globalhd,localhd,"hdi");
//      printf("r val is %d\n",r);
      if(r==1) {
//	 printf("actual installation goes here\n");
	 fileio_dirname(dirval,localhd);
#ifdef DEBUG
	 printf("installing to %s\n",dirval);
#endif
	 if(!fileio_dir_exists(dirval))
	   setup_hderr("Destination Directory does not exist:",dirval,"Please Run Setup first");
	 else {
	    fileio_cp(globalhd,localhd);
	    setup_hderr("      HD Image successfully installed!      ","",localhd);
//	    printf("directori exists\n");
	 }
      }   
   } else 
     setup_hderr("Install image does not Exist:","",globalhd);
}

int cmd_insthdd(){
   char localhd[80], globalhd[80], dirval[80];
   int r;

   printf("INSTALL HDD!!!!\n");
   cmd_gethdx(globalhd,"hdd", "global");
   cmd_gethdx(localhd,"hdd", "local");
   printf("HDD*: local:%s\n",localhd);
   printf("HDD*: global:%s\n",globalhd);
   if(fileio_dir_exists(globalhd)) {
      r=setup_hd(globalhd,localhd,"hdd");
//      printf("r val is %d\n",r);
      if(r==1) {
//	 printf("actual installation goes here\n");
	 fileio_dirname(dirval,localhd);
	 printf("installing to %s\n",dirval);
	 if(!fileio_dir_exists(dirval))
	   setup_hderr("Destination Directory does not exist:",dirval,"Please Run Setup first");
	 else {
	    //	    fileio_cp(globalhd,localhd);
	    if(fileio_dir_exists(localhd)) {
	       setup_hderr("It appears the HD is already installed ",localhd,"Please remove it if you want to reinstall");
	    } else {
#ifdef WIN32
	       sprintf(dirval,"xcopy %s\\* %s /s /i ",globalhd,localhd);
#else
	       sprintf(dirval,"cp -rp %s %s",globalhd,localhd);
#endif
	       printf("%s\n",dirval);
	       system(dirval);
	       setup_hderr("        HD successfully installed!        ","",localhd);
	       ////	    printf("directori exists\n");
	    }
	 }
      }   
   } else 
     setup_hderr("HD source does not Exist:","",globalhd);
}


int cmd_gethdi(char *myhd) {
   // For hard drive images
   char localhd[80], globalhd[80];
   int i;
   strcpy(myhd,"NA");
   strcpy(globalhd,"");
   strcpy(localhd,"");
   for(i=0;i<cmdtidx;i++) {
      if(strcmp(cmdtbl[i].optname,"hdi")==0) {
	 if(strcmp(cmdtbl[i].envpat,"local")==0) 
	   cmd_scanvar(localhd,cmdtbl[i].cmdopt);
	 if(strcmp(cmdtbl[i].envpat,"global")==0) 
	   cmd_scanvar(globalhd,cmdtbl[i].cmdopt);
      }
   }
   printf("HDI*: local:%s\n",localhd);
   printf("HDI*: global:%s\n",globalhd);
   if(strcmp(localhd,"")!=0 && file_exists(localhd))
     strcpy(myhd,localhd);
   else {
      // check for zipped global images (or should that be in the
      // install process, since they'd be useless here?
      if(strcmp(globalhd,"")!=0 && file_exists(globalhd))
	strcpy(myhd,globalhd);
   }
}

int cmd_gethdd(char *myhd) {
   // For directories doubling as hard drives
   char localhd[80], globalhd[80];
   int i;
   strcpy(myhd,"NA");
   strcpy(globalhd,"");
   strcpy(localhd,"");
   for(i=0;i<cmdtidx;i++) {
      if(strcmp(cmdtbl[i].optname,"hdd")==0) {
	 if(strcmp(cmdtbl[i].envpat,"local")==0) 
	   cmd_scanvar(localhd,cmdtbl[i].cmdopt);
	 if(strcmp(cmdtbl[i].envpat,"global")==0) 
	   cmd_scanvar(globalhd,cmdtbl[i].cmdopt);
      }
   }
//   printf("HDD*: local:%s\n",localhd);
//   printf("HDD*: global:%s\n",globalhd);
   if(strcmp(localhd,"")!=0 && fileio_dir_exists(localhd))
     strcpy(myhd,localhd);
   else {
      if(strcmp(globalhd,"")!=0 && fileio_dir_exists(globalhd))
	strcpy(myhd,globalhd);
   }
}

int cmd_getwd(char *wd) {
   // If a cd is needed before running an emulator (IE, to find a rom image)
   // It is read hear, from the first uwd entry in the cmd_table
   int i;
   strcpy(wd,"");
   for(i=0;i<cmdtidx;i++) {
      if(strcmp(cmdtbl[i].optname,"uwd")==0) {
	 cmd_scanvar(wd,cmdtbl[i].cmdopt);
	 break;
      }
   }
}

int cmd_getcmdline(char *bintype) {
   int i;
   
//   printf("in cmd_getcmdline\n");
   
   for(i=0;i<cmdtidx;i++) {
      if(strcmp(cmdtbl[i].optname,"cmd")==0 && strcmp(cmdtbl[i].envpat,bintype)==0 ) {
//   printf("option is %s\n",cmdtbl[i].cmdopt);
	 strcpy(emuopt.cmd_patt,cmdtbl[i].cmdopt);
	 break;
      }
      if(strcmp(cmdtbl[i].optname,"cmd")==0 && strcmp(cmdtbl[i].envpat,"*")==0 )
	strcpy(emuopt.cmd_patt,cmdtbl[i].cmdopt);
   }
}

int cmd_getvar(char *currl, char *currvar) {
   // Replace the %VAR% with a real value
   int i,r,didcomm=0,i2;
   char envo[20], eval[40], edir[40], optvar[60];
   
//   if(strcmp(currvar,"BIN")==0 || strcmp(currvar,"PROG")==0
//       || strcmp(currvar,"RBIN")==0 || strcmp(currvar,"RPROG")==0 
//       || strcmp(currvar,"/")==0  ) {
   if(strcmp(currvar,"BIN")==0) {
      if(env_isset("USEBIN")) 
	sprintf(envo,"USEBIN",imenu.emulator);
      else
	sprintf(envo,"%s_bin",imenu.emulator);
      env_get(eval,envo);
      sprintf(envo,"%s_dir",imenu.emulator);
      env_get(edir,envo);
      sprintf(currl,"%s%s%c%s",basedir,edir,mysep,eval);
      didcomm=1;
   }
   if(strcmp(currvar,"RBIN")==0) {
      sprintf(currl,"%s%s",imenu.sysbase,imenu.emulator);
      didcomm=1;
   }
   if(strcmp(currvar,"RPROG")==0) {
      strcpy(currl,imenu.game); 
      didcomm=1;
   }
   if(strcmp(currvar,"SYS")==0) { 
      strcpy(currl,imenu.system); 
      didcomm=1;
   }
   if(strcmp(currvar,"DISKLOC")==0) {
      strcpy(currl,emuopt.diskloc); 
      didcomm=1;
   }
   if(strcmp(currvar,"/")==0) { 
      sprintf(currl,"%c",mysep);
      didcomm=1;
   }
   if(strcmp(currvar,"EQ")==0) { 
      strcpy(currl,"=");
      didcomm=1;
   }
   if(strcmp(currvar,"PROG")==0) {
      // find extension somehow
      strcpy(currl,emuopt.fqrom);
      didcomm=1;
   }
   if(strcmp(currvar,"BPROG")==0) {
      // find extension somehow
      emu_basename(currl,emuopt.fqrom);
      didcomm=1;
   }
   if(strcmp(currvar,"HDD")==0) {
      cmd_gethdd(currl);
      didcomm=1;
   }
   if(strcmp(currvar,"HDI")==0) {
      cmd_gethdi(currl);
      didcomm=1;
   }
   if(strlen(currvar)==5 && strncmp(currvar,"DISK",4)==0) {
   // DISK1, DISK2..8
#ifdef DEBUG
      printf("CURRVAR is %s\n",currvar);
#endif
      i2=currvar[4]-48;
//      printf("%d is i2\n",i2);
      if(i2>0 && i2<9) {
	 strcpy(currl,arc[i2-1].name);
	 didcomm=1;
      }
   }
   
   if(didcomm==0) {
      for(i=0;i<cmdtidx;i++) {
	 if(strcmp(cmdtbl[i].optname,currvar)==0) {
	    if(strcmp(cmdtbl[i].envpat,"*")==0 || strcmp(cmdtbl[i].envpat,"<<")==0 || strcmp(cmdtbl[i].envpat,"<")==0) {
	       if(strcmp(cmdtbl[i].envpat,"*")==0) strcpy(currl,cmdtbl[i].cmdopt);
	       if(strcmp(cmdtbl[i].envpat,"<<")==0) {
		  env_get(eval,cmdtbl[i].cmdopt);
		  dfixsep2(edir,eval,1);
		  strcpy(currl,edir);
	       }
	       if(strcmp(cmdtbl[i].envpat,"<")==0) {
		  env_get(eval,cmdtbl[i].cmdopt);
//		  dfixsep2(edir,eval,0);
		  strcpy(currl,eval);
	       }
	       break;
	    } else {
	       r=env_cmp(cmdtbl[i].envpat);
//	       if(strcmp(currvar,"SCAN")==0) printf("====SCAN: %s -> %d\n",cmdtbl[i].envpat,r);
	       if(r==1) {
		  strcpy(currl,cmdtbl[i].cmdopt);
		  break;
	       }
	    }
	 } // if optname
      } // for(i)
   } // BIN / PROG exception
}

int cmd_scanvar(char *strout, const char *strin) {
   // This will replace all %VARS% in strin, with their values,
   // recursively
   // The recursion is broken if input + output matches.  Might be buggy
   // may need a no/var flag
   char currvar[32],currl[300],cmdline[1536];
   int i,i2;
   
   for(i=0;i<strlen(strin);i++) {
      if(strin[i]=='%') {
	 for(i2=i+1; i2<strlen(strin);i2++) {
	    if(strin[i2]=='%') {
	       currvar[i2-(i+1)]='\0';
	       i=i2;
	       break;
	    } else
	      currvar[i2-(i+1)]=strin[i2];
	 }
	 strcpy(currl,"");
//	 if(strcmp(currvar,"FAST")==0) strcpy(currl,emuopt.fastcpu);
	 cmd_getvar(currl,currvar);
#ifdef DEBUG
	 printf("CMD is %s  CL is %s\n",currvar,currl);
#endif
      } else {
	 // printf("I AM HERE\n");
	 sprintf(currl,"%c",strin[i]);
      }
      strcat(strout,currl);
   }
   if(strcmp(strout,strin)!=0) {
      strcpy(cmdline,strout);
      strcpy(strout,"");
      cmd_scanvar(strout,cmdline);
   }
}

int build_cmd() {
//   char currvar[12],currl[50],cmdline[1536];
//   int i,i2;
   cmd_getcmdline(emuopt.bintype);
   if(strncmp(emuopt.cmd_patt,"bin2disk",8)==0) {
      bin2disk();
      cmd_getcmdline("disk");
   }
#ifdef DEBUG
   printf("BINTYPE: %s\n",emuopt.bintype);
   printf("CMD LINE: %s\n",emuopt.cmd_patt);
#endif
   cmd_scanvar(emuopt.cmd_line,emuopt.cmd_patt);
}

int cmdtbl_clear() {
   cmdtidx=0;
}

int cmdtbl_new(char *sopt,char *senv, char *scmd) {
   if(cmdtidx>=MAX_CMD) {
      printf("ERROR: Cmd table exhausted!  Increase MAX_CMD and recompile\n");
   } else {
      //   printf("index %d\n",cmdtidx);
      strcpy(cmdtbl[cmdtidx].optname,sopt);
      strcpy(cmdtbl[cmdtidx].envpat,senv);
      strcpy(cmdtbl[cmdtidx].cmdopt,scmd);
      cmdtidx++;
//      printf("just inserted %d\n",cmdtidx);
   }
}

int cmdtbl_replace(char *sopt,char *senv, char *scmd) {
   // used to replace an option, namely ext options
   int i, inst=0;
   
   for(i=0;i<cmdtidx;i++) {
      if(strcmp(cmdtbl[i].cmdopt,scmd)==0) {
	 strcpy(cmdtbl[i].optname,sopt);
	 strcpy(cmdtbl[i].envpat,senv);
	 strcpy(cmdtbl[i].cmdopt,scmd);
	 inst=1;
	 break;
      }
   }
   if(inst==0)
     cmdtbl_new(sopt,senv,scmd);
}

int cmdtbl_print() {
   int i;
   printf("print of cmdtbl\n");
   for(i=0;i<cmdtidx;i++) {
      printf("opt=%s    env=%s   cmd=%s\n",cmdtbl[i].optname,cmdtbl[i].envpat,cmdtbl[i].cmdopt);
   }
}

int env_clear() {
   envidx=0;
}

int env_print() {
   int i;
   printf("print of env\n");
   for(i=0;i<envidx;i++) {
      printf("%s=%s\n",enviro[i].var,enviro[i].value);
   }
}

int env_isset(const char *svar) {
   int i,rv=0;
   for(i=0;i<envidx;i++) {
      if(strcmp(enviro[i].var,svar)==0) {
	 rv=1;
	 break;
      }
   }
   return(rv);
}

int env_set(char *senv) {
   char var[40], val[80];
   int i,cc=0;
   hss_index(var,senv,0,'=');
   hss_index(val,senv,1,'=');
   // special cases
   if(strcmp(var,"CPLOCAL")==0) emuopt.cplocal=val[0];
   if(strcmp(var,"EMUHOME")==0 && strcmp(val,"$EMUHOME")==0) 
     strcpy(val,basedir);
//   printf("%s==%s\n",var,val);
   for(i=0;i<envidx;i++) {
      if(strcmp(enviro[i].var,var)==0) {
	 strcpy(enviro[i].value,val);
	 cc=1;
//	 printf("set_env: replaced\n");
      }
   }
   if(cc==0) {
      if(envidx>=MAX_ENV) {
	 printf("ERROR: Environment space exhausted, increase MAX_ENV & recompile\n");
      } else {
	 strcpy(enviro[envidx].var,var);
	 strcpy(enviro[envidx].value,val);
//	 printf("set_env: added #%d\n",envidx);
	 envidx++;
      }
   }
}

int env_load(const char *emuenv) {
   FILE *fp;
   int i;
   char lb[260], value[20], var[40], v2[40];
#ifdef DEBUG
   printf("EMUENV file is %s (env_load)\n",emuenv);
#endif
   fp=fopen(emuenv,"rb");
   while(!feof(fp)) {
      fgets(lb,255,fp);
      if(lb[0]!='#') {
#ifdef WIN32
	 // remove ^M linefeeds
	 for(i=0;i<strlen(lb);i++) {
	    if(lb[i]==13)
	      lb[i]=0;
	 }
#endif
	 hss_index(var,lb,0,'=');
	 if(strcmp(var,"emulist") !=0 && strcmp(var,"syslist") !=0) {
	    hss_index(value,var,0,'_');
	    if(strcmp(value,var)==0) {
	       env_set(lb);
	    } else {
	       // System / Emu specific setting
	       if(strcmp(value,imenu.system)==0 || strcmp(value,imenu.emulator)==0 ) {
		  hss_index(value,var,1,'_');
		  if(strcmp(value,"bin")==0 || strcmp(value,"dir")==0 || strcmp(value,"rombase")==0 ) {
		     env_set(lb);
		  } else {
		     // put the stripped off env var in
		     hss_index(v2,lb,1,'=');
		     sprintf(lb,"%s=%s",value,v2);
		     env_set(lb);
		  }
	       }
	    } 
	 } // if not emulist or syslist (unneeded)
      } // if not comment
   } // While
   fclose(fp);
}

int mod_cleantmp() {
   if(emuopt.localcfg!='Y' && strcmp(emuopt.diskloc,"n/a")!=0) {
#ifdef WIN32
      sprintf(emuopt.cmd_line,"del /Q %s\\*.*",emuopt.diskloc);
#else
      sprintf(emuopt.cmd_line,"rm -f %s/*",emuopt.diskloc);
#endif
      printf("e=xecing %s\n",emuopt.cmd_line);
      system(emuopt.cmd_line);
      fileio_rmdir(emuopt.cmd_line);
   }
}

int setbootdisk() {
   // If BOOTDISK!=N, then we push the BOOTDISK into DISK1 slot, pushing
   // everything else back
   char haveboot[60], nenv[12];
   int i;
   env_get(haveboot,"BOOTDISK");
   if(haveboot[0]!='N' && haveboot[0]!='n') {
      cmd_getvar(haveboot,"BOOTDISK");
      for(i=7;i>0;i--) {
	 printf("disk %d before: %s\n",i+1,arc[i].name);
	 if(arc[i-1].inserted>0) {
	    strcpy(arc[i].name,arc[i-1].name);
	    strcpy(arc[i].type,arc[i-1].type);
	    arc[i].inserted=arc[i-1].inserted;
	    sprintf(nenv,"HAVEDISK%d=Y",i+1);
	    env_set(nenv);
	   }
	 printf("disk %d after: %s\n",i+1,arc[i].name);
      }
      strcpy(arc[0].name,haveboot);
      strcpy(arc[0].type,"disk");
      arc[0].inserted=1;
   }
}

int load_settings() {
   // C version of the shell script
   char *homet, emuenv[160];
   
//   // For disk-based configuration
//#ifdef WIN32
//   // if the following directory exists:
//   strcpy(emuopt.cfgdir,"C:\\emulator");
//   // else
//   //   home=TEMP\\emulator
//   // endif
//#else
//   homet=(char *)getenv("HOME");
//   sprintf(emuopt.cfgdir,"%s/.emulator",homet);
//#endif

   // For flash-drive based configurations
   // cfg1 implies that several configurations can be stored.  Not supported
   // in the front-end at this time, but can be renamed.
   sprintf(emuopt.cfgdir,"%s%cuser_config%ccfg1%c",basedir,mysep,mysep,mysep);
   
   
//   env_clear();
#ifdef DEBUG
   printf("emuopt.cfgdir dir: %s\n",emuopt.cfgdir);
#endif
   // should we save this into imenu, emuopts or other structure?  Probably
   sprintf(emuenv,"%s%cetc%cemucd.env",basedir,mysep,mysep);
   env_load(emuenv); // load global
   sprintf(emuenv,"%s%cemucd.env",emuopt.cfgdir,mysep);
//   printf("local env: %s\n",emuenv);
   emuopt.localcfg='N';
   if(file_exists(emuenv)) {
      env_load(emuenv); // load local
      env_set("LOCALCFG=Y");
      emuopt.localcfg='Y';
      sprintf(emuenv,"CFGDIR=%s",emuopt.cfgdir);
      env_set(emuenv);
#ifdef DEBUG
      env_print();
#endif
   }
   env_get(emuopt.tmpdir,"TMPDIR");
   // Check for wrong version
   // Check for wrong EMUHOME
}

int mod_loadcfg(const char *emucfg) {
   FILE *fp;
   char lb[260], value[80], var[40];
   char optname[30], envpat[60], cmdopt[150],cond[40],pval[60];
   int i;

//   sprintf(emucfg,"%s%c%s%cetc%cemu_%s.cfg",basedir,mysep,imenu.sysbase,mysep,mysep,imenu.emulator);
#ifdef DEBUG
   printf("EMUCFG2 file is %s\n",emucfg);
#endif
   emuopt.usecfg='N';
   fp=fopen(emucfg,"rb");
   while(!feof(fp)) {
      fgets(lb,255,fp);
//      printf("%s",lb);
#ifdef WIN32
      // remove ^M linefeeds
      for(i=0;i<strlen(lb);i++) {
	 if(lb[i]==13)
	   lb[i]=0;
      }
#endif
      if(feof(fp)) break;
      if(lb[0]!='#') {
	 hss_index(var,lb,0,'|');
	 hss_index(cond,lb,1,'|');
	 hss_index(value,lb,2,'|');
	 if(strcmp(var,"exe")==0) {
	    if(value[0]=='Y' || value[0]=='y' 
	       || value[0]=='T' || value[0]=='t') 
	      emuopt.exec=1;
	 }
	 
	 if(strcmp(var,"cmd")==0 || strcmp(var,"exp")==0 
	    || strcmp(var,"uwd")==0 || strcmp(var,"hdd")==0
	    || strcmp(var,"hdi")==0 ) { 
//	    strcpy(emuopt.cmd_patt,value);
	    cmdtbl_new(var,cond,value);
	 }
	 if(strcmp(var,"ext")==0) { 
	    cmdtbl_replace(var,cond,value);
	 }
	 if(strcmp(var,"env")==0) { 
	    // Set or alter an env file based on another setting
	    // Useful to bump up TOS versions, for instance
	    if(env_cmp(cond))
	      env_set(value);
	 }
	 if(strcmp(var,"opt")==0) {
	    strcpy(envpat, cond);
//	    hss_index(value,lb,2,'|');
	    for(i=0;i<hss_count(value,';');i++) {
	       hss_index(pval,value,i,';');
//	       printf("eval i=%d  pval=%s\n",i,pval);
	       hss_index(cmdopt,pval,1,'=');
	       hss_index(optname,pval,0,'=');
	       cmdtbl_new(optname,envpat,cmdopt);
	    }
	 }
	 if(strcmp(var,"sfq")==0) {
	    cmdtbl_new(value,"<<",cond);
	 }
	 if(strcmp(var,"set")==0) {
	    cmdtbl_new(value,"<",cond);
	 }
	 if(strcmp(var,"cfg")==0 || strcmp(var,"cfp")==0) {
//	    sprintf(envpat,"CFGFILE=%s" ,value);
	    strcpy(envpat,"");
	    cmd_scanvar(envpat,value);
	    strcpy(emuopt.cfgfile,envpat);
//	    printf("WAHT? %s\n",emuopt.cfgfile);
            if(strcmp(var,"cfg")==0) {
	       emuopt.usecfg='Y';
	       strcpy(cmdopt,"%DISKLOC%%/%");
#ifdef DEBUG
	       printf("DISKLOC: %s\n",emuopt.diskloc);
#endif
	    }
            if(strcmp(var,"cfp")==0) { 
	       emuopt.usecfg='P';
	       if(emuopt.localcfg=='Y') {
		  sprintf(cmdopt,"%s%ccfg",emuopt.cfgdir,mysep);
		  fileio_mkdir_p(cmdopt);
		  strcpy(cmdopt,"%CFGDIR%%/%cfg%/%");
	       } else {
		  sprintf(cmdopt,"%s%ccfg",emuopt.tmpdir,mysep);
		  fileio_mkdir_p(cmdopt);
		  sprintf(cmdopt,"%s%ccfg%c",emuopt.tmpdir,mysep,mysep);
	       }
//   fileio_mkdir_p(emuopt.diskloc);
	    }
//	    sprintf(cmdopt,"\%DISKLOC\%\%/\%");
	    if(strcmp(cond,"local")==0) {
//	       basename(pval,envpat);
//	       printf("basename(%s,%s)\n",pval,envpat);
	       fileio_basename(pval,envpat);
//	       printf("fileio_basename(%s,%s)\n",pval,envpat);
	       strcat(cmdopt,pval);
//	       printf("CFGi: %s::%s\n",cmdopt,pval);
	    } else {
	       strcat(cmdopt,cond);
//	       printf("CFGi: %s\n",cmdopt);
	    }
	    cmdtbl_new("CFGFILE","*",cmdopt);
	 }
      }
   }
   fclose(fp);   
#ifdef DEBUG
   printf("Loaded EMUCFG2 file\n");
   cmdtbl_print();
#endif
}

int mod_loademucfg() {
   char cfgfile[160], emuver[45] ,etc[14];
   // Try version-specific file first
   sprintf(cfgfile,"%s_bin",imenu.emulator);
   env_get(emuver,cfgfile);
   
   strcpy(etc,"etc");
   sprintf(cfgfile,"%s%c%s%s%c%s",basedir,mysep,imenu.sysbase,etc,mysep,emuopt.osname);
   if(fileio_dir_exists(cfgfile))
     sprintf(etc,"etc%c%s",mysep,emuopt.osname);
//   printf("ETC is now %s\n",etc);
      
  // Mew -- attempt to load alternate profiles
   if(imenu.profile == 0 ) {
      sprintf(cfgfile,"%s%c%s%s%cemu_%s.cfg",basedir,mysep,imenu.sysbase,etc,mysep,emuver);
   } else {
      sprintf(cfgfile,"%s%c%s%s%cemu_%s_prf%d.cfg",basedir,mysep,imenu.sysbase,etc,mysep,emuver,imenu.profile);	
   }
#ifdef DEBUG
   printf("TRYING version-specifc CFG File: %s\n",cfgfile);
#endif
   if(file_exists(cfgfile)) {
      mod_loadcfg(cfgfile);
   } else {
      // Fallback, should it default to alternate profile or main?
      if(imenu.profile == 0 )
	sprintf(cfgfile,"%s%c%s%s%cemu_%s.cfg",basedir,mysep,imenu.sysbase,etc,mysep,imenu.emulator);
      else 
	sprintf(cfgfile,"%s%c%s%s%cemu_%s_prf%d.cfg",basedir,mysep,imenu.sysbase,etc,mysep,imenu.emulator,imenu.profile);
      
#ifdef DEBUG
      printf("TRYING CFG File: %s\n",cfgfile);
#endif
      if(file_exists(cfgfile)) 
	mod_loadcfg(cfgfile);
      else
	strcpy(emuopt.cmd_patt,"EMU=%RBIN%:GAME=%RPROG%");
      //  printf("LOOKED FOR %s\n",cfgfile); 
   }
}

int mod_loadsyscfg() {
   char cfgfile[160];
   sprintf(cfgfile,"%s%c%setc%cemu_%s.cfg",basedir,mysep,imenu.sysbase,mysep,imenu.system);
#ifdef DEBUG
   printf("TRYING CFG File: %s\n",cfgfile);
#endif
   if(file_exists(cfgfile)) 
     mod_loadcfg(cfgfile);
   else
     strcpy(emuopt.cmd_patt,"EMU=%RBIN%:GAME=%RPROG%");
}

int mod_loadpergame() {
   FILE *fp;
   char cfgfile[160];
   char lb[260], value[40], var[40], rom[30];
   int coldef, numdef=0,i;
   
//   basename(rom,imenu.game);
   sprintf(cfgfile,"%s%c%setc%cemu_pergame.cfg",basedir,mysep,imenu.sysbase,mysep);
   if(file_exists(cfgfile)) {
      fp=fopen(cfgfile,"rb");
      while(!feof(fp)) {
	 fgets(lb,255,fp);
#ifdef WIN32
	 // remove ^M linefeeds
	 for(i=0;i<strlen(lb);i++) {
	    if(lb[i]==13)
	      lb[i]=0;
	 }
#endif
	 if(feof(fp)) break;
	 if(lb[0]!='#') {
	    mod_colidx(var,lb,0,' ');
	    // If game is found
	    if(strcmp(var,emuopt.uqrom)==0) {
//	       printf(">%s<=>%s< %d\n",var,value,numdef);
	       for(i=1;i<=numdef;i++) {
		  mod_colidx(value,lb,i,' ');
//		  printf("column %d is %s\n",i,value);
		  if(strcmp(value,"-")!=0) {
		     sprintf(rom,"%s=%s",cmdtbl[i+cmdtidx].envpat,value);
		     //		  printf("setting... %s\n",rom);
		     env_set(rom);
		  }
	       }
	    }
	    if(strcmp(var,"COLDEF")==0) {
	       mod_colidx(value,lb,1,' ');
	       coldef=atoi(value);
	       mod_colidx(value,lb,2,' ');
	       strcpy(cmdtbl[coldef+cmdtidx].envpat,value);
	       if(coldef>numdef) numdef=coldef;
//	       printf("%s value:%s\n",rom,value);
	       // Experimental default value
	       mod_colidx(value,lb,3,' ');
	       if(strcmp(value,"")==0) {
		  printf("NO DEFAULT VALUE!\n"); 
	       } else {
		  sprintf(lb,"%s=%s",cmdtbl[coldef+cmdtidx].envpat,value);
//		  printf("DEFAULT VALUE: %s\n",lb);
		  env_set(lb);
	       }
	    }
	 }
      }
      fclose(fp);
   }
}

int mod_getsystem(char *msys, char *msysbase) {
   // Determine system id, while maintaining compatibility
   FILE *fp;
   int i;
   char emuenv[160], lb[260], value[80], var[220];
   
   sprintf(lb,"%s%cetc%cemucd.env",basedir,mysep,mysep);
   dfixsep2(emuenv,lb,0);
#ifdef DEBUG
   printf("EMUENV file is %s (mod_getsystem)\n",emuenv);
#endif
   fp=fopen(emuenv,"rb");
   while(!feof(fp)) {
      fgets(lb,255,fp);
      if(lb[0]!='#') {
#ifdef WIN32
	 for(i=0;i<strlen(lb);i++) {
	    if(lb[i]==13)
	      lb[i]=0;
	 }
#endif
	 hss_index(var,lb,1,'=');
	 strcat(var,"/");
	 for(i=0;i<strlen(var);i++) {
	    if(var[i]=='/' || var[i]=='\\')
	      var[i]=mysep;
	 }
#ifdef DEBUG
	 printf("%s<->%s\n",msysbase,var);
#endif
	 if(strcmp(var,msysbase)==0) {
	    hss_index(value,lb,0,'=');
	    hss_index(msys,lb,0,'_');
#ifdef DEBUG
	   printf("ding: %s\n",msys);
#endif
	 }
      }
   }
   fclose(fp);
}

int mod_searchbin(char *btype,char *fqfile, const char *filen) {
   int i;
   char temps[50];
   
   strcpy(btype,"none");
   for(i=0;i<cmdtidx;i++) {
      if(strcmp(cmdtbl[i].optname,"ext")==0) {
	 // New, we need to be able to handle directories
	 if(strcmp(cmdtbl[i].cmdopt,"N/A")==0)
	   strcpy(temps,filen);
	 else
	   sprintf(temps,"%s.%s",filen,cmdtbl[i].cmdopt);
	 dfixsep2(fqfile,temps,1);
	 //	 printf("trying: %s\n",emuopt.fqrom);
	 if(file_exists(fqfile)) {
	    strcpy(btype,cmdtbl[i].envpat);
	    break;
	 } // end if file_exists
      } // end if
   } // for
}

int mod_exportvars() {
   // set any export vars
   int i;
   char var[20], val1[70], value[110];
   
//   printf("In mod_export()\n");
   for(i=0;i<cmdtidx;i++) {
      if(strcmp(cmdtbl[i].optname,"exp")==0) {
//	 printf("found an exp!\n");
	 if(env_cmp(cmdtbl[i].envpat) || strcmp(cmdtbl[i].envpat,"*")==0) {
	    hss_index(var,cmdtbl[i].cmdopt,0,'=');
	    hss_index(val1,cmdtbl[i].cmdopt,1,'=');
	    strcpy(value,"");
	    cmd_scanvar(value,val1);
//	    printf("env_setxp: %s=%s\n",var,value);
#ifndef WIN32
	    // how do you do this in windows?
	    setenv(var,value,1);
#endif
	    break;
	 }
      }   
   }
}

int mod_getbintype(char *btype) {
   int i;
   char temps[50];
   
//   printf("mod_getbintype: %s\n",emuopt.uqrom);
   if(strncmp(emuopt.uqrom,"CMD",3)==0 || strncmp(emuopt.uqrom,"HARDDRIVE",9)==0) {
      if(strncmp(emuopt.uqrom,"CMD",3)==0) {
	 strcpy(btype,"cmd");
	 sprintf(temps,"%s%cetc%cblank.zip",imenu.sysbase,mysep,mysep);
	 dfixsep2(emuopt.fqrom,temps,1);
      }
      if(strncmp(emuopt.uqrom,"HARDDRIVE",9)==0) {
	strcpy(btype,"hdd");
      }
   } else {
      mod_searchbin(btype,emuopt.fqrom, imenu.game);
   } // if
}

int bin2disk() {
   // This is used to put loose .bin files onto (xfd) disk images for 
   // emulators that can't load .bins directly.  Currently that means any 
   // emulator EXCEPT atari 800
   // 
   // New: dosdisk.xfd needs to be removed prior to running this
   char diskzip[20], util[20],fpath[120],cmd[200],tmpp[120];
   hss_index(diskzip,emuopt.cmd_patt,1,':');
   hss_index(util,emuopt.cmd_patt,2,':');
   sprintf(tmpp,"%s%c%s%cetc%c%s",basedir,mysep,imenu.sysbase,mysep,mysep,diskzip);
   strcpy(fpath,emuopt.fqrom);
   dfixsep2(emuopt.fqrom,tmpp);
   dealwithzip(OVERWRITE);
   if(strcmp(util,"xfd_write")==0) {
      dfixsep2(tmpp,"tools/xfd/xfd_write",1);
      sprintf(cmd,"%s %s AUTORUN.SYS <%s",tmpp,arc[0].name,fpath);
      strcpy(emuopt.fqrom,arc[0].name);
   }
#ifdef DEBUG
   printf("bin2disk command:%s\n",cmd);
#endif
   system(cmd);
}

int process_cmd(char *cmd) {
   int nd,i;
   char dsk[80],type[12],ext[6],tmpp[88];
   
   if(strcmp(cmd,"CMDstdisk")==0 || strcmp(cmd,"CMDstdisk")==0 || strcmp(cmd,"CMDstdisk")==0 || strcmp(cmd,"CMDstdisk")==0) {
      printf("Please use CMDnewdisk from now on!!!\n");
      strcpy(cmd,"CMDnewdisk");
   }
   if(strcmp(cmd,"CMDsteemclean")==0) {
      printf("Please use CMDcleanconf from now on!!!\n");
      strcpy(cmd,"CMDcleanconf");
   }
   if(strcmp(cmd,"CMDsetup")==0) {
      printf("CMD Setup\n");      
//      run_setup("setup1");
      setup_go();
   }
   if(strcmp(cmd,"CMDcleanconf")==0) {
      strcpy(tmpp,"");
      cmd_scanvar(tmpp,"%CFGFILE%");
      printf("config is %s\n",tmpp);
      if(strcmp(tmpp,"")!=0 && file_exists(tmpp)) {
	 printf("removing config file %s\n",tmpp);
	 remove(tmpp);
      }
   }
   
   if(strcmp(cmd,"CMDinsthdi")==0) {
      cmd_insthdi();
   }
   
   if(strcmp(cmd,"CMDinsthdd")==0) {
      cmd_insthdd();
   }
   
   if(strcmp(cmd,"CMDnewdisk")==0) {
      printf("Create new disk\n");
      dealwithzip(NO_OVERWRITE);
      emu_basename(tmpp,arc[0].name);
      hss_index(ext,tmpp,1,'.');
      for(nd=1;nd<12;nd++) {
	 sprintf(dsk,"%s%cblank%02d.%s",emuopt.diskloc,mysep,nd,ext);
	 printf("checking: %s\n",dsk);
//	 mod_searchbin(type,tmpp,dsk);
//	 if(strcmp(type,"none")==0) {
	 if(!file_exists(dsk)) {
	    printf("rename %s %s\n",arc[0].name,dsk);
	    rename(arc[0].name,dsk);
	    break;
	 }
      }
   }
}

/* -------------------------------------------------------------------
 * 
 *  EmuModules start here
 * 
 * ------------------------------------------------------------------- */

int emumodule_generic() {
#ifdef DEBUG
   printf("EMUmodule: generic\n");
#endif
   load_settings();
   mod_loademucfg();
//   env_set("SOUND=4"); test
#ifdef DEBUG
   env_print();
#endif
}

int emumodule_computer() {
   int didcp=0;
#ifdef DEBUG
   printf("EMUmodule: computer\n");
#endif
// Test We need to load env first if versioned configs are to work, is there
// a downside?
   load_settings();
   mod_loademucfg();
//   load_settings();
//   env_load();   
//   env_set("SOUND=4"); test
#ifdef DEBUG
   env_print();
#endif
   // regetting the bintype, in case it changed  should we remove it from
   // earlier?
   mod_getbintype(emuopt.bintype);
#ifdef DEBUG
   printf("bintype #2: %s fqrom: %s\n",emuopt.bintype, emuopt.fqrom);
#endif
   // probably need to set the work dir here, (diskloc), not in dealwithzip
   // Need to create and cd to a subdir named after the system
   // 
   // if we have a local cfg directory, then use it for disks and cfg
   if(emuopt.localcfg=='Y') {
      sprintf(emuopt.diskloc,"%s%cdisks%c%s",emuopt.cfgdir,mysep,mysep,imenu.system);      
   } else {
      sprintf(emuopt.diskloc,"%s%c%s",emuopt.tmpdir,mysep,imenu.system);
   }
//   printf("Disk Dir is %s\n",emuopt.diskloc);
   fileio_mkdir_p(emuopt.diskloc);
//#ifdef WIN32
//   mkdir(emuopt.diskloc); // do a check
//#else
//   mkdir(emuopt.diskloc,S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH); // do a check
//#endif
   if(strcmp(emuopt.bintype,"disk")==0) {
      // Set DISK1 if type=disk
      strcpy(arc[0].name,emuopt.fqrom);
      strcpy(arc[0].type,"disk");
      arc[0].inserted=1;
      env_set("HAVEDISK1=Y");
      printf("arc0 = %s\n",arc[0].name);
   }
   
   if(strcmp(emuopt.bintype,"zip")==0) {
      didcp=1;
      dealwithzip(NO_OVERWRITE);
      // Disk is sorta assumed, this is to deal with other things that may
      // be in the zip
//      printf("testtttt: %s\n",arc[0].type);
      if(strcmp(arc[0].type,"disk")!=0) {
	 strcpy(emuopt.bintype,arc[0].type);
	 strcpy(emuopt.fqrom,arc[0].name);
      }
   }
   
   if(strcmp(emuopt.bintype,"dzip")==0) {
      if (emuopt.cplocal=='Y') { 
	 dealwithzip(NO_OVERWRITE);
	 if(arc[0].inserted==1) {
	    // Best practice?   if we change the type to zip, we can
	    // take advantage of features like multiple disks
	    // if we change the type to whatever is in the zip, and the fqrom
	    // then we can support whatever type is actually in there.   
	    // or we can do both, like this:
	    if(strcmp(arc[0].type,"disk")==0)
	      strcpy(emuopt.bintype,"zip");
	    else {
	       strcpy(emuopt.bintype,arc[0].type);
	       strcpy(emuopt.fqrom,arc[0].name);
	    }
	 }
	 didcp=1;
      } else {
	 strcpy(emuopt.bintype,"disk");
      }
   }
   
//   if(strcmp(emuopt.bintype,"zip")!=0 && strcmp(emuopt.bintype,"dzip")!=0 && emuopt.cplocal=='Y') { 
//      cplocaldisk();
//   }
//   if(strcmp(emuopt.bintype,"zip")!=0 && strcmp(emuopt.bintype,"dzip")!=0) {
   if(emuopt.cplocal=='Y' && didcp==0)  
     cplocaldisk();
//   }

   if(env_isset("BOOTDISK")) {
//      printf("BOOTDSK config detected\n");
      setbootdisk();
   }

//   printf("usecfg=%c\n",emuopt.usecfg);
   if(emuopt.usecfg=='Y' && strcmp(emuopt.bintype,"cmd")!=0) 
     customcfg();
   if(emuopt.usecfg=='P' && strcmp(emuopt.bintype,"cmd")!=0) 
     p_customcfg();
}

int sysmodule_arcade() {
#ifdef DEBUG
   printf("SYSmodule: arcade\n");
#endif
   env_clear();
   mod_loadpergame();
//   mod_loadsyscfg();
   strcpy(emuopt.bintype,"rom");
   emu_basename(emuopt.fqrom,imenu.game);
#ifdef DEBUG
   printf("bintype: %s fqrom: %s\n",emuopt.bintype, emuopt.fqrom);
#endif
   emumodule_generic();
}

int sysmodule_generic() {
  
#ifdef DEBUG
   printf("SYSmodule: generic\n");
#endif
   env_clear();
   mod_loadpergame();
   mod_loadsyscfg();
//   sprintf(emuopt.fqrom,"%s%c%s.bin",basedir,mysep,imenu.game);
//   if(file_exists(emuopt.fqrom))
//     printf("IT exists\n");
//   else     
//     printf("No existo\n");
   mod_getbintype(emuopt.bintype);
#ifdef DEBUG
   printf("bintype: %s fqrom: %s\n",emuopt.bintype, emuopt.fqrom);
#endif
//   strcpy(emuopt.bintype,"cart");
   emumodule_generic();
}

int sysmodule_computer() {
  
#ifdef DEBUG
   printf("SYSmodule: computer\n");
#endif
   env_clear();
   mod_loadpergame();
   mod_loadsyscfg();
//   sprintf(emuopt.fqrom,"%s%c%s.bin",basedir,mysep,imenu.game);
//   if(file_exists(emuopt.fqrom))
//     printf("IT exists\n");
//   else     
//     printf("No existo\n");
//     We are doing this later on, can probably remove this.
   mod_getbintype(emuopt.bintype);
#ifdef DEBUG
   printf("bintype: %s fqrom: %s\n",emuopt.bintype, emuopt.fqrom);
#endif
//   strcpy(emuopt.bintype,"cart");
//   if(strcmp(emuopt.bintype,"cmd")!=0) 
     emumodule_computer();
}

int module_exec() {
   int ransys=0;
   char cddir[80];
   
   emuopt.exec=0;
   
//  printf("OO:mod_getsystem(%s,%s)\n",imenu.system,imenu.sysbase);
   mod_getsystem(imenu.system,imenu.sysbase);
   emu_basename(emuopt.uqrom,imenu.game);
#ifdef WIN32
   strcpy(emuopt.osname,"win32");
#else
   strcpy(emuopt.osname,"linux");
#endif
   
#ifdef DEBUG
   printf("\n+----------------------------------\n");
   printf("| SYSTEM: %s\n",imenu.system);
   printf("| SYSBASE: %s\n",imenu.sysbase);
   printf("| EMULATOR: %s\n",imenu.emulator);
   printf("| PROGRAM: %s\n",imenu.game);
   printf("| ROM: %s\n",emuopt.uqrom);
   printf("| OS: %s\n",emuopt.osname);
   printf("| TYPE: %s\n",emuopt.bintype);
   printf("+----------------------------------\n\n");
#endif
   // Do we need to parse the system info out of emucd.env?   is it useful?
   // 
   // Check for certain systems, else execute generic
   
   // I don't know what happened to cmd processing, but let's try adding it here
   if(strncmp(imenu.game,"CMD",3)==0) {
      strcpy(emuopt.bintype,"cmd");
      strcpy(emuopt.uqrom,imenu.game);
      ransys=1;
   }  
   
   strcpy(emuopt.diskloc,"n/a");      
   if(strcmp(imenu.system,"arcade")==0) {
      sysmodule_arcade();
      ransys=1;
   }
   if(strcmp(imenu.system,"a800")==0 || strcmp(imenu.system,"ST")==0 || 
      strcmp(imenu.system,"Amiga")==0 || strcmp(imenu.system,"c64")==0 ||
      strcmp(imenu.emulator,"gens")==0) {
      sysmodule_computer();
      ransys=1;
   }
   if(ransys==0) {
      sysmodule_generic();
   }
//   build_cmd0();
   if(strcmp(emuopt.bintype,"cmd")==0) {
      printf("PROCESSCOMMAND: %s\n",emuopt.uqrom);
      process_cmd(emuopt.uqrom);
      return(0);
   }
//   SDL_QuitSubSystem(SDL_INIT_VIDEO); // shutdown gfx
   settxtmode();
   cmd_getwd(cddir);
   build_cmd();
   if(emuopt.exec==1 && imenu.noexec<1) {
//     set_gfx_mode(4,640,480,0,0);
      if(strcmp(cddir,"")!=0) {
	 printf("CCHDIR:%s\n",cddir);
	 chdir(cddir);
      }
      mod_exportvars();
      printf("CUSECMD:%s\n", emuopt.cmd_line);
      system(emuopt.cmd_line);
      mod_cleantmp();
      strcpy(emuopt.cmd_line,"");
      emuopt.exec=0;
   } else {
      if(strcmp(cddir,"")!=0) 
	printf("CHDIR:%s\n",cddir);
      printf("USECMD:%s\n", emuopt.cmd_line);
//     Think this was supposed to prevent bookmark writing in test mode
      if(imenu.noexec==1) {
	 mod_writebm(0);
	 exit(0);
      }
   }
   cmdtbl_clear();
   //   env_clear();
   // SDL_InitSubSystem(SDL_INIT_VIDEO);// restart gfx
   if(imenu.noexec!=2)
     setgfxmode(); // restart gfx
}
