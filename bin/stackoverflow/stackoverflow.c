#include <stdio.h>

void sum(int a, int b){
  __attribute__((unused))int a2[100];
  sum(a,b);
}

int main(){
  sum(5,6);
  return 0;
}
