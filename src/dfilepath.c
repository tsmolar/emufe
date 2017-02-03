#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "emufe.h"
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifndef PACKAGE
#define PACKAGE emufe
#endif

// Path separators
#ifdef WIN32
   char mysep='\\';
#else
   char mysep='/';
#endif

int dfp_index(char* rstr, char *ostr, int idx, char del) {
   // Alternative to strtok
   // For some reason, can't use hss_index in here.
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

int abs_dirname(char *paath, char *bin) {
   
   // This mess will try to return the absolute pathname for any directory
   // passed.   absolute, meaning that .'s are translated.   The only things
   // not currently translated are ~ and ..
   // 
   char seps[4], path[160],strs[200], lcarp[40], *carp=NULL;
   
   strcpy(strs,bin);
   strcpy(seps,"/\\");
   strcpy(path,"");  // Clear path, seems to have problems if you don't

   if(bin[0]==mysep) { 
      sprintf(path,"%c",mysep);
   }
   carp=strtok(strs,seps);
   
   while(carp != NULL) {
      strcpy(lcarp,carp);
      carp=strtok(NULL,seps);
      if(carp!=NULL) {
	 if(strcmp(lcarp,".")==0) {
	   getcwd(path,sizeof(path));
	 } else {
#ifdef WIN32
	    if(bin[1]!=':') 
#else
	    if(path[0]!=mysep)
#endif
	      {
		  getcwd(path,sizeof(path));
	       } 
#ifdef WIN32
	    if(strlen(path)<2) {
	       strcpy(path,lcarp);
#else
	    if(strcmp(path,"/")==0) {
	       sprintf(path,"/%s",lcarp);
#endif
	       } else
		 sprintf(path,"%s%c%s",path,mysep,lcarp);
	 } 
      }
   }
   strcpy(paath,path);
}

int emu_basename(char *paath, char *bin) {
   
   // basename   
   char seps[4], strs[200], *carp=NULL;
   strcpy(strs,bin);
   strcpy(seps,"/\\");

   carp=strtok(strs,seps);
   
   while(carp != NULL) {
      carp=strtok(NULL,seps);
      if(carp!=NULL) {
	 strcpy(paath,carp);
      }
   }
}

void find_datadir(char *ddir, char *bin) {
   char path[160],dirname[160];

   abs_dirname(path,bin);
   if(strcmp(path,"")==0) {
      strcpy(path,INSTPREFIX);
   }
   emu_basename(dirname,path);
   // Probably don't want the gamename hardcoded here
   // This needs to be influenced by an env var, probably EMUHOME
//   if(strcmp(dirname,"bin")==0 || strcmp(dirname,"src")==0) {
   if(strcmp(dirname,"src")==0) {
      abs_dirname(dirname,path);
      sprintf(ddir,"%s%cshare%c%s",dirname,mysep,mysep,PACKAGE);
   } else {
//      sprintf(ddir,"%s%cshare%c%s",path,mysep,mysep,PACKAGE);
      sprintf(ddir,"%s%c",path,mysep);
   }
   if(strcmp(dirname,"bin")==0 ) {
      abs_dirname(dirname,path);
      printf("dirname is now %s\n",dirname);
     sprintf(ddir,"%s/",dirname);
   }
   //   if(getenv("CDROOT"))
   //     sprintf(ddir,"%s/",getenv("CDROOT"));
#ifdef DEBUG
   LOG(4, ("find_datadir() ddir is now: %s cdroot is >%s<\n",ddir,cdroot));
#endif
   if(strcmp(cdroot,"") != 0)
     sprintf(ddir,"%s/",cdroot);
#ifdef DEBUG
   LOG(4, ("find_datadir() ddir is now+: %s \n",ddir));
#endif
   //     strcpy(ddir,getenv("CDROOT"));
}

void getnxtpath(char *opath, char *ipath) {
   // experimental recursive path straightener
   int i,nsep=0, fsep=0;
   char front[30],back[90],combo[120],tcomp[30];
   
////   printf("getnxt: %s\n",ipath);
   for(i=0;i<strlen(ipath);i++) {
      if(ipath[i]==mysep) { 
	 nsep++;
//	 printf("%c at %d\n",mysep,i);
	 if(fsep==0) fsep=i+1;
      }
   }
////   printf("gnpath: fsep:%d   nsep:%d\n",fsep,nsep);
   if(nsep>0) {
      dfp_index(front,ipath,0,mysep);
//      for(i=1;i<nsep;i++) {
//	 printf("d1\n");
      strcpy(back,ipath+fsep);
//	 printf("d2\n");
//      }
      
      if(strcmp(front,".")==0) {
	 getnxtpath(combo,back);
	 strcpy(opath,combo);
      }
      
      if(strcmp(front,"..")==0) { 
	 getnxtpath(combo,back);
	 sprintf(opath,"+-%s",combo);
      }
      if(strcmp(front,".")!=0 && strcmp(front,"..")!=0) {      
////	 printf("Front: %s  Back: %s  op: %s\n",front,back,ipath);
	 getnxtpath(combo,back);
	 if(combo[0]=='+') {
	    sprintf(opath,"%s",combo+2);
	 } else {
////	    printf("Combo: %s\n",combo);
	    sprintf(opath,"%s%c%s",front,mysep,combo);
	 }
      }
   } else {
      sprintf(opath,"%s",ipath);
   }   
}
      
void dfixsep2(char *opath, char *ipath, int setfq) {
   int i,i2,nsep;
//#ifdef WIN32
   char tdir[122],tcomp[30];
//   basename(tempd,ipath);
//   basename(tdir,ipath);
//   abs_dirname(tempd,ipath);
//   sprintf(opath,"%s%c%s",tempd,mysep,tdir);
   // not yet
   if(setfq==1)
      sprintf(tdir,"%s%c%s",basedir,mysep,ipath);
   else
      strcpy(tdir,ipath);
#ifdef DEBUG
   LOG(2, ("tdir is %s\n",tdir));
#endif
   //#else
//   strcpy(opath,ipath);
//#endif
   nsep=0;
   for(i=0;i<strlen(tdir);i++) {
      if(tdir[i]=='/') {
	 tdir[i]=mysep;
	 if(tdir[i+1]==mysep) {
	    // Remove double path separator
	    for(i2=i+1;i2<strlen(tdir);i2++)
	      tdir[i2]=tdir[i2+1];
	 }
      }
   }
//   printf("nsep: %d  %s\n",nsep,opath);
   // experimental
   getnxtpath(opath,tdir);
#ifdef DEBUG
   LOG(2, ("**** opath: %s %s\n",opath,tdir));
#endif
}
