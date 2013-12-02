//#include <syscalls.h>
#include <ustdlib.h>
#include <stdarg.h>
#include <defs.h>

uint16_t write(const char* buf){
  __asm__("movq $0x2, %rax;\n\tint $0x80;"); 
  return 0;
}

int printf(const char *fmt, ...) {
       const char *p;
       char buf[100];
       uint16_t ind,cnt=0;
    //   void *v;
//       uint64_t addr;
       int i; // integer argument
       //unsigned u; // unsigned int argument
       char *s; // string argument
       va_list arg_p; // pointer to the variable argument list
//       buf = "Chidhu";
//       write((const char*)buf);
       va_start(arg_p, fmt); /* Initializes the pointer to retrieve the additional
                               parameters. Should call va_end before end. fmt is
                               also passed because, we need to know where the last
                               fixed argument is in order to find the starting of
                               the variable list.
                            */
       for(p=fmt; *p ; p++){
               char* conv;
               if(*p != '%'){
                        buf[cnt++] = *p;
                       continue;
               }
               switch(*++p){
                       case 'c': i = va_arg(arg_p, int);
                                 buf[cnt++] = i;
                                 break;
                       case 's': s = va_arg(arg_p, char*);
                                 ind = 0;
                                 while(s[ind])
                                      buf[cnt++] = s[ind++];
                                 break;
                       case 'd': i = va_arg(arg_p, int);
                                 conv = convert_u(i,10);
                                 for(ind=0; ind<8; ind++)
                                    if(conv[ind] != ' ')
                                        buf[cnt++] = conv[ind];
                                 break;
                      case 'x':  i = va_arg(arg_p, int);
                                 buf[cnt++] = '0';
                                 buf[cnt++] = 'x';
                                 conv = convert_u(i,10);
                                 for(ind=0; ind<8; ind++)
                                    if(conv[ind] != ' ')
                                        buf[cnt++] = conv[ind];
                                 break;
/*                      *case 'p':  addr = va_arg(arg_p, uint64_t); // Print Address of the pointer
                                 //addr = (uint64_t) v;
    //                             write_string(0x1F,"0x");
                                 break;*/

               }
       }
       va_end(arg_p);
       write((const char*)buf);
       memset(buf, '\0', sizeof(buf));
	return 0;
}
