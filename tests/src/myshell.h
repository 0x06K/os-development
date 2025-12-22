#ifndef MYSHELL_H
#define MYSHELL_H

#include <stdint.h>
#include <stddef.h>

/* Max limits */
#define SHELL_MAX_LINE       256
#define SHELL_MAX_ARGS       16
#define SHELL_MAX_COMMANDS   64

/* Command function type */
typedef int (*shell_cmd_func_t)(int argc, char **argv);

/* Command struct */
typedef struct {
    const char *name;
    shell_cmd_func_t func;
} shell_command_t;

/* ------- Core Shell Functions ------- */

/* Initialize shell (register built-ins, prompt, etc.) */
void shell_init(void);

/* Main interactive loop */
void shell_run(void);

/* Read a line from keyboard driver */
int shell_read_line(char *buf, size_t max_len);

/* Split input into argv[] array */
int shell_parse(char *line, char **argv, int max_args);

/* Execute a command */
int shell_execute(int argc, char **argv);

/* ------- Command Registration ------- */

/* Register a command: "name" + handler function */
int shell_register_command(const char *name, shell_cmd_func_t func);

/* Find command by name */
shell_cmd_func_t shell_find_command(const char *name);

/* ------- Built-in Commands ------- */

int shell_cmd_help(int argc, char **argv);
int shell_cmd_clear(int argc, char **argv);
int shell_cmd_echo(int argc, char **argv);

#endif
