#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include<SDL.h>
#include"sdl_allegro.h"
//#ifdef HAVE_SDLMIXER
//#include"SDL_mixer.h"
//#endif

#define SDL_ALLEGRO_VERS_C "0.6.99"

SDL_PixelFormat *sdlpixfmt;
SDL_Surface *screen;
#ifdef SDL2
SDL_Window *sdlWindow;
SDL_Renderer *sdlRenderer;
#endif
int sa_depth;
char sdldriver[20];
char sa_win_title[256]; 
#ifdef ZAURUS
int zcomp=0;
#endif
//#ifdef ZAURUS
//int zcomp=0;
//#endif
//extern int num_joysticks;

void sa_version_info() {
#ifdef SDL1
   const SDL_version * v = SDL_Linked_Version();
   
   printf("SDL compile-time version: %d.%d.%d\n",SDL_MAJOR_VERSION,SDL_MINOR_VERSION,SDL_PATCHLEVEL);
   printf("SDL run-time version: %d.%d.%d\n",v->major,v->minor,v->patch);
   printf("SDL/Allegro Library Version %s\n",SDL_ALLEGRO_VERS_C);
   printf("Compiled with legacy SDL1, consider upgrading to SDL2\n");
#endif
#ifdef SDL2
   SDL_version compiled;
   SDL_version linked;
  
   SDL_VERSION(&compiled);
   SDL_GetVersion(&linked);
   printf("SDL compile-time version: %d.%d.%d\n",compiled.major,compiled.minor,compiled.patch);
   printf("SDL linked version: %d.%d.%d\n",linked.major,linked.minor,linked.patch);
   printf("Compiled with SDL2\n");
#endif
#ifdef HAVE_LIBSDL_IMAGE
   printf("  * Have SDLImage for file loading\n");
#else
   printf("  * Not Using SDLImage\n");
#endif
#ifdef HAVE_LIBSDL_MIXER
   printf("  * Have SDLMixer for audio\n");
#else
   printf("  * Not Using SDLMixer\n");
#endif
#ifdef ZAURUS
   printf("  * Zaurus touchscreen compensation for bvdd/SDL\n");
#endif
}

void sa_sdl_info() {
#ifdef SDL1
   const SDL_VideoInfo* xx=SDL_GetVideoInfo();
   printf("SDL Hardware info\n");
   printf("-----------------\n");
   printf("Can create hw surface?  %d\n",xx->hw_available);
   printf("Window Manager available?  %d\n",xx->wm_available);
   printf("Hardware->H Blits accellerated?  %d\n",xx->blit_hw);
   printf("Hardware->H CC Blits accellerated?  %d\n",xx->blit_hw_CC);
   printf("Hardware->H Alpha Blits accellerated?  %d\n",xx->blit_hw_A);
   printf("Software->H Blits accellerated?  %d\n",xx->blit_sw);
   printf("Software->H CC Blits accellerated?  %d\n",xx->blit_sw_CC);
   printf("Software->H Alpha Blits accellerated?  %d\n",xx->blit_sw_A);
   printf("Fills accellerated?  %d\n",xx->blit_fill);
#endif
# ifdef SDL2
   printf("SDL Hardware info (SDL2)\n");
   printf("------------------------\n");
   printf("OS:   %s\n",SDL_GetPlatform());
   printf("CPUs:   %d\n",SDL_GetCPUCount());
#endif
}

void sa_getmodes() {
   # ifdef SDL1
   SDL_Rect **modes;
   int i;
   
//   modes=SDL_ListModes(NULL, SDL_FULLSCREEN|SDL_HWSURFACE);
   modes=SDL_ListModes(NULL,SDL_FULLSCREEN);
   /* Check if there are any modes available */
   if(modes == (SDL_Rect **)0) {
      printf("No modes available!\n");
      exit(-1);
   }
   
   /* Check if our resolution is restricted */
   if(modes == (SDL_Rect **)-1) {
      printf("All resolutions available.\n");
   } else {
      /* Print valid modes */
      printf("Available Modes\n");
      for(i=0;modes[i];++i)
	printf("  %d x %d\n", modes[i]->w, modes[i]->h);
   }
   # endif
   # ifdef SDL2
   printf(" TODO: sa_getmodes() is not yet implemented for SDL2\n");
   # endif   
}

