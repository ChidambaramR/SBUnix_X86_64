#include <sys/shell.h>
#include <defs.h>
#include <stdio.h>
#include <ustdlib.h>

volatile char a[] = "abcsdef\n";
char prompt[] = "SBUINX> ";

int parseline(const char *cmdline, struct cmdline_tokens *tok) 
{

    char array[100];          /* holds local copy of command line */
    const char delims[10] = " \t\r\n";   /* argument delimiters (white-space) */
    char *buf = array;                   /* ptr that traverses command line */
    char *endbuf;                        /* ptr to the end of the cmdline string */
    int next_str_ind=0;

    if (cmdline == NULL) {
        printf("Error: command line is NULL\n");
        return -1;
    }

    (void) strncpy(buf, cmdline, 100);
    endbuf = buf + strlen(buf);

    tok->argc = 0;
//    printf("buf = %s\n",buf);
//    printf("cmdline = %s\n",cmdline);
    while (buf < endbuf) {
        if (buf >= endbuf) break;
        next_str_ind = strcpsn(buf, delims);
        tok->argv[tok->argc++] = buf;
            /* Detect quoted tokens */

        if (!strcmp(tok->argv[0], "quit")) {                 /* quit command */
            tok->builtins = BUILTIN_QUIT;
        } else if (!strcmp(tok->argv[0], "jobs")) {          /* jobs command */
            tok->builtins = BUILTIN_JOBS;
        } else {
            tok->builtins = BUILTIN_NONE;
        }
        buf = buf + 1 + next_str_ind;
    }
    return 0;
}

void usage(){
    printf("1. cls - Clear Screen\n");
    printf("2. exec filename - Execute a process\n");
    printf("3. ps - print the current running process\n");
    printf("4. ls - List the contents of current working directory\n");
}

int exec(char* name){
    int ret;
//    printf("in exec\n");
    ret = execve(name);
    return ret;
}


int eval(char *cmdline) 
{
    struct cmdline_tokens tok;
    int j,ret=0;
    char cmd[100], tmp[100];
    /* Parse command line */
     parseline(cmdline, &tok);
//     printf("argc = %d\n",tok.argc);
//    for(i=0; i< tok.argc; i++){
      memset(cmd, 0, sizeof(cmd));
      memset(tmp, 0, sizeof(tmp));
//      printf("argv %s\n",tok.argv[i]);
      strncpy(tmp, tok.argv[0], strlen(tok.argv[0]));
//      printf("tmp %s len = %d\n",tmp,strlen(tmp));
      for(j=0; (tmp[j] != ' ' && tmp[j] != '\r' && j < strlen(tmp)); j++)
        cmd[j] = tmp[j];
      if(strcmp(cmd, "cls") == 0)
          cls();
      else if(strcmp(cmd, "pwd") == 0)
          print_pwd();
      else if(strcmp(cmd, "h") == 0)
          usage();
      else if(strcmp(cmd, "exec") == 0){
//          printf("calling exec\n");
          ret = exec(tok.argv[1]);
     //     break;
      }
      else if(strcmp(cmd, "cd") == 0){
//          printf("calling exec\n");
          ret = cd(tok.argv[1]);
     //     break;
      }
      else if(strcmp(cmd, "ps") == 0)
          exec("bin/ps");
      else if(strcmp(cmd, "ll") == 0)
          exec("bin/ll");
      else if(strcmp(cmd, "ls") == 0)
          exec("bin/ls");
      else
          printf("Enter a valid command\n");
//      }

    if (tok.argv[0] == NULL)  return -1;   /* ignore empty lines */

    return ret;
}

/*
 * main - The shell's main routine 
 */
int main(int argc, char* argv[], char* envp[]) 
{
    char cmdline[100];    /* cmdline for fgets */
    int emit_prompt = 1; /* emit prompt (default) */
    int pid, ret;
    /* Execute the shell's read/eval loop */
    //cls();
    printf("Welcome USER. Type h for help\n");
//    printf("once once\n");
    while (1) {
        memset(cmdline, '\0', sizeof(cmdline));
        if (emit_prompt) {
            printf("%s", prompt);
        }
        read(cmdline, STDIN);
//        printf("read %s from the terminal\n",cmdline);
        /* Remove the trailing newline */
        cmdline[strlen(cmdline)-1] = '\0';
        
      // Forking a new Child to execute the command 
       pid = fork();
        if(pid > 0){
          // In the child which is the actual worker
          ret = eval(cmdline);
          if(ret == -1)
            printf("Bad File name\n");
          //while(1);
            exit(1);
          }
        else{
          wait();
          //printf("\nProcess %s exited normally pid = %d\n\n",cmdline,getpid());
        }
    } 
   
  return 0; 
}
