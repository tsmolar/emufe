#include "config.h"
#define SDL_ALLEGRO_VERS_H "0.7.00"

typedef SDL_Surface BITMAP;

#ifdef HAVE_LIBSDL_MIXER
#include"SDL_mixer.h"
typedef Mix_Chunk SAMPLE;
#endif

/* This was copied from allegro.h */
typedef struct RGB {
   unsigned char r,g,b;
   unsigned char filler;
} RGB;
/* might not be the best way */
//typedef SDL_Palette PALETTE;
typedef RGB PALETTE[256];

BITMAP *screen;
#ifdef SDL2
SDL_Window *sdlWindow;
SDL_Renderer *sdlRenderer;
#endif

/* JOYstick types */
#define JOY_TYPE_AUTODETECT 0

/* GFX modes */
#define GFX_AUTODETECT 0
#define GFX_AUTODETECT_WINDOWED 1
#define GFX_AUTODETECT_FULLSCREEN 2
#define GFX_SAFE 4
#define GFX_TEXT 8
#define GFX_HIDE 16

/* Mouse */
int mouse_x, mouse_y, mouse_z, mouse_b;

/* functions */
void sa_version_info();
void sa_sdl_info();
void sa_getmodes();
void sa_surface_info();
int allegro_init();
void install_keyboard();
void remove_keyboard();
void clear_keybuf();
void set_keyboard_rate(int delay, int repeat);
void simulate_keypress(int key);
int keypressed();
int readkey();
void set_color_depth(int depth);
void install_mouse();
void show_mouse(BITMAP *bmp);
void scare_mouse();
void unscare_mouse();
int mouse_needs_poll();
int poll_mouse();
unsigned int makecol(int r, int g, int b);
unsigned int makecol16(Uint8 r, Uint8 g, Uint8 b);
unsigned int makecol24(Uint8 r, Uint8 g, Uint8 b);
unsigned int makecol32(Uint8 r, Uint8 g, Uint8 b);
unsigned int makeacol16(Uint8 r, Uint8 g, Uint8 b, Uint8 a);
unsigned int makeacol24(Uint8 r, Uint8 g, Uint8 b, Uint8 a);
unsigned int makeacol32(Uint8 r, Uint8 g, Uint8 b, Uint8 a);
unsigned int makeacol(int r, int g, int b, int a);
int set_gfx_mode(int card, int w, int h, int v_w, int v_h);
void get_palette(PALETTE *p);
BITMAP *create_bitmap(int width, int height);
BITMAP *create_system_bitmap(int width, int height);
void destroy_bitmap(BITMAP *bmp);
// int blit(SDL_Surface *src, SDL_Surface *dest, int srx, int sry, int dsx, int dsy, int wdt, int hgt);
void masked_blit(SDL_Surface *src, SDL_Surface *dest, int srx, int sry, int dsx, int dsy, int wdt, int hgt);
void stretch_blit(SDL_Surface *src, SDL_Surface *dst,int src_x, int src_y, int src_w, int src_h, int dst_x, int dst_y, int dst_w, int dst_h);
int show_video_bitmap(BITMAP *bitmap);
BITMAP *load_bitmap(const char *filename, RGB *pal);
int file_select_ex(const char *message, char *path, const char *ext, int size, int w, int h );
void set_window_title(const char *name);

void rest(unsigned int itime);

// custom pcx functions
int save_pcx_pf(FILE *pf, BITMAP *bm, RGB *pal);
int save_pcx(const char *fname, BITMAP *bm, RGB *pal);
BITMAP *sa_readpcx_pf(FILE *pf, RGB *pal);
BITMAP *sa_readpcx(const char *filename, RGB *pal);

#ifdef HAVE_LIBSDL_MIXER
int install_sound(int digi, int midi, const char *cfg_path);
SAMPLE *load_sample(const char *filename);
SAMPLE *load_wav(const char *filename);
int play_sample(const Mix_Chunk *spl, Uint8 vol, Uint8 pan, Uint16 freq, int loop);
#endif

/* Globals */
SDL_Surface *screen;
//int mouse_x, mouse_y, mouse_z,mouse_b;
int SCREEN_W,SCREEN_H,VIRTUAL_W,VIRTUAL_H;
int SA_AUTOUPDATE;

// Joystick
typedef struct JOYSTICK_AXIS_INFO {
   int pos;                 // analogue axis position
   int d1, d2;              // digital axis position
   char *name;              // description of this axis
} JOYSTICK_AXIS_INFO;

typedef struct JOYSTICK_STICK_INFO {
   int flags;       // status flags for this input
   int num_axis;    // how many axes do we have? (note the misspelling)
   JOYSTICK_AXIS_INFO axis[16];   // axis state information
   char *name;                   // description of this input
} JOYSTICK_STICK_INFO;

typedef struct JOYSTICK_BUTTON_INFO {
   int b;                    // boolean on/off flag
   char *name;               // description of this button
} JOYSTICK_BUTTON_INFO;

