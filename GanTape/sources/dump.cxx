#include <stdio.h>

void dump(char* p, int nb)
{
   int i, j, k;
   char* car;
   unsigned short* pShort;
   unsigned char* pChar;
   j = nb / 16;
   if (j * 16 < nb) j++;
   pShort = (unsigned short*)p;
   pChar = (unsigned char*)p;
   for (i = 0; i < j; i++) {
      printf("\n%8x:", pChar);
      for (k = 0; k < 8; k++)
         printf(" %04x", *pShort++);
      printf("    ");
      for (k = 0; k < 16; k++) {
         if ((*pChar >= 32) && (*pChar < 127))
            printf("%c", *pChar);
         else
            printf(".");
         pChar++;
      }
   }
   printf("\n\r");
}
