#include <defs.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <io.h>

char* video_memory = (char *)START_MEMORY;
char* timer_memory = (char *)TIMER_MEMORY;
char* keyboard_memory = (char*)KEYBOARD_WARMUP_MEMORY;
extern char* io_buff;
extern int reading;
/*
FUnction to convert a number to any base and it returns as a character pointer,
*/
char *convert(uint64_t num, int base)
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

void write_time(uint16_t su, uint16_t st, uint16_t mu, uint16_t mt, uint16_t hu, uint16_t ht)
{
       volatile char *video = (volatile char*)timer_memory;
       uint8_t color = TEXT_COLOR;
       char *su_p, *st_p, *mu_p, *mt_p, *hu_p, *ht_p;
       ht_p = convert(ht,10);
       char col = ':';
       *video++ = *ht_p;
       *video++ = color;
       hu_p = convert(hu,10);
       *video++ = *hu_p;
       *video++ = color;
       *video++ = col;
       *video++ = color;
       mt_p = convert(mt,10);
       *video++ = *mt_p;
       *video++ = color;
       mu_p = convert(mu,10);
       *video++ = *mu_p;
       *video++ = color;
       *video++ = col;
       *video++ = color;
       st_p = convert(st,10);
       *video++ = *st_p;
       *video++ = color;
       su_p = convert(su,10);
       *video++ = *su_p;
       *video++ = color;
//       kprintf("Timer handled\n");
}

void *memset(void *s, uint8_t c, uint32_t n)
{
    unsigned char* p= (unsigned char*)s;
    while(n--)
        *p++ = (unsigned char)c;
    return s;
}

void write_key(int color, char c){
    volatile char* video = (volatile char*)keyboard_memory;
    *video++ = c;
    *video++ = color;
}

/*
Trivial function. Writes a byte ( character ) to the current pointer of the video
buffer and updates the video memory.
*/
void write_char(int color, char c)
{
       volatile char *video = (volatile char*)video_memory;
       volatile uint64_t diff;
       volatile uint64_t col1;
       if(c == '\n' || (int)c == 13){
              /*
               * \n is a special case. We need to push the video_memory by one row. Since
               * each character occupies two bytes, to move just one row down, we need to
               * add to the video_memory a value of 160 ( 80*2 ) = 0xA0.
               * */
               video = (video + 0xA0);
               diff = (unsigned long)(video - START_MEMORY);
               col1 = (diff/0xA0);
               video = (volatile char*)(START_MEMORY + 0xA0*col1);
       }
       else{
               *video++ = c;
               *video++ = color;
       }
       video_memory = (char *)(video);
       update_cursor();
}


void write_io_buff(char c)
{
//       printf("received %d\n",c);
       if(c == '\n' || (int)c == 13){
            *io_buff++ = c;
            //printf("setting reading = 0\n");
            reading = 0;
       }
       else{
            *io_buff++ = c;
       }
}

void strncpy(char* dest, const char* src, uint32_t size){
    uint32_t i;  
    for(i=0; i<size; i++)
        dest[i] = src[i];
}
/*
Writes to the video memory buffer. The text screen video memory for color monitors
resides at 0xB8000. Text mode memory takes two bytes for each character on the
screen. One is the ASCII code byte, other is the attribute byte. The attribute
byte carries the foreground color in the lower 4 bits, and the background color
in the higher 3 bits. 0x00 is black on black and nothing can be seen. 0x07 is
lightgrey on black. 0x1F is white on blue.
I guess as and when we write to this buffer of the video memory, it
consumes it and puts it out on the screen.
*/

void write_string( int color, const char *string )
{
    volatile char *video = (volatile char*)video_memory;
    volatile uint64_t diff;
    volatile uint64_t col1;
    //int length = strlen(string);
    while( *string != NULL )
    {
       if(*string == '\n'){
              /*
               * \n is a special case. We need to push the video_memory by one row. Since
               * each character occupies two bytes, to move just one row down, we need to
               * add to the video_memory a value of 160 ( 80*2 ) = 0xA0.
               * */
               video = (video + 0xA0);
               diff = (unsigned long)(video - START_MEMORY);
               col1 = (diff/0xA0);
               video = (volatile char*)(START_MEMORY + 0xA0*col1);
               string++;
        }
       else{
        *video++ = *string++;
        *video++ = color;
       }
//     (char *)video_memory++;
    }
    video_memory = (char *)(video);
    update_cursor();
}

/*
Prints the given values to the screen. Since it is 64 bit machine, the way va_arg
works is different. All the variables passed through the registers are pushed into
register-saved-area of the stack. Then the general purpose registers and the floating
point registers follow. Thus the va_arg function accesses this area smoothly.
*/
int printf(const char *fmt,...){
       const char *p;
    //   void *v;
       uint64_t addr;
       int i; // integer argument
       //unsigned u; // unsigned int argument
       char *s; // string argument
       va_list arg_p; // pointer to the variable argument list
       va_start(arg_p, fmt); /* Initializes the pointer to retrieve the additional
                               parameters. Should call va_end before end. fmt is
                               also passed because, we need to know where the last
                               fixed argument is in order to find the starting of
                               the variable list.
                            */
       for(p=fmt; *p ; p++){
               if(*p != '%'){
                       write_char(0x1F,*p);
                       continue;
               }
               switch(*++p){
                       case 'c': i = va_arg(arg_p, int);
                                 write_char(0x1F,i);
                                 break;
                       case 's': s = va_arg(arg_p, char*);
                                 write_string(0x1F,s);
                                 break;
                       case 'd': i = va_arg(arg_p, int);
                                 write_string(0x1F,convert(i,10));
                                 break;
                      case 'x':  i = va_arg(arg_p, int);
                                 write_string(0x1F,"0x");
                                 write_string(0x1F,convert(i,16));
                                 break;
                      case 'p':  addr = va_arg(arg_p, uint64_t); // Print Address of the pointer
                                 //addr = (uint64_t) v;
    //                             write_string(0x1F,"0x");
                                 write_string(0x1F,convert(addr,16));
                                 break;

               }
       }
       va_end(arg_p);
       return 1;
}

