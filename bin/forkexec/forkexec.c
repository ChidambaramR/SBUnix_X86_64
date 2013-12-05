#include <stdio.h>
#include <ustdlib.h>

int main(){
  int pid;
  pid = fork();
  if(pid > 0){
  printf("Hello. In Child\n");
  exit(1);
  }
  else
    printf("In parent\n");
  return 0;
}
