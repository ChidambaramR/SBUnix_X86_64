#include<defs.h>
#include<ustdlib.h>
char *convert_u(uint64_t num, uint16_t base)
{
static char buf[65];
char *ptr;

ptr=&buf[sizeof(buf)-1];
*ptr='\0';
do
{
*--ptr="0123456789abcdef"[num%base];
num/=base;
}while(num!=0);
return(ptr);
}