void sa_surface_info(SDL_Surface *asurf, char *name) {
   SDL_PixelFormat *fmt;
   fmt=asurf->format;

#ifdef SDL1
   printf("\n%s SDL Surface Info\n",name);
   printf("----------------------------------\n");
   printf("Bits Per Pixel: %d\n",fmt->BitsPerPixel);
   printf("colorkey: %d\n",fmt->colorkey);
   printf("alpha: %d\n",fmt->alpha);
   printf("flags: %d\n",asurf->flags);
   printf("clip: x:%d y:%d w:%d h:%d\n",asurf->clip_rect.x,asurf->clip_rect.y,asurf->clip_rect.w,asurf->clip_rect.h);
   printf("SDL_SRCALPHA: %d\n",SDL_SRCALPHA);
#endif
#ifdef SDL2
   printf("\n%s SDL Surface Info\n",name);
   printf("----------------------------------\n");
   printf("Bits Per Pixel: %d\n",fmt->BitsPerPixel);
#endif
}

int allegro_init() {
   char driver[21];
   int rv;
//   printf("sdl_allegro version: %s  header: %s \n",SDL_ALLEGRO_VERS_C,SDL_ALLEGRO_VERS_H);
   SA_AUTOUPDATE=1;
   strcpy(sa_win_title,"sdlallegro App");
   rv=SDL_Init(SDL_INIT_EVERYTHING);
#ifdef SDL1 
   SDL_VideoDriverName(sdldriver,19);
#endif
#ifdef SDL2
   //sdldriver = SDL_GetCurrentVideoDriver();
   // if the above fails, set this:
    sprintf(sdldriver,"SDL2-unknown");
#endif
//   SDL_VideoDriverName(driver,20);
#ifdef DEBUG
   printf("Video driver in use is %s\n",sdldriver);
#endif
#ifdef ZAURUS
   // Aparently, only the bvdd SDL suffers this problem
   if(strcmp(sdldriver,"bvdd")==0) {
      zcomp=1;
      printf("Enabling touchscreen compensation\n");
   }
#endif
//   By default, update rects will be done automatically
   atexit(SDL_Quit);
   return rv;
}


//
//  KEYBOARD SECTION
//
void install_keyboard()  {
#ifdef SDL1
   SDL_EnableUNICODE(1);
#endif
//   printf("install_keyboard\n");
}

void remove_keyboard() {
}

void clear_keybuf() {
   SDL_Event event;
   while(SDL_PollEvent(&event)) 
     {
     }
   
}

void set_keyboard_rate(int delay, int repeat) {
#ifdef SDL1
   SDL_EnableKeyRepeat(delay,repeat);
#endif
#ifdef SDL2
   printf("set_keyboard_rate() is not implemented for SDL2 (yet!)\n");
#endif
}

void simulate_keypress(int key) {
   SDL_Event event;
//   SDL_KeyboardEvent keyevent;
//   printf("GSK:%d\n",key); 
    
   event.type=SDL_KEYDOWN;
   event.key.type=SDL_KEYDOWN;
   event.key.state=SDL_PRESSED;
   event.key.keysym.sym=(key>>8);
   SDL_PushEvent(&event);
   event.type=SDL_KEYUP;
   event.key.type=SDL_KEYUP;
   event.key.state=SDL_RELEASED;
   SDL_PushEvent(&event);
}

int keypressed() {
   SDL_Event event;
   poll_mouse();
   if(SDL_PollEvent(&event) && event.type==SDL_KEYDOWN) {
      SDL_PushEvent(&event);
      //   if(SDL_PeepEvents(&event,1, SDL_PEEKEVENT, SDL_KEYDOWN)>0)
      return(1);
   } else
     return(0);
}

