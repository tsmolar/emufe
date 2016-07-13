#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#ifdef HAVE_LIBSDL_IMAGE
#include <SDL.h>
#include <SDL_image.h>
#include "sdl_allegro.h"
//
// Image Load
// Apparently SDL_image cannot save
SDL_Surface *load_bitmap(const char *filename, RGB *pal) {
   
   // Covert to screen format, should be faster
   // (seems to add overhead and offer no benefit)
//   SDL_Surface *newsr = IMG_Load( filename );
//   SDL_Surface *image = SDL_DisplayFormat(newsr);
//   SDL_FreeSurface(newsr);
   // if you don't want to convert the image to the video format
    SDL_Surface *image = IMG_Load( filename );
//   if(!image) 
//     printf("ERROROR : coult not load %s\n",filename);
   return(image);
}

SDL_Surface *load_pcx(const char *filename, RGB *pal) {
   SDL_Surface *image;
   SDL_RWops *rwop;
   rwop=SDL_RWFromFile(filename,"rb");
   image= IMG_LoadPCX_RW( rwop );
   return(image);
}
SDL_Surface *load_bmp(const char *filename, RGB *pal) {
   SDL_Surface *image;
   SDL_RWops *rwop;
   rwop=SDL_RWFromFile(filename,"rb");
   image= IMG_LoadBMP_RW( rwop );
   return(image);
}
SDL_Surface *load_lbm(const char *filename, RGB *pal) {
   SDL_Surface *image;
   SDL_RWops *rwop;
   rwop=SDL_RWFromFile(filename,"rb");
   image= IMG_LoadLBM_RW( rwop );
   return(image);
}
SDL_Surface *load_tga(const char *filename, RGB *pal) {
   SDL_Surface *image;
   SDL_RWops *rwop;
   rwop=SDL_RWFromFile(filename,"rb");
   image= IMG_LoadTGA_RW( rwop );
   return(image);
}
#endif
