#include <stdio.h>

char types[600];
char *names[53][600], *roms[53][600];

load_menu(char *menu) {
   FILE *fp;
   char type,w;
   char name[53], rom[25];
   int i=1;

   printf("Loading Menu");
   fp=fopen(menu, "r");
   while( type != 'e' ) {
      type=fgetc(fp);
      types[i]=type;
      w=fgetc(fp);
      fgets(name, 53, fp);
      names[0][i]=name;
      roms[0][i]=rom;
      fgets(rom, 25, fp);
/*      printf("%d %s\n",i,roms[0][i]); */
      printf(".");
      i++;
   }
   fclose(fp);
   printf("done\n");
}

main() {
   load_menu("index.menu");
}
