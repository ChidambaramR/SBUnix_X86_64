#include <stdio.h>
#include <ustdlib.h>

int main(){
  printf("Process sleeping for 10 seconds\n");
  sleep(10);
  printf("Process woken up after 10 seconds\n");
  return 0;
}
