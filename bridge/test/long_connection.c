#include <stdio.h>
#include <stddef.h>
#include <time.h>
#include <unistd.h>

#define RANDOM(x) (rand()%x)
#define MAC_ADDR_LENGTH 12
#define FMT_MAC_ADDR_LEN (MAC_ADDR_LENGTH+5)

unsigned char HEXCHAR[16]={'0','1','2','3','4','5','6','7','8','9','A','B','C', 'D','E','F'};
unsigned char genMACAddr[MAC_ADDR_LENGTH];
unsigned char fmtMACAddr[FMT_MAC_ADDR_LEN]={'0','0','-','0','0','-','0','0','-','0','0','-','0','0','-','0','0'};

void formatMACAddr()
{
  unsigned short i=0;
  unsigned short n=0;
  for(i=0;i<MAC_ADDR_LENGTH;i++,n++)
  {
    if(fmtMACAddr[n]!='-')
    {
      fmtMACAddr[n]=genMACAddr[i];
    }
    else
    {
      n++;
      fmtMACAddr[n]=genMACAddr[i];
    }
  }
}


size_t RSHash(const char *str)  
{  
  size_t ch;
  register size_t hash = 0;  
  size_t magic = 63689;     
  while ( ch = (size_t)*str++)  
  {  
      hash = hash * magic + ch;  
      magic *= 378551;  
  }  
  return hash;  
}

void generateRandomMAC()
{
  unsigned short i=0;
  unsigned short n=0;
  
  for(int i=0;i<MAC_ADDR_LENGTH;i++)
  {
    n=RANDOM(16);
    genMACAddr[i]=HEXCHAR[n];
  }
  formatMACAddr();
}

void main()
{
  srand(time(0));
  for (int i = 0; i < 10; i++)
  {
    generateRandomMAC();
    printf("%s ",fmtMACAddr);
    printf("%u \n",RSHash(fmtMACAddr));
    // sleep(1);
  }
}