int readkey() {
   SDL_Event event;
#ifdef SDL1
   int key=0,key2;
   
   while(SDL_PollEvent(&event)) {
      if (event.type==SDL_KEYDOWN) {
	 key_shifts=0;
	 key=event.key.keysym.sym;
	 key=key<<8;

	 key=key+(event.key.keysym.unicode & 0x7F);
	 if(event.key.keysym.mod & KMOD_SHIFT) key_shifts=(key_shifts | KB_SHIFT_FLAG); 
	 if(event.key.keysym.mod & KMOD_CTRL) key_shifts=(key_shifts | KB_CTRL_FLAG); 
	 if(event.key.keysym.mod & KMOD_ALT) key_shifts=(key_shifts | KB_ALT_FLAG); 
	 if(event.key.keysym.mod & KMOD_LMETA) key_shifts=(key_shifts | KB_LWIN_FLAG); 
	 if(event.key.keysym.mod & KMOD_RMETA) key_shifts=(key_shifts | KB_RWIN_FLAG); 
	 if(event.key.keysym.mod & KMOD_NUM) key_shifts=(key_shifts | KB_NUMLOCK_FLAG); 
	 if(event.key.keysym.mod & KMOD_CAPS) key_shifts=(key_shifts | KB_CAPSLOCK_FLAG); 
	 if(event.key.keysym.mod & KMOD_MODE) key_shifts=(key_shifts | KB_MENU_FLAG); 
      }
      if (event.type==SDL_KEYUP) {
	 break;
      }
   }
#endif
#ifdef SDL2
   int key=0,key2=0,sc,uc;
   
   while(SDL_PollEvent(&event)) {
      if (event.type==SDL_KEYDOWN) {
	 key_shifts=0;
	 key=event.key.keysym.sym;
	 sc=event.key.keysym.scancode;
         uc=key;
	 
	 // translations for SDL1 compatability follow
	 // function keys
	 if(sc>=58 && sc<=69) {
	    uc=0;
	    key=sc+224;
	 }
	 // shift,control, etc
	 switch(sc) {
	  case 225: key=304; uc=0; break;
	  case 229: key=303; uc=0; break;
	  case 224: key=306; uc=0; break;
	  case 227: key=311; uc=0; break;
	  case 226: key=308; uc=0; break;
	  case 230: key=307; uc=0; break;
	  case 231: key=312; uc=0; break;
	  case 101: key=319; uc=0; break;
	  case 228: key=305; uc=0; break;
	    // Arrow section
	  case 71: key=302; uc=0; break;
	  case 72: key=19; uc=0; break;
	  case 73: key=277; uc=0; break;
	  case 74: key=278; uc=0; break;
	  case 75: key=280; uc=0; break;
	  case 76: key=127; uc=127; break;
	  case 77: key=279; uc=0; break;
	  case 78: key=281; uc=0; break;
	  case 82: key=273; uc=0; break;
	  case 80: key=276; uc=0; break;
	  case 81: key=274; uc=0; break;
	  case 79: key=275; uc=0; break;
	    // keypad
	  case 83: key=300; uc=0; break;
	  case 84: key=267; uc=47; break;
	  case 85: key=268; uc=42; break;
	  case 86: key=269; uc=45; break;
	  case 95: key=263; uc=0; break;
	  case 96: key=264; uc=0; break;
	  case 97: key=265; uc=0; break;
	  case 87: key=270; uc=43; break;
	  case 92: key=260; uc=0; break;
	  case 93: key=261; uc=0; break;
	  case 94: key=262; uc=0; break;
	  case 89: key=257; uc=0; break;
	  case 90: key=258; uc=0; break;
	  case 91: key=259; uc=0; break;
	  case 88: key=271; uc=13; break;
	  case 98: key=256; uc=0; break;
	  case 99: key=266; uc=0; break;
	 }
	
	 key=key<<8;
	 
	 key=key+uc;
	 if(event.key.keysym.mod & KMOD_SHIFT) key_shifts=(key_shifts | KB_SHIFT_FLAG);
	 if(event.key.keysym.mod & KMOD_CTRL) key_shifts=(key_shifts | KB_CTRL_FLAG);
	 if(event.key.keysym.mod & KMOD_ALT) key_shifts=(key_shifts | KB_ALT_FLAG);
	 if(event.key.keysym.mod & KMOD_LGUI) key_shifts=(key_shifts | KB_LWIN_FLAG); 
	 if(event.key.keysym.mod & KMOD_RGUI) key_shifts=(key_shifts | KB_RWIN_FLAG); 
	 if(event.key.keysym.mod & KMOD_NUM) key_shifts=(key_shifts | KB_NUMLOCK_FLAG);
	 if(event.key.keysym.mod & KMOD_CAPS) key_shifts=(key_shifts | KB_CAPSLOCK_FLAG);
	 if(event.key.keysym.mod & KMOD_MODE) key_shifts=(key_shifts | KB_MENU_FLAG);
      }
      
      if (event.type==SDL_KEYUP) {
	 break;
      }
   }	 
#endif
   return(key);
      
}
// END Keyboard section

