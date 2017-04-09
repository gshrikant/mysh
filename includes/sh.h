/*
 * sh.h
 */

#define MAXARGS (512)

struct cmd_t {
    char* cmdline;
    char* argv[MAXARGS];
};
