#include<stdio.h>

int nument, enddec;
nument=3;
entdec=3;

int readkee() 
{
   char key;
   if(nument>1) 
     {
	nument=nument-entdec;
	key=85;
	entdec++;
	if(entdec>9) entdec=3;
     } else 
     { 
	if(nument < 0) 
	  {
	     	key=85;
	  } else 
	  {
	     
	key=67;
	  }
	
     }
   nument++;
   return key;
}

main() 
{
   int i,k;
   for(i=0;i<120;i++) 
     {
	k=readkee();
	printf("%d ",k);
     }
   printf("\ndec %d\n",entdec);
}