// Joystick section
//install_joystick(int type)  {
//   num_joysticks=SDL_NumJoysticks(void);
//   //   printf("install_joystick stub\n");
//}
// END Joystick section

void set_color_depth(int depth)  {
//   printf("set_color_depth\n");
   sa_depth=depth;
}

//
// Mouse Section
//

void install_mouse()  {
//   printf("install_mouse stub\n");
}
void scare_mouse() {
//   printf("scare_mouse stub\n");
}
void show_mouse(BITMAP *bmp) {
//   printf("scare_mouse stub\n");
}
void unscare_mouse() {
//   printf("scare_mouse stub\n");
}

int mouse_needs_poll() {
   return(1);
}

int poll_mouse() {
   int const num = 32;
   SDL_Event event, events[num];
   mouse_b=0;
   SDL_PumpEvents();
   SDL_GetMouseState(&mouse_x,&mouse_y);
   int count, i;
//#ifdef ZAURUS
//   if(zcomp==1)
//     mouse_y=mouse_y+33+(mouse_y/80);
//#endif
   // These should be bitwise!
   if(SDL_GetMouseState(NULL, NULL)&SDL_BUTTON(1))
     mouse_b=1;
   if(SDL_GetMouseState(NULL, NULL)&SDL_BUTTON(3))
     mouse_b=2;
   if(SDL_GetMouseState(NULL, NULL)&SDL_BUTTON(2))
     mouse_b=4;
   // I took this code off the internet
#ifdef SDL1
   count = SDL_PeepEvents(events, num, SDL_GETEVENT, SDL_EVENTMASK(SDL_MOUSEBUTTONDOWN));

   for( i = 0; i<count;++i) {
      if(events[i].button.button == SDL_BUTTON_WHEELUP)
	{ mouse_z--; }
      else if(events[i].button.button == SDL_BUTTON_WHEELDOWN)
	{ mouse_z++; }
   }
   
   SDL_PeepEvents(events, num, SDL_GETEVENT, SDL_EVENTMASK(SDL_MOUSEBUTTONUP) );
#endif
   // End Internet code
#ifdef SDL2
   // I adopted this from the SDL1 code, not sure if this is the best way to read the mouse
   // wheel under SDL2, it feels a little laggy
   count = SDL_PeepEvents(events, num, SDL_GETEVENT, SDL_MOUSEWHEEL, SDL_MOUSEWHEEL);

   for( i = 0; i<count;++i) {
      if(events[i].wheel.direction == SDL_MOUSEWHEEL_NORMAL)
	{ mouse_z=mouse_z-events[i].wheel.y; }
   }
   // Is this needed (to flush the buffer maybe?)
   // SDL_PeepEvents(events, num, SDL_GETEVENT, SDL_MOUSEWHEEL, SDL_MOUSEWHEEL );
#endif
   
//   if(SDL_PollEvent(&event) && event.type==SDL_MOUSEBUTTONDOWN) {
//      if(event.type!=SDL_MOUSEBUTTONDOWN && event.type!=SDL_MOUSEBUTTONUP) SDL_PushEvent(&event);
//      // Mouse Wheel
//      if (event.type==SDL_MOUSEBUTTONDOWN) {
//	 if(event.button.button & SDL_BUTTON_WHEELUP)
//	   mouse_z--;
//	 if(event.button.button & SDL_BUTTON_WHEELDOWN)
//	   mouse_z++;
//      }
////      if (event.type==SDL_MOUSEBUTTONUP)
////	break;
//}
   //     printf("Mouse Button 1(left) is pressed.\n");
}

// Graphics Section

int makecol(int r, int g, int b) {
   // Note: 8-bit would require doing a palette lookup and finding the
   // closest match, something that could be avoided in this day and age
   // 15-bit seems rather obscure, so only 16,24 and 32 bit are currently 
   // implemented
   int rv;

   switch(sa_depth) {
    case 16: rv=makecol16(r,g,b);
      break;
    case 24: rv=makecol24(r,g,b);
      break;
    case 32: rv=makecol32(r,g,b);
      break;
   }
   return(rv);
}