typedef struct JOYSTICK_INFO {
   SDL_Joystick *handle;
   int flags;                       // status flags for this joystick
   int num_sticks;                // how many stick inputs?
   int num_buttons;               // how many buttons?
   JOYSTICK_STICK_INFO stick[4];  // stick state information
   JOYSTICK_BUTTON_INFO button[16];  //button state information
} JOYSTICK_INFO;

extern int num_joysticks;
extern JOYSTICK_INFO joy[8];

// Keyboard
Uint16 key_shifts;

/* Key Shift Masks */
#define KB_SHIFT_FLAG 0x0001
#define KB_CTRL_FLAG 0x0002
#define KB_ALT_FLAG 0x0004
#define KB_LWIN_FLAG 0x0008
#define KB_RWIN_FLAG 0x0010
#define KB_MENU_FLAG 0x0020
#define KB_COMMAND_FLAG 0x0040
#define KB_SCROLOCK_FLAG 0x0080
#define KB_NUMLOCK_FLAG 0x0100
#define KB_CAPSLOCK_FLAG 0x0200
#define KB_INALTSEQ_FLAG 0x0400
#define KB_ACCENT1_FLAG 0x0800
#define KB_ACCENT2_FLAG 0x1000
#define KB_ACCENT3_FLAG 0x2000
#define KB_ACCENT4_FLAG 0x4000

/* Key Syms */
#define KEY_ENTER_PAD SDLK_KP_ENTER
#define KEY_ENTER SDLK_RETURN
#define KEY_ESC SDLK_ESCAPE
#define KEY_SPACE SDLK_SPACE
#define KEY_BACKSPACE SDLK_BACKSPACE
#define KEY_TAB SDLK_TAB
#ifdef SDL1
#define KEY_LEFT SDLK_LEFT
#define KEY_RIGHT SDLK_RIGHT
#define KEY_UP SDLK_UP
#define KEY_DOWN SDLK_DOWN
#define KEY_PGUP SDLK_PAGEUP
#define KEY_PGDN SDLK_PAGEDOWN
#define KEY_HOME SDLK_HOME
#define KEY_END SDLK_END
#define KEY_F1 SDLK_F1
#define KEY_F2 SDLK_F2
#define KEY_F3 SDLK_F3
#define KEY_F4 SDLK_F4
#define KEY_F5 SDLK_F5
#define KEY_F6 SDLK_F6
#define KEY_F7 SDLK_F7
#define KEY_F8 SDLK_F8
#define KEY_F9 SDLK_F9
#define KEY_F10 SDLK_F10
#define KEY_F11 SDLK_F11
#define KEY_F12 SDLK_F12
#else
// Special handling because these don't work right in SDL2
#define KEY_LEFT 276
#define KEY_RIGHT 275
#define KEY_UP 273
#define KEY_DOWN 274
#define KEY_PGUP 280
#define KEY_PGDN 281
#define KEY_HOME 278
#define KEY_END 279
#define KEY_F1 282
#define KEY_F2 283
#define KEY_F3 284
#define KEY_F4 285
#define KEY_F5 286
#define KEY_F6 287
#define KEY_F7 288
#define KEY_F8 289
#define KEY_F9 290
#define KEY_F10 291
#define KEY_F11 292
#define KEY_F12 293
#endif
// #define KEY_DOWN 274
#define KEY_DEL SDLK_DELETE
#define KEY_0 SDLK_0
#define KEY_1 SDLK_1
#define KEY_2 SDLK_2
#define KEY_3 SDLK_3
#define KEY_4 SDLK_4
#define KEY_5 SDLK_5
#define KEY_6 SDLK_6
#define KEY_7 SDLK_7
#define KEY_8 SDLK_8
#define KEY_9 SDLK_9
#define KEY_A SDLK_a
#define KEY_B SDLK_b
#define KEY_C SDLK_c
#define KEY_D SDLK_d
#define KEY_E SDLK_e
#define KEY_F SDLK_f
#define KEY_G SDLK_g
#define KEY_H SDLK_h
#define KEY_I SDLK_i
#define KEY_J SDLK_j
#define KEY_K SDLK_k
#define KEY_L SDLK_l
#define KEY_M SDLK_m
#define KEY_N SDLK_n
#define KEY_O SDLK_o
#define KEY_P SDLK_p
#define KEY_Q SDLK_q
#define KEY_R SDLK_r
#define KEY_S SDLK_s
#define KEY_T SDLK_t
#define KEY_U SDLK_u
#define KEY_V SDLK_v
#define KEY_W SDLK_w
#define KEY_X SDLK_x
#define KEY_Y SDLK_y
#define KEY_Z SDLK_z

/* Audio Related */
#define DIGI_NONE 0
#define DIGI_AUTODETECT 1
#define DIGI_OSS 2
#define DIGI_ESD 3
#define DIGI_ARTS 4
#define DIGI_ALSA 5
#define DIGI_JACK 6

#define MIDI_NONE 0
#define MIDI_AUTODETECT 1
#define MIDI_OSS 2
#define MIDI_DIGMID 3
#define MIDI_ALSA 4
#define MIDI_WIN32MAPPER 5