/*
Function to clear the screen. Starts from the start of the memory (0xB8000) and
fills every ASCII byte to 0x20 which is space and assigns the attribute byte to
0x0 which is black.
*/
void cls(){
       int i;
       volatile char* video = (volatile char*)START_MEMORY;
       for(i=0; i<(MAX_ROWS * MAX_COLUMNS); i++){
               *video++ = 0x20;
               *video++ = 0x0;
       }
       video_memory = (char *)START_MEMORY;
       update_cursor();
}

/*
Updates the position of the cursor. Memory is linear. So if we want to go to
5*6 ( row*column ), multiplying 5 by 80 will give 400. Plus 6 will give 6 bytes.
Thus 406th byte is the actual position of 5*6.
0x3d0 - 0x3DF is the port address mapping range for VGA / CGA memory. When any
port in this port range is specified, the controller picks up the data and
gives it to the controller of the device so that the device does as we asked.
*/
void update_cursor()
{
    volatile uint64_t diff = (long)(video_memory - START_MEMORY);
    volatile char* video = (volatile char*)START_MEMORY;
    char* tmp_video_memory;
    /* Long is used because the size a pointer in 64 bit machine is 8 bytes.
       So we cannot type cast it to an int which is of lesser size. In C
       we can always typecast from lower size to upper size but not from
       bigger size to lower size.
    */
    uint16_t i;
    int temp_row;
    uint64_t pos; // Long because, we cant typecast temp_row*80 + START_MEMORY to char*.
             // If we do so, we would typecast from int to char* which is 4 bytes
             // to 8 bytes. In C small to big typecast is not allowed.
    uint32_t row1 = (diff/80)/2;
    uint32_t col1 = ((diff - row1*80*2)/2)%80;

    /*
    If the output scrolls over the screen, then we have to update it, so that
    the number of rows which comes after the 25th row, pushes the equal amount
    of rows from the top back. It can be thought similar to linux scrolling. Memcpy
    starts to copy leaving x number of rows from the start, where x is the number of
    rows which scrolled past
    */
    if(row1 >= 24){
       temp_row = row1 - 24 + 1; // Getting the no of rows to be scrolled
       pos = temp_row*80*2 + START_MEMORY;
       memcpy((char *)video, (const char*)(pos), (24-temp_row)*80*2);
       video_memory = (char *)(23*80*2 + START_MEMORY);
       tmp_video_memory = video_memory;
       row1 = 23;
       col1=0;
       for(i=0; i<80; i++){
          *tmp_video_memory++ = 0x20;
          *tmp_video_memory++ = 0x0; 
       }
    }

    unsigned short position=(row1*80) + col1;
    // cursor LOW port to vga INDEX register
    outb(0x3D4, 0x0F);
    outb(0x3D5, (unsigned char)(position&0xFF)); // Send the lower byte
    // cursor HIGH port to vga INDEX register
    outb(0x3D4, 0x0E);
    outb(0x3D5, (unsigned char )((position>>8)&0xFF)); // Send the upper byte.

    /*
       This should not be confused as sending x position first and sending y
       position second. If we need to go to 80th row and 4th column, then
       we 80*80 + 4 = 6404. In binary it is 1100100000100. This first
       outb statementwill yield 00000100 and second outb statement will yield
       00011001.
    */

}

char *memcpy(char *dest, const char *src, uint32_t n)
{
    char *dp = dest;
    const char *sp = src;
    while (n--){
/*        if(*sp == NULL){
          dp++;
          sp++;
          continue;
        }*/
        *dp++ = *sp++;
    }
    return dest;
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


void PANIC(const char *s, uint16_t line, const char* str){
    printf("Kernel PANIC at %s:%d. REASON: %s",s,line,str);
    printf("\n...Backtrace of PANIC..\n");
    printf("\t.....0x%p\n",__builtin_return_address(0));
    while(1);
//    printf("\t.....0x%p\n",__builtin_return_address(1));
//    printf("\t.....0x%p\n",__builtin_return_address(2));
}

void SYS_TRACE(const char *s, uint16_t line, const char* str){
    printf("WARNING!! at %s:%d. REASON: %s",s,line,str);
}

uint32_t my_atoi(char *str){
    uint32_t res=0;
    uint16_t i;
    for(i=0; str[i] != '\0'; i++){
//      printf("%c ",str[i]);
      res = res*10 + (str[i] - '0');
    }
    return res;
}


uint32_t my_atool(char *str){
    uint32_t res=0;
    uint16_t i;
    for(i=0; str[i] != '\0'; i++){
//      printf("%c ",str[i]);
      res = res*8 + (str[i] - '0');
    }
    return res;
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