int makecol16(Uint8 r, Uint8 g, Uint8 b) {
   Uint32 pixel;
   pixel=SDL_MapRGB(screen->format,r,g,b);
   return(pixel);
}

int makecol24(Uint8 r, Uint8 g, Uint8 b) {
   Uint32 pixel;
   pixel=SDL_MapRGB(screen->format,r,g,b);
   return(pixel);
}

int makecol32(Uint8 r, Uint8 g, Uint8 b) {
   Uint32 pixel;
   pixel=SDL_MapRGB(screen->format,r,g,b);
   return(pixel);
}

int makeacol16(Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
   Uint32 pixel;
   pixel=SDL_MapRGBA(screen->format,r,g,b,a);
//   printf("returning %d\n",pixel);
   return(pixel);
}

int makeacol24(Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
   Uint32 pixel;
   pixel=SDL_MapRGBA(screen->format,r,g,b,a);
//   printf("returning %d\n",pixel);
   return(pixel);
}

int makeacol32(Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
   Uint32 pixel;
   pixel=SDL_MapRGBA(screen->format,r,g,b,a);
//   printf("returning %d\n",pixel);
   return(pixel);
}

unsigned int makeacol(int r, int g, int b, int a) {
   // Note: 8-bit would require doing a palette lookup and finding the
   // closest match, something that could be avoided in this day and age
   // 15-bit seems rather obscure, so only 16,24 and 32 bit are currently 
   // implemented
   int rv;
   switch(sa_depth) {
    case 16: rv=makeacol16(r,g,b,a);
      break;
    case 24: rv=makeacol24(r,g,b,a);
      break;
    case 32: rv=makeacol32(r,g,b,a);
      break;
   }
//   printf("got makeacol: returning %d\n",rv);
   return(rv);
}

// Alpha blending
int sa_setalpha(SDL_Surface *surf, Uint8 alpha) {
   int rv;
#ifdef SDL1
   rv=SDL_SetAlpha(surf, SDL_SRCALPHA, alpha);
#endif
#ifdef SDL2
   rv=SDL_SetSurfaceAlphaMod(surf, alpha);
#endif
   return rv;
}

#ifdef SDL2
// replacement for SDL_Flip() (which shouldn't be used)
int s2a_flip(SDL_Surface* mysurface) {
   SDL_Rect frect;
   
   frect.x=0;
   frect.y=0;
   frect.w=screen->w;
   frect.h=screen->h;
   
   SDL_RenderCopy(sdlRenderer,mysurface,&frect,&frect);
   SDL_RenderPresent(sdlRenderer);
   
   return(0);
}
#endif 

/* Modes */
int set_gfx_mode(int card, int w, int h, int v_w, int v_h) {
   Uint32 sdlflag,wasinit;
   int retval=0;
   
   switch(card) {
#ifdef SDL2
    case GFX_AUTODETECT_WINDOWED:
      sdlflag=SDL_WINDOW_SHOWN; break;
    case GFX_AUTODETECT:
      sdlflag=SDL_WINDOW_FULLSCREEN_DESKTOP; break;
    case GFX_AUTODETECT_FULLSCREEN:
      sdlflag=SDL_WINDOW_FULLSCREEN_DESKTOP; break;
#else
    case GFX_AUTODETECT_WINDOWED:
      sdlflag=SDL_ANYFORMAT|SDL_DOUBLEBUF; break;
    case GFX_AUTODETECT:
      sdlflag=SDL_ANYFORMAT; break;
    case GFX_AUTODETECT_FULLSCREEN:
      sdlflag=SDL_FULLSCREEN; break;
#endif
    case GFX_TEXT:
      sdlflag=-1; break;
   }

#ifdef SDL2
   // need to define SDL_Window
   if(sdlflag==SDL_WINDOW_FULLSCREEN_DESKTOP) {
      if (!(sdlWindow = SDL_CreateWindow(sa_win_title,SDL_WINDOWPOS_UNDEFINED,
				SDL_WINDOWPOS_UNDEFINED,0,0,sdlflag)))
	retval=1;
   } else {
      if (!(sdlWindow = SDL_CreateWindow(sa_win_title,SDL_WINDOWPOS_UNDEFINED,
				SDL_WINDOWPOS_UNDEFINED,w,h,sdlflag)))
	retval=1;
   }
   
   sdlRenderer = SDL_CreateRenderer(sdlWindow, -1, 0);
   if(sdlRenderer == NULL) {
      /* Handle problem */
      fprintf(stderr, "%s\n", SDL_GetError());
      SDL_Quit();
   }
  
   // make it black
   SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 255);
   SDL_RenderClear(sdlRenderer);
   SDL_RenderPresent(sdlRenderer); // like SDL Flip?
   SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
   SDL_RenderSetLogicalSize(sdlRenderer, w, h);   
