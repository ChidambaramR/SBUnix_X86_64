#include <stdio.h>
#include <ustdlib.h>
int b=199;
void sum(int a, int b){
  __attribute__((__unused__))int a1[100];
  sum(5,6);
}

int main(int argc, char* argv[]) {
       // while(1)
      int a;
      a = fork();
      if(a == 0){
      int no;
//      printf("b = %d",b); 
//          b = 5;   
      char buf[30];
      printf("Welcome to Parent. Enter some data\n");
      while((no = read(buf,STDIN)) == -1);
      printf("parent got data %s\n", buf);
//      sum(5,6); 
      printf("waiting for childs to exit\n");
      wait();
      printf("parent exiting since its children are no more\n");
  //        printf("parent looping for data %d\n",no);
//    while(1)
//      printf("parent receieved data no = %d\n", no);
//          printf("address of b = %p val of b = %d",&b,b);
      }
      else{
      //int no;
      //char buf[30];
      printf("Welcome to child. Child is sleeping for 10 seconds before exec'ng world\n");
      sleep(10); 
      printf("10 seconds over. Child is exec'ng\n");
/*      if(execve("bin/world") == -1)
          printf("Bad arguments for execve\n");
      else
          printf("exec success\n"); */
      //while( (no = read(buf)) == -1);
//        printf("child trying again\n");
      //printf("child received data %s\n",buf);
      //printf("heelo %d\n",a);
      //printf("world %d",a);
      }
//      while(1){
//  printf("ppid = %d chid \n", getpid());
//        printf("user\n");
//      }
//        while(1)
  //        printf("hi user\n");
	return 0;
}
