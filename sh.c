/*
 * sh.c
 */

#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <assert.h>
#include <sys/wait.h>
#include <dirent.h>

#include <linenoise.h>
#include <sh.h>

const char *DELIM = "\n\t ";

struct cmd_t cmd;

void die(char *estr)
{
	perror(estr);
	exit(EXIT_FAILURE);
}

char *sh_read(void)
{
	char *line = NULL;
	if ((line = linenoise("> ")) != NULL)
		linenoiseHistoryAdd(line);

        return line;
}

/* Split command line */
struct cmd_t *split(char *str)
{
#ifdef DEBUG
        assert(str != NULL);
#endif
	char *r;
        struct cmd_t *cmd;
	unsigned int i = 0;

	if ((r = strtok(str, DELIM)) == NULL)
                return NULL;

        if ((cmd = malloc(sizeof(struct cmd_t))) == NULL)
                return NULL;

	cmd->cmdline = str;
	cmd->argv[i++] = r;

	/* Parse remaining args */
	while (r && i < MAXARGS) {
                /* Handle errors here */
		r = strtok(NULL, DELIM);
		cmd->argv[i++] = r;
	}

        return cmd;
}

void sh_chdir(struct cmd_t *cmd)
{
	int r;
	char *dest;

        /* Probably overwrites stuff on stack! */
	dest = cmd->argv[1] ? cmd->argv[1] : getenv("HOME");

	/* when $HOME isn't set */
	dest = dest ? dest : "/";
	r = chdir(dest);
	if (r < 0)
		perror(dest);
}

void sh_execute(struct cmd_t *cmd)
{
	pid_t cpid;

#ifdef DEBUG
        assert(cmd->argv[0] != NULL);
#endif

	if (!strcmp(cmd->argv[0], "cd")) {
		sh_chdir(cmd);
		return;
	}

	cpid = fork();

	if (cpid < 0)
		die("fork");
	
        if (cpid > 0) {
		waitpid(-1, NULL, 0);
	} else {
		execvp(cmd->argv[0], cmd->argv);
		die(cmd->argv[0]);
	}
}

void
completion(const char *buf, linenoiseCompletions *lc)
{
        DIR *dir;
        struct dirent *ent;

        dir = opendir("/bin");
        if (!dir)
                return;

        while ((ent = readdir(dir)) != NULL) {
                if ((strstr(ent->d_name, buf) != 0) && (ent->d_type == DT_REG))
                        linenoiseAddCompletion(lc, ent->d_name);
        }

        closedir(dir);
}

int main(void)
{
	char *raw_line = NULL;
        struct cmd_t *cmd = NULL;

        linenoiseSetCompletionCallback(completion);

	while (1) {
		raw_line = sh_read();
		if (!raw_line)
                        goto clean_line;

                cmd = split(raw_line);

		if (!cmd)
                        goto clean_cmd;

		sh_execute(cmd);

                free(raw_line);
                free(cmd);
	}

	return EXIT_SUCCESS;

clean_cmd:
        free(cmd);

clean_line:
	free(raw_line);
        return EXIT_FAILURE;
}
