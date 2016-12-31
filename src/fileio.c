#include <stdio.h>
#include <sys/stat.h>
#include <string.h>

#include <dirent.h>
#ifdef WIN32
#include <windows.h>
#endif

extern char mysep;

int fileio_dir_exists(const char *mydir) {
#ifdef WIN32
   int rv;
   WIN32_FIND_DATA data;
   char mytmpdir[222];
   sprintf(mytmpdir,"%s\\*",mydir);
   
   HANDLE handle = FindFirstFile(mytmpdir,&data);
   if(handle == INVALID_HANDLE_VALUE) 
     rv=0;
   else
     rv=1;
//   printf("direxists?%d   %s\n",rv,mytmpdir);
   return (rv);
#else
   DIR *dr;
   int rv;
   if(dr=opendir(mydir)) {
      rv=1;
      closedir(dr);
//      printf("exists! %s\n",mydir);
   } else {
      rv=0;
//      printf("not exists! %s\n",mydir);
   }
   return(rv);
#endif
}

int fileio_file_exists(char *myfile) {
   // This may need to be different under Windows or some unixes
   struct stat buffer;
   return (stat (myfile, &buffer) == 0);
}

int fileio_dirname(char *dname, char *fpath) {
   int i,li=0;
   for(i=0;i<strlen(fpath);i++) {
      if(fpath[i]==mysep) li=i;
   }
   if(li==0) 
     strcpy(dname,"");
   else {
      for(i=0;i<li;i++) dname[i]=fpath[i];
      dname[li]=0;
     }
   //printf("dirname: idx is %d, string was %s\n",li,fpath);
   //printf("dirname is >>%s<<\n",dname);
}

int fileio_basename(char *dname, char *fpath) {
   int i,li=0;
   // search for both / and \ instead of mysep, no particular reason for
   // doing this, can be switched to mysep if it causes issues.
   for(i=0;i<strlen(fpath);i++) {
      if(fpath[i]=='/' || fpath[i]=='\\') li=i;
   }
   if(li==0) 
     strcpy(dname,fpath);
   else {
      for(i=li;i<strlen(fpath);i++) dname[i]=fpath[i];
      dname[i-li]=fpath[i];
     }
   dname[strlen(fpath)]=0;
   //printf("dirname: idx is %d, string was %s\n",li,fpath);
   //printf("dirname is >>%s<<\n",dname);
}

int fileio_rmdir(const char *mydir) {
      rmdir(mydir); // do a check
}

int fileio_rm(const char *myfile) {
   unlink(myfile);
}

int fileio_mkdir_p(const char *mydir) {
   char basedir[222];
   if(fileio_dir_exists(mydir)==0) {
      fileio_dirname(basedir,mydir);   
//      printf("dir not exist %s,  dirname: %s\n",mydir,basedir);
      fileio_mkdir_p(basedir);
      printf("mkdir %s\n",mydir);
#ifdef WIN32
      mkdir(mydir); // do a check
#else
      mkdir(mydir,S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH); // do$
#endif
      if(fileio_dir_exists(mydir)) {
//	 printf("mkdir successful!\n");
	 return(1);
      } else {
//	 printf("mkdir notsuccessful!\n");
	 return(0);
      }
   }
}

int fileio_cp(char *src, char *dest) {
   FILE *in, *out;
   char ch;
   
   if((in=fopen(src, "rb")) == NULL) {
      printf("Cannot open input file.\n");
      return 1;
   }
   if((out=fopen(dest, "wb")) == NULL) {
      printf("Cannot open output file.\n");
      return 2;
   }
   
   while(!feof(in)) {
      ch = getc(in);
      if(ferror(in)) {
	 printf("Read Error");
	 clearerr(in);
	 break;
      } else {
	 if(!feof(in)) putc(ch, out);
	 if(ferror(out)) {
	    printf("Write Error");
	    clearerr(out);
	    break;
	 }
      }
   }
   fclose(in);
   fclose(out);
   
   return 0;
}

int fileio_mv(const char *src, const char *dest) {
   fileio_cp(src,dest);
   fileio_rm(src);
}
