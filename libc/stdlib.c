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

void memset(void *s, uint8_t c, uint32_t n)
{
    unsigned char* p= (unsigned char*)s;
    while(n--)
        *p++ = (unsigned char)c;
}


void strncpy(char* dest, const char* src, uint32_t size){
    uint32_t i;
    for(i=0; i<size; i++)
        dest[i] = src[i];
}
/*
Function to find the length of the string.
*/
int strlen(const char *s){
       int length=0;
       char* local_s = (char *)s;
       while(*local_s){
               length++;
               local_s++;
       }
       return length;
}

int strcmp(char *a, char* b){
    int i=0, equals = 0; // equals = 0 is the tru value
    int len1 = strlen(a);
    int len2 = strlen(b);
    if( len1 != len2 )
        return 1;
    while(a[i] != NULL){
        if(a[i] == b[i]){
          i++;
          continue;
        }
        else{
          equals = 1;
          break;
        }
    }
    return equals;
}

char *strchr(const char *s, int c)
{
    while (*s != (char)c)
        if (!*s++)
            return 0;
    return (char *)s;
}

int strcpsn(const char *s1, const char *s2)
{
    int ret=0;
    while(*s1)
        if(strchr(s2,*s1))
            return ret;
        else
            s1++,ret++;
    return ret;
}
