#ifndef _SHELL_H_
#define _SHELL_H

#define MAXARGS 10

struct cmdline_tokens {
    int argc;               /* Number of arguments */
    char *argv[MAXARGS];    /* The arguments list */
};

#endif
