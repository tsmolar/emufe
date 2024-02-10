#include<SDL.h>
#include"sdl_allegro.h"

int num_joysticks;
JOYSTICK_INFO joy[8];

// Joystick section
void install_joystick(int type)  {
   int i;
   num_joysticks=SDL_NumJoysticks();
   for(i=0;i<num_joysticks;i++) {
      joy[i].handle=SDL_JoystickOpen(i);
      if(joy[i].handle) {
	 joy[i].stick[0].num_axis=SDL_JoystickNumHats(joy[i].handle);
//	 joy[i].stick[0].num_axis=SDL_JoystickNumAxes(joy[i].handle);
	 joy[i].num_buttons=SDL_JoystickNumButtons(joy[i].handle);
	 joy[i].num_sticks=1;
      }
   }
   SDL_JoystickEventState(SDL_QUERY);
}
// END Joystick section

void poll_joystick() {
   Uint8 stick;
   int axis;
   SDL_JoystickUpdate();
   int i,b,b2;

   // poll sticks
   for(i=0;i<num_joysticks;i++) {
      // poll buttons
      for(b=0;b<joy[i].num_buttons;b++) {
	 // Try to make the button mapping the same as allegro
	 if(b==0) b2=2;  if(b==1) b2=3;
	 if(b==2) b2=0;  if(b==3) b2=1;
	 joy[i].button[b2].b=SDL_JoystickGetButton(joy[i].handle, b);
//	 if(joy[i].button[b2].b)
//	   printf("stick %d button %d pressed\n",i,b2);
      }
      stick=SDL_JoystickGetHat(joy[i].handle, 0);
//if(stick == SDL_HAT_UP) printf("hat up!\n");
//if(stick == SDL_HAT_DOWN) printf("hat down!\n");
      if(stick == SDL_HAT_CENTERED) {
	 joy[i].stick[0].axis[0].d1=0;
	 joy[i].stick[0].axis[0].d2=0;
	 joy[i].stick[0].axis[1].d1=0;
	 joy[i].stick[0].axis[1].d2=0;
      }


      axis=SDL_JoystickGetAxis(joy[i].handle, 0);
      joy[i].stick[0].axis[0].d1=joy[i].stick[0].axis[0].d2=0;
      if(axis>0) joy[i].stick[0].axis[0].d2=1;
      if(axis<0) joy[i].stick[0].axis[0].d1=1;

      axis=SDL_JoystickGetAxis(joy[i].handle, 1);
      joy[i].stick[0].axis[1].d1=joy[i].stick[0].axis[1].d2=0;
      if(axis>0) joy[i].stick[0].axis[1].d2=1;
      if(axis<0) joy[i].stick[0].axis[1].d1=1;
//      printf("stick=%d\n",axis);

      if(stick == SDL_HAT_LEFT) joy[i].stick[0].axis[0].d1=1;
//      else joy[i].stick[0].axis[0].d1=0;
      if(stick == SDL_HAT_RIGHT) joy[i].stick[0].axis[0].d2=1;
//      else joy[i].stick[0].axis[0].d2=0;
      
      if(stick == SDL_HAT_UP) joy[i].stick[0].axis[1].d1=1;
      if(stick == SDL_HAT_DOWN) joy[i].stick[0].axis[1].d2=1;

      
// DEBUG INFO      
//      if(joy[i].stick[0].axis[0].d1) printf("*left?\n");
//      if(joy[i].stick[0].axis[0].d1) printf("*left? %d\n",joy[i].stick[0].axis[0].d1);
//      if(joy[i].stick[0].axis[0].d2) printf("right?\n");
      
//      if(stick | SDL_HAT_UP) joy[i].stick[0].axis[1].d1=1;
//      else joy[i].stick[0].axis[1].d1=0;
//      if(stick | SDL_HAT_DOWN) joy[i].stick[0].axis[1].d2=1;
//      else joy[i].stick[0].axis[1].d2=0;
      
//      if(joy[i].stick[0].axis[1].d1) printf("  up?\n");
//      if(joy[i].stick[0].axis[1].d2) printf("down?\n");
   }
//   if(joy[0].stick[0].axis[1].d1) printf("*up? %d\n",joy[0].stick[0].axis[1].d1);
}
