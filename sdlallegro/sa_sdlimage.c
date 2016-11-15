#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#if defined(HAVE_LIBSDL_IMAGE) || defined(HAVE_LIBSDL2_IMAGE)
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

int save_bmp(const char *filename, SDL_Surface *bmp, const RGB *pal) {
   int r;
   r=SDL_SaveBMP(bmp, filename);
   return r;
}

// save_pcx moved into sa_pcx,  that procedure does NOT require SDL_Image

int save_bitmap(const char *filename, BITMAP *bmp, const RGB *pal) {
   char type[6];
   int i,ci=0, r=0;
   
   for(i=strlen(filename)-3;i<=strlen(filename);i++) {
      type[ci]=filename[i];
      ci++;
   }
   type[ci]=0;
   printf("SDLa: bitmap type is %s\n",type);
   if(strcmp(type,"pcx")==0) r=save_pcx(filename,bmp,pal);
   if(strcmp(type,"PCX")==0) r=save_pcx(filename,bmp,pal);
   if(strcmp(type,"bmp")==0) r=save_bmp(filename,bmp,pal);
   if(strcmp(type,"BMP")==0) r=save_bmp(filename,bmp,pal);
   return r;
}
#endif
