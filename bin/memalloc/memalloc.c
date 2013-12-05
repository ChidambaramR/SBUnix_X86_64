#include <stdio.h>
#include <ustdlib.h>

int main(int argc, char *argv[]){
  printf("Testing user space malloc\n");
  int* a = (int*)malloc(10);
  int *b = (int*)malloc(20);
  *a = 22;
  *b = 33;
  printf("value a = %d\n",*a);
  printf("value of b = %d\n",*b);
  return 0;
}