//   screen = SDL_CreateRGBSurface(0, w, h, sa_depth, rmask, gmask, bmask, amask);
   screen = create_bitmap(w,h);
#else
   if (sdlflag != -1) {
      if (!(screen = SDL_SetVideoMode(w, h, sa_depth, sdlflag)))
	retval=1;
   } else {
      SDL_QuitSubSystem(SDL_INIT_VIDEO);
      SDL_InitSubSystem(SDL_INIT_VIDEO);	
   }   
#endif

   if(retval!=1) {
      SCREEN_W=w; SCREEN_H=h;
      VIRTUAL_W=w; VIRTUAL_H=h;
//      SDL_SetAlpha(screen,0,255);
   }
   
   return(retval);
}

// delete this once the new one works
#ifdef SDL1
int set_gfx_mode0(int card, int w, int h, int v_w, int v_h) {
   Uint32 sdlflag,wasinit;
   int retval=0;
   
   /* Start here */
   switch(card) {
    case GFX_AUTODETECT_WINDOWED:
      if (!(screen = SDL_SetVideoMode(w, h, sa_depth, SDL_ANYFORMAT|SDL_DOUBLEBUF)))
	retval=1;
      break;
    case GFX_AUTODETECT:
      if (!(screen = SDL_SetVideoMode(w, h, sa_depth, SDL_ANYFORMAT)))
	retval=1;
      break;
    case GFX_AUTODETECT_FULLSCREEN:
      if (!(screen = SDL_SetVideoMode(w, h, sa_depth, SDL_FULLSCREEN)))
//      if (!(screen = SDL_SetVideoMode(w, h, sa_depth, SDL_FULLSCREEN|SDL_HWSURFACE)))
	retval=1;
      break;
    case GFX_TEXT:
//      wasinit=SDL_WasInit(SDL_INIT_EVERYTHING);
//      if(subsystem_init&SDL_INIT_VIDEO) {
	 SDL_QuitSubSystem(SDL_INIT_VIDEO);
	 SDL_InitSubSystem(SDL_INIT_VIDEO);
//      }
      retval=1;
//      if(!SDL_WM_IconifyWindow())
//	retval=1;
      break;
   }
   if(retval!=1) {
      SCREEN_W=w; SCREEN_H=h;
      VIRTUAL_W=w; VIRTUAL_H=h;
//      SDL_SetAlpha(screen,0,255);
   }
   
   return(retval);
}
#endif

// Palette Routines
void get_palette(PALETTE *p) {
//   printf("get_palette stub\n");
}

/* BITMAP routines */

SDL_Surface *create_bitmap(int width, int height) {
   SDL_Surface *new_surface, *xyz;
   #ifdef SDL1
   Uint16 rmask,gmask,bmask,amask;
   
   #if SDL_BYTEORDER == SDL_BIG_ENDIAN
     rmask = 0xf000;
     gmask = 0x0f00;
     bmask = 0x00f0;
     amask = 0x000f;
   #else
     rmask = 0x000f;
     gmask = 0x00f0;
     bmask = 0x0f00;
     amask = 0xf000;
   #endif
   
   xyz = SDL_CreateRGBSurface(SDL_SWSURFACE,width,height,sa_depth,rmask,gmask,bmask,amask);
//   new_surface = SDL_CreateRGBSurface(SDL_SWSURFACE,width,height,sa_depth,rmask,gmask,bmask,amask);
   new_surface = SDL_DisplayFormat(xyz);
//     printf("created\n"); else printf("failed to convert surface\n") ;
   SDL_FreeSurface(xyz);
   SDL_SetAlpha(new_surface,0,255);
   #endif
   
#ifdef SDL2
   Uint32 rmask,gmask,bmask,amask;
   
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
   rmask = 0xff000000;
   gmask = 0x00ff0000;
   bmask = 0x0000ff00;
   amask = 0x000000ff;
#else
   rmask = 0x000000ff;
   gmask = 0x0000ff00;
   bmask = 0x00ff0000;
   amask = 0xff000000;
#endif
   new_surface = SDL_CreateRGBSurface(0,width,height,sa_depth,rmask,gmask,bmask,amask);
   // might need theis for compatability
   //   new_surface=SDL_ConvertSurface(xyz, const SDL_PixelFormat* fmt, 0);
   //   SDL_FreeSurface(xyz);
   SDL_SetSurfaceAlphaMod(new_surface, 255);
   //  SDL_SetAlpha(new_surface,0,255);
#endif
   
//   SDL_SetAlpha(new_surface,SDL_SRCALPHA,SDL_ALPHA_OPAQUE);
   return(new_surface);
}

