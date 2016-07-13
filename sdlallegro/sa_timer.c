#include<SDL.h>
#include"sdl_allegro.h"

/* Internal Use Only */
typedef struct timerar_t {
   int id;
   int (*name)();
   int inuse; // Test to see if it's in use
} timerar_t;

timerar_t timerar[16];

int sa_reg_timer(int tid, int (*tname)()) {
   // Throws timer info into array for tracking
   int i, rv=-1;
   for(i=0;i<16;i++) {
      if( timerar[i].inuse!=44 ) {
	 timerar[i].id=tid;
	 timerar[i].inuse=44;
	 timerar[i].name=tname;
	 rv=0;
	 break;
      }
   }
   return(rv);
}

int sa_lookup_timer(int (*tname)()) {
   // returns the ID of a timer name, or -1
   int i, rv=-1;
   for(i=0;i<16;i++) {
      if( timerar[i].name==tname )
	rv=timerar[i].id;
   }
   return(rv);
}

int sa_unreg_timer(int tid) {
   // Frees up timer from array
   int i, rv=-1;
   for(i=0;i<16;i++) {
      if( timerar[i].id==tid ) {
	 timerar[i].id=0;
	 timerar[i].inuse=0;
	 rv=0;
      }
   }
   return(rv);
}

/* End Internal Use */

/* Timer Routines */
void rest(unsigned int itime) {
   if(itime==0) 
     SDL_WaitEvent(NULL);
   else
     SDL_Delay(itime);
}

int install_timer() {
   // Not needed by SDL
   return 0;
}

int remove_timer() {
   // Not needed by SDL
   return 0;
}

// Note:  A big difference between SDL and Allegro is that SDL seems
//        to require parameters for the callbacks.   Not sure if there's
//        a way to make that platform neutral.

int install_int(void (*proc)(),int speed) {
   int tid,rv;
   tid=sa_lookup_timer(proc);
   if(tid>-1) {
//      printf("attempting to remove timer %d\n",tid);
      rv=SDL_RemoveTimer(tid);
      rv=sa_unreg_timer(tid);
   }
   tid=SDL_AddTimer(speed, proc, 0);
   rv=sa_reg_timer(tid, proc);
//   printf("installing...  int : %d\n",tid);
   return(rv);
}

int remove_int(void (*proc)()) {
   int tid,rv=0;
   tid=sa_lookup_timer(proc);
   rv=SDL_RemoveTimer(tid);
   rv=sa_unreg_timer(tid);
   return(rv);
}
