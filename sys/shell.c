#include <sys/shell.h>
#include <defs.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/isr.h>
#define MAXLINE 100

extern uint32_t do_exec(char*);
int parseline(const char *cmdline, struct cmdline_tokens *tok) 
{

    static char array[MAXLINE];          /* holds local copy of command line */
    const char delims[10] = " \t\r\n";   /* argument delimiters (white-space) */
    char *buf = array;                   /* ptr that traverses command line */
    char *endbuf;                        /* ptr to the end of the cmdline string */
    int next_str_ind=0;

    if (cmdline == NULL) {
        printf("Error: command line is NULL\n");
        return -1;
    }

    (void) strncpy(buf, cmdline, MAXLINE);
    endbuf = buf + strlen(buf);

    tok->argc = 0;

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
}

void exec(char* name){
    uint16_t ret;
    ret = do_exec(name);
    if(ret == 0)
      printf("Bad filename or File Not found\n");
}

void eval(char *cmdline) 
{
    struct cmdline_tokens tok;
    uint16_t i,j;
    char cmd[100], tmp[100];
    /* Parse command line */
     parseline(cmdline, &tok);
    for(i=0; i< tok.argc; i++){
      memset(cmd, 0, sizeof(cmd));
      memset(tmp, 0, sizeof(tmp));
//      printf("argv %s\n",tok.argv[i]);
      strncpy(tmp, tok.argv[i], strlen(tok.argv[i]));
//      printf("tmp %s len = %d\n",tmp,strlen(tmp));
      for(j=0; (tmp[j] != ' ' && tmp[j] != '\r' && j < strlen(tmp)); j++)
        cmd[j] = tmp[j];
//      printf("cmd %s\n",cmd);
      if(strcmp(cmd, "cls") == 0)
          cls();
      else if(strcmp(cmd, "h") == 0)
          usage();
      else if(strcmp(cmd, "exec") == 0)
          exec(tok.argv[1]);
      else if(strcmp(cmd, "ps") == 0)
          exec("bin/ps");
    } 

    if (tok.argv[0] == NULL)  return;   /* ignore empty lines */

    return;
}

/*
 * main - The shell's main routine 
 */
void shell_main() 
{
    char cmdline[MAXLINE];    /* cmdline for fgets */
    int emit_prompt = 1; /* emit prompt (default) */
    char prompt[] = "SBUINX> ";

    /* Execute the shell's read/eval loop */
    memset(cmdline, 0, sizeof(cmdline));
    cls();
    printf("Welcome USER. Type h for help\n");
    while (1) {

        if (emit_prompt) {
            printf("%s", prompt);
        }
        doread(cmdline,STDIN);
//        printf("read %s from the terminal\n",cmdline);
        /* Remove the trailing newline */
        cmdline[strlen(cmdline)-1] = '\0';
        
        /* Evaluate the command line */
        eval(cmdline);
        
    } 
    
}