SDL_Surface *create_system_bitmap(int width, int height) {
   return create_bitmap(width,height);
}

void destroy_bitmap(SDL_Surface *bmp) 
{
   SDL_FreeSurface(bmp);
}


void blit(SDL_Surface *src, SDL_Surface *dest, int srx, int sry, int dsx, int dsy, int wdt, int hgt)  
{
   SDL_Rect srect, drect;
   
//   if(SDL_MUSTLOCK(dest))
//     if(SDL_LockSurface(dest) < 0) return;
   
   srect.x = srx;
   srect.y = sry;
   srect.w = wdt;
   srect.h = hgt;
   drect.x = dsx;
   drect.y = dsy;
   drect.w = wdt;
   drect.h = hgt;

//   printf("1call to blit\n");
   SDL_BlitSurface(src, &srect, dest, &drect);
//   printf("2call to blit\n");
   if(SA_AUTOUPDATE==1) {	
      if(dest == screen)
#ifdef SDL1
	SDL_UpdateRect(dest, dsx, dsy, wdt, hgt);
//      printf("SDL UP: %d %d %d %d \n",dsx,dsy,wdt,hgt);
#endif
#ifdef SDL2
      SDL_RenderCopy(sdlRenderer,screen,&drect,&drect);
      SDL_RenderPresent(sdlRenderer);
#endif
   }
//     SDL_Flip(dest);
}

void masked_blit(SDL_Surface *src, SDL_Surface *dest, int srx, int sry, int dsx, int dsy, int wdt, int hgt)  
{
   // probably can reuse more of this
   #ifdef SDL1  
   SDL_Rect srect, drect;
   Uint32 key;
// Test removal
   key=SDL_MapRGB(src->format,255,0,255);
   SDL_SetColorKey(src,SDL_SRCCOLORKEY,key);

   srect.x = srx;
   srect.y = sry;
   srect.w = wdt;
   srect.h = hgt;
   drect.x = dsx;
   drect.y = dsy;
   drect.w = wdt;
   drect.h = hgt;
   
   SDL_BlitSurface(src, &srect, dest, &drect);
//   printf("call to masked blit\n");
   if(SA_AUTOUPDATE==1) {	
      if(dest == screen) {
#ifdef SDL1
	 SDL_UpdateRect(dest, dsx, dsy, wdt, hgt);
#endif
#ifdef SDL2
	 SDL_RenderCopy(sdlRenderer,screen,&drect,&drect);
	 SDL_RenderPresent(sdlRenderer);
#endif
      }
   }
//     SDL_Flip(dest);
  #endif
}


