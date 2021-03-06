char picsdir[90];
char bgpic[90];
char picbox[40];
char menuname[20];
extern char defimg[20];
extern char descdir[90];
extern char gthemedir[96];
extern char theme[200];
extern char tfontbmp[30];
extern char basedir[160];
extern int rx0, ry0, usex, usey;
extern char fullscr;
extern int joy_enable;

// debug level can be 0-5
#define DEBUG_LEVEL 5

// debug print macro
#ifdef DEBUG
# define LOG(level, x) do { if ( level <= DEBUG_LEVEL ) { printf("log(%d): ",level); printf x;} } while(0)
#else
# define LOG(level, x)
#endif

typedef struct crgba_t {
   int r,g,b,a;
   char enable;
} crgba_t;

typedef struct fgbg_t {
   crgba_t fg,bg,sh;
} fgbg_t;

typedef struct prop_t {
   char resolution[16];
   char ttfont[30];
   char fontsize[10];
   fgbg_t banr;

  // new for 3.0
  int mb_x, mb_y, mb_w, mb_h, mb_x2, mb_y2; // menu box
  int bb_x, bb_y, bb_w, bb_h, bb_x2, bb_y2; // banner box
  int pb_x, pb_y, pb_w, pb_h, pb_x2, pb_y2; // pic box
  int db_x, db_y, db_w, db_h, db_x2, db_y2; // desc box
  int font_w, font_h;  // these should be in the font lib
  int txdesc_r,txdesc_g,txdesc_b;    // move to menuinfo_t
} prop_t;

#define SYS_GENERIC 0
#define SYS_ARCADE 1
#define SYS_COMPUTER 2

typedef struct menuinfo_t {
   int mode;
   int level;
   int noexec;
   char system[20];
   char sysbase[40];
   int systype;
   char emulator[40];
   char game[40];
   char rc[70];
   char menu[70]; // what's this for?
   char lastmenu[25];
   char title[44];
//   char kbname[40];   // contains system_emulator or null for kb images
   int no_launch;
   int autosel;
   int profile;

   // text color settings
   fgbg_t col[4];   //  imenu.col[0].bg.r
} menuinfo_t;

typedef struct env_t {
   char var[20];
   char value[80];
} env_t;

#define B_BOXSCAN 0
#define B_MEDIA 1
#define B_KEYBOARD 2
#define B_PICBOX 3
#define B_SSHOT1 4
#define B_SSHOT2 5
#define B_SSHOT3 6
#define B_SSHOT4 7

typedef struct imgbox_t {
   int enabled;
   int init;
   char name[20];
   int x,y,w,h,x2,y2;
   char pfx[6];
   int r,g,b;
   int mgn;
   char imgname[90];
   int masktype;   // for masking empty areas
   int ovpct;   // overlay pct  0=overlay off
   char ovname[90];  // name of overlay
   char mask[90];   // name of mask bitmap (for rounded TV screens)
} imgbox_t;

#define B_BANR 0
#define B_MENU 1
#define B_DESC 2
#define B_SETUP 3

typedef struct txtbox_t {
   int enabled;
   char name[20];
   char box[40];  // bg filename or trans
   int x,y,w,h,x2,y2;
   char font[40];
   int fonttype;
   int font_w;
   int font_h;   // height of font
   int font_v;   // font vertical space, allows more space between lines
} txtbox_t;

extern imgbox_t imgbx[12];
extern txtbox_t txtbx[4];
extern menuinfo_t imenu;
extern prop_t rc;
extern char cdroot[220];

void draw_desc(int fg, int bg);
void draw_imgbx(int boxno);
void draw_menubox(int fg, int bg);
void draw_title(int fg, int bg);
void set_bg();
int title(int x1,int y1,char *s);
