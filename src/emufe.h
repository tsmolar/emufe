#define MODE_RIBBON 0
#define MODE_GRID 1
#define MODE_CLASSIC 2

// used by grid.c
#define DIRCTN_UP 0
#define DIRCTN_RIGHT 1
#define DIRCTN_DOWN 2
#define DIRCTN_LEFT 3
#define RTRN_X 0
#define RTRN_Y 1

#define ACTION_NULL 255
#define ACTION_UP 0
#define ACTION_RIGHT 1
#define ACTION_DOWN 2
#define ACTION_LEFT 3
#define ACTION_SELECT 4
#define ACTION_ESCAPE 5
#define ACTION_CONFIG 10
#define ACTION_HOME 20
#define ACTION_END 21
#define ACTION_PGUP 22
#define ACTION_PGDN 23
#define ACTION_TAB 24
#define ACTION_ALPHANUM 30
#define ACTION_F1 101
#define ACTION_F2 102
#define ACTION_F10 110
#define ACTION_F11 111

char picsdir[90];
char bgpic[90];
extern char picbox[40];
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
extern int cdclock;

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
   int mode;
   int startmode;
   fgbg_t banr;

   // new for 3.0
   int mb_x, mb_y, mb_w, mb_h, mb_x2, mb_y2; // menu box
   int bb_x, bb_y, bb_w, bb_h, bb_x2, bb_y2; // banner box
   int pb_x, pb_y, pb_w, pb_h, pb_x2, pb_y2; // pic box
   int db_x, db_y, db_w, db_h, db_x2, db_y2; // desc box
   int font_w, font_h;  // these should be in the font lib
   int font_sox, font_soy;  // shadow offset for font
   int txdesc_r,txdesc_g,txdesc_b;    // move to menuinfo_t

   // imported for grid.c
   char fullsrc;
   char bgimage[256];  // replace me
   char click[192];
   char basedir[192];  // replace??
   char font[256];    // replace
   int res_x, res_y;
   int numgames, poweroff;
   int numrows;
   int reqjoy, reqkb, reqmouse;
   int g_grid_w, g_grid_h, g_grid_x, g_grid_y; // allow menu to override
   int grid_w, grid_h, grid_x, grid_y, gridres, gridfontsize;
   int g_img_w, g_img_h, g_icon_w, g_icon_h; // allow menu to override
   int img_w, img_h, icon_w, icon_h;
   int g_hdr_x, g_hdr_y, hdr_x, hdr_y;
   int icon_power_x, icon_gear_x, icon_joy_x;
   int topicons_y, topicons_w, topicons_h;
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
   char game[64];
   char rc[70];
   char menu[70]; // what's this for?
   char lastmenu[25];
   char title[44];
   char altromdir[24];
//   char kbname[40];   // contains system_emulator or null for kb images
   int no_launch;
   int autosel;
   int profile;

   // text color settings
   fgbg_t col[4];   //  imenu.col[0].bg.r
} menuinfo_t;

// Note, this might ultimately be used for the menu, but not now
typedef struct menu_t {
   char type;
   char name[42];
   char rom[28];
} menu_t;

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
#define B_JOYST 8
#define B_MCONFIG 9
#define B_MBUTTON 10
#define B_ECONFIG 11
#define B_KBJOY 12
#define B_BOXDESC 13

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

// from grid.c, might be replacable with another structure
typedef struct game_t { 
   char title[128];
   char stitle[128];  // stripped title
   char icon[128];
   char type[6];
   char exec[160];
   int showtext;
} game_t;

//extern conf_t rc;
extern game_t game[200];  // from grid.c
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

// menu stuff required by grid.c
extern char dirname[120];
extern char startdir[160];
extern char passdir[160];
extern menu_t menu[600];
