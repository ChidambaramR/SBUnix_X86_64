#include <stdio.h>
#include <ustdlib.h>

int main(){
  char buf[1000];
  int fd = open("bin/world");
  if( fd == 0 )
      printf("File not found!\n");
  else{
      // Read from the file specified by the FD.
      read(buf,fd);
      printf("fd = %d\n",fd);
      printf("Buffer = %s\n",buf);
      // Close the file.
      close(fd);
  }
      
  return 0;
}
