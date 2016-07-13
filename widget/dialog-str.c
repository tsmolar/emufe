#include <stdio.h>
#include <string.h>

// #include <time.h>
// #include "gwtypes.h"

// String functions that can be useful to a wide range of programs.
// They should have a base library of their own

dss_cutstr(char *sstr,const char *istr,int start, int len) {
   // len is length of string, not end point
   int i,o=0;

   for(i=start;i<start+len;i++) {
      sstr[o]=istr[i];
      o++;
   }
   sstr[start+len]=0;
}

int dss_count(const char *ostr, const char del) {
   int rv=1,i;
   for(i=0;i<strlen(ostr);i++) {
      if(ostr[i]==del) rv++;
   }
   return(rv);
}

dss_index(char* rstr, char *ostr, int idx, char del) {
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

int dss_getindex(char *list, const char *mstr, char del) {
   // return the index of a substr
   int i, r=-1;
   char ws[80];
   
   for(i=0;i<dss_count(list,del);i++) {
      hss_index(ws,list,i,del);
      if(strcmp(mstr,ws)==0) {
	 r=i;
	 break;
      }
   }
   return r;
}