SDL_Surface *sa_scale_bm(SDL_Surface *orgbm,int dw, int dh) {
   
   // SDL 1.2 doesn't seem to scale bitmaps properly
   //         this could be moved into sdl_allegro
   //         and wrapped by stretch_blit()
   
   SDL_Surface *newbm;
   int nw,nh,px,py,pc,ox,oy;
   Uint8 rrr,ggg,bbb;
   float fow,foh,fdw,fdh,xf,yf,uf;

   // Need to do the math with floats or it won't work
   fow=orgbm->w;foh=orgbm->h;fdw=dw;fdh=dh;
   xf=fow/fdw;
   yf=foh/fdh;

//  if(xf>1 || yf>1) {
    if(xf>yf) 
      uf=xf;
    else
      uf=yf;

   // uf is the factor to scale by
   nw=fow/uf;
   nh=foh/uf;     
   printf("scale to:  %dx%d\n",nw,nh);
   newbm=create_bitmap(nw,nh);

   
   yf=0;
   for(py=0;py<nh;py++) {
     xf=0;
     for(px=0;px<nw;px++) {
       ox=xf;oy=yf;
//       if(xf>nw) ox=nw;
//       if(yf>nh) oy=nh;
       pc=getpixel(orgbm,ox,oy);
       SDL_GetRGB(pc,orgbm->format,&rrr,&ggg,&bbb);
	
	// I don't understand why upscaling (uf<1) reverses
	// the red and blue pixels, but not always!
	if(uf < 1) 
	  putpixel(newbm,px,py,makecol(bbb,ggg,rrr));
	else
	  putpixel(newbm,px,py,makecol(rrr,ggg,bbb));

       xf=xf+uf;
//       printf("XF: %f\n",xf);
     }
     yf=yf+uf;
   }

   return(newbm);
}  // sa_scale_bm

void stretch_blit(SDL_Surface *src, SDL_Surface *dst,int src_x, int src_y, int src_w, int src_h, int dst_x, int dst_y, int dst_w, int dst_h) {

   // SDL1 method

   BITMAP *newbm;   
   SDL_Rect src_r, dst_r;
   
   src_r.x = src_x;
   src_r.y = src_y;
   src_r.w = dst_w;
   src_r.h = dst_h;
   dst_r.x = dst_x;
   dst_r.y = dst_y;
   dst_r.w = dst_w;
   dst_r.h = dst_h;

   newbm=sa_scale_bm(src, dst_w, dst_h);
   SDL_BlitSurface(newbm, &src_r, dst, &dst_r);
   destroy_bitmap(newbm);
// end SDL1 method
   
   if(SA_AUTOUPDATE==1) {	
      if(dst == screen) {
#ifdef SDL1
	 SDL_UpdateRect(dst, dst_x, dst_y, dst_w, dst_h);
#endif
#ifdef SDL2
	 SDL_RenderCopy(sdlRenderer,screen,&dst_r,&dst_r);
	 SDL_RenderPresent(sdlRenderer);
#endif
      }      
   }
}

int show_video_bitmap(BITMAP *bitmap) {
   int i,x,y;
   // emulates routine used for double buffering
   // Since SDL has built-in double buffering, this just calls a flip
   // If allegro-compatible double buffering is needed, this may beed to
   // be rewritten
   
   // It may already be compatible,  the SDL docs aren't clear what happens
   // if you pass anything other than the screen surface to this:
#ifdef SDL1
   i=SDL_Flip(bitmap);   
#endif
#ifdef SDL2
   i=s2a_flip(bitmap);   
#endif
   return(i);
}

// Audio Section
// Currently SDL_mixer is needed,  it may be possible to implement
// with straight SDL, but it would require a little more work.
#ifdef HAVE_LIBSDL_MIXER
int install_sound(int digi, int midi, const char *cfg_path) {
   Mix_OpenAudio(44100, AUDIO_S16SYS, 2, 1024);
}

Mix_Chunk *load_sample(const char *filename) {
   Mix_Chunk *sampl=Mix_LoadWAV(filename);
   if(sampl==NULL) fprintf(stderr, "Couldn't load %s\n",filename);
   return(sampl);
}

SAMPLE *load_wav(const char *filename) {
   SAMPLE *sampl=Mix_LoadWAV(filename);
   if(sampl==NULL) fprintf(stderr, "Couldn't load %s\n",filename);
   return(sampl);
}
int play_sample(const Mix_Chunk *spl, Uint8 vol, Uint8 pan, Uint16 freq, int loop) { 
   int r;
   if(loop>0) loop=-1;
   Mix_VolumeChunk(spl, (vol/2));
   r=Mix_PlayChannel(-1, spl, loop);
   return(r);
}
#endif
// End Audio Section

// GUI section
int file_select_ex(const char *message, char *path, const char *ext, int size, int w, int h ) {
   printf("File selector not (yet) implemented\n");
}

// End GUI Section

// WM section
void set_window_title(const char *name) {
#ifdef SDL2
   strcpy(sa_win_title,name);
   printf("FYI: set_window_title() must be called prior to creating window in sdl2!\n");
#else
   SDL_WM_SetCaption(name, name);
#endif
}
// END WM section

