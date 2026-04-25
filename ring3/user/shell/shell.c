// shell.c — userspace shell
// Uses syscall stubs from syscalls.h

#include <stdint.h>
#include <stddef.h>
#include <syscalls/syscalls.h>

// ─────────────────────────────────────────────────────────────────────────────
// CONFIG
// ─────────────────────────────────────────────────────────────────────────────

#define MAX_INPUT   256
#define MAX_ARGS    8
#define SHELL_NAME  "ksh"
#define VERSION     "0.1"

// ─────────────────────────────────────────────────────────────────────────────
// STRING HELPERS
// ─────────────────────────────────────────────────────────────────────────────

static int sh_strlen(const char *s) {
    int i = 0; while (s[i]) i++; return i;
}

static int sh_strcmp(const char *a, const char *b) {
    while (*a && *a == *b) { a++; b++; }
    return (unsigned char)*a - (unsigned char)*b;
}

// ─────────────────────────────────────────────────────────────────────────────
// PRINT HELPERS
// ─────────────────────────────────────────────────────────────────────────────

static void println(const char *s) {
    write((uint8_t *)s);
    write((uint8_t *)"\n");
}

static void print_line(char c, int len) {
    static char buf[128];
    int i = 0;
    while (i < len && i < 126) buf[i++] = c;
    buf[i++] = '\n';
    buf[i]   = '\0';
    write((uint8_t *)buf);
}

// ─────────────────────────────────────────────────────────────────────────────
// BANNER
// ─────────────────────────────────────────────────────────────────────────────

static void print_banner(void) {
    println("");
    println("  Kernel Shell v" VERSION "  --  type 'help' to get started");
    print_line('-', 50);
    println("");
}

// ─────────────────────────────────────────────────────────────────────────────
// HELP MENU
// ─────────────────────────────────────────────────────────────────────────────

static void print_help(void) {
    println("");
    print_line('=', 50);
    println("  KSH COMMAND REFERENCE");
    print_line('=', 50);
    println("");

    println("  FILE COMMANDS");
    print_line('-', 50);
    println("  cat   <file>            print file contents");
    println("  touch <file>            create empty file");
    println("  rm    <file>            delete file");
    println("  mv    <src>  <dst>      rename / move file");
    println("  cp    <src>  <dst>      copy file");
    println("");

    println("  DIRECTORY COMMANDS");
    print_line('-', 50);
    println("  ls                      list current directory");
    println("  mkdir <dir>             create directory");
    println("  mkdir -p <path>         create nested directories");
    println("  rmdir <dir>             remove empty directory");
    println("  rmdir -r <dir>          remove directory recursively");
    println("");

    println("  NAVIGATION");
    print_line('-', 50);
    println("  cd    <path>            change directory");
    println("  pwd                     print working directory");
    println("  stat  <path>            show file / dir info");
    println("  find  <name>            search recursively");
    println("");

    println("  DISK");
    print_line('-', 50);
    println("  df                      show disk free space");
    println("  du    <file>            show file disk usage");
    println("  sync                    flush pending disk writes");
    println("");

    println("  OTHER");
    print_line('-', 50);
    println("  echo  <text>            print text to terminal");
    println("  echo  <text> > <file>   write text to file");
    println("  help                    show this menu");
    println("  clear                   clear the screen");
    println("  version                 show shell version");
    println("");

    print_line('=', 50);
    println("");
}

// ─────────────────────────────────────────────────────────────────────────────
// CLEAR SCREEN
// ─────────────────────────────────────────────────────────────────────────────

static void shell_clear(void) {
    clear();
}

// ─────────────────────────────────────────────────────────────────────────────
// VERSION
// ─────────────────────────────────────────────────────────────────────────────

static void print_version(void) {
    println("");
    println("  " SHELL_NAME " version " VERSION);
    println("  built for x86 bare-metal");
    println("");
}

// ─────────────────────────────────────────────────────────────────────────────
// PARSER
// ─────────────────────────────────────────────────────────────────────────────

static int parse(char *input, char *argv[], int max_args) {
    int argc = 0;
    char *p  = input;

    while (*p && argc < max_args) {
        while (*p == ' ') p++;
        if (!*p) break;
        argv[argc++] = p;
        while (*p && *p != ' ') p++;
        if (*p == ' ') *p++ = '\0';
    }
    return argc;
}

static int find_redirect(char *argv[], int argc) {
    for (int i = 0; i < argc; i++)
        if (argv[i][0] == '>' && argv[i][1] == '\0')
            return i;
    return -1;
}

// ─────────────────────────────────────────────────────────────────────────────
// ERROR HELPER
// ─────────────────────────────────────────────────────────────────────────────

static void usage(const char *msg) {
    write((uint8_t *)"  usage: ");
    println(msg);
}

// ─────────────────────────────────────────────────────────────────────────────
// COMMAND DISPATCH
// ─────────────────────────────────────────────────────────────────────────────

static void dispatch(char *argv[], int argc) {
    if (argc == 0) return;
    char *cmd = argv[0];

    if (sh_strcmp(cmd, "help") == 0) {
        print_help();
    }

    else if (sh_strcmp(cmd, "clear") == 0) {
        shell_clear();
    }

    else if (sh_strcmp(cmd, "version") == 0) {
        print_version();
    }

    else if (sh_strcmp(cmd, "cat") == 0) {
        if (argc < 2) { usage("cat <file>"); return; }
        println("");
        cat((uint8_t *)argv[1]);
        println("");
    }

    else if (sh_strcmp(cmd, "touch") == 0) {
        if (argc < 2) { usage("touch <file>"); return; }
        touch((uint8_t *)argv[1]);
        write((uint8_t *)"  created: ");
        println(argv[1]);
    }

    else if (sh_strcmp(cmd, "rm") == 0) {
        if (argc < 2) { usage("rm <file>"); return; }
        rm((uint8_t *)argv[1]);
        write((uint8_t *)"  removed: ");
        println(argv[1]);
    }

    else if (sh_strcmp(cmd, "mv") == 0) {
        if (argc < 3) { usage("mv <src> <dst>"); return; }
        mv((uint8_t *)argv[1], (uint8_t *)argv[2]);
        write((uint8_t *)"  moved: ");
        write((uint8_t *)argv[1]);
        write((uint8_t *)" -> ");
        println(argv[2]);
    }

    else if (sh_strcmp(cmd, "cp") == 0) {
        if (argc < 3) { usage("cp <src> <dst>"); return; }
        cp((uint8_t *)argv[1], (uint8_t *)argv[2]);
        write((uint8_t *)"  copied: ");
        write((uint8_t *)argv[1]);
        write((uint8_t *)" -> ");
        println(argv[2]);
    }

    else if (sh_strcmp(cmd, "ls") == 0) {
        println("");
        print_line('-', 40);
        ls(0);
        print_line('-', 40);
        println("");
    }

    else if (sh_strcmp(cmd, "mkdir") == 0) {
        if (argc < 2) { usage("mkdir [-p] <path>"); return; }
        if (sh_strcmp(argv[1], "-p") == 0) {
            if (argc < 3) { usage("mkdir -p <path>"); return; }
            mkdir_p((uint8_t *)argv[2]);
            write((uint8_t *)"  created path: ");
            println(argv[2]);
        } else {
            mkdir((uint8_t *)argv[1]);
            write((uint8_t *)"  created dir: ");
            println(argv[1]);
        }
    }

    else if (sh_strcmp(cmd, "rmdir") == 0) {
        if (argc < 2) { usage("rmdir [-r] <dir>"); return; }
        if (sh_strcmp(argv[1], "-r") == 0) {
            if (argc < 3) { usage("rmdir -r <dir>"); return; }
            rmdir_r((uint8_t *)argv[2]);
        } else {
            rmdir((uint8_t *)argv[1]);
        }
        write((uint8_t *)"  removed dir: ");
        println(argc > 2 ? argv[2] : argv[1]);
    }

    else if (sh_strcmp(cmd, "cd") == 0) {
        if (argc < 2) { usage("cd <path>"); return; }
        cd((uint8_t *)argv[1]);
    }

    else if (sh_strcmp(cmd, "pwd") == 0) {
        uint8_t *cwd = pwd();
        if (cwd) println((char *)cwd);
    }

    else if (sh_strcmp(cmd, "stat") == 0) {
        if (argc < 2) { usage("stat <path>"); return; }
        struct stat_info info;
        println("");
        print_line('-', 40);
        stat((uint8_t *)argv[1], &info);
        print_line('-', 40);
        println("");
    }

    else if (sh_strcmp(cmd, "find") == 0) {
        if (argc < 2) { usage("find <name>"); return; }
        println("");
        find((uint8_t *)argv[1]);
        println("");
    }

    else if (sh_strcmp(cmd, "df") == 0) {
        println("");
        print_line('-', 40);
        df();
        print_line('-', 40);
        println("");
    }

    else if (sh_strcmp(cmd, "du") == 0) {
        if (argc < 2) { usage("du <file>"); return; }
        println("");
        du((uint8_t *)argv[1]);
        println("");
    }

    else if (sh_strcmp(cmd, "sync") == 0) {
        sync();
        println("  disk synced.");
    }

    else if (sh_strcmp(cmd, "echo") == 0) {
        if (argc < 2) { println(""); return; }
        int redir = find_redirect(argv, argc);
        if (redir >= 0) {
            if (redir + 1 >= argc) { println("  echo: missing filename after >"); return; }
            if (redir < 2)         { println("  echo: missing text before >");    return; }
            echo((uint8_t *)argv[1], (uint8_t *)argv[redir + 1]);
            write((uint8_t *)"  written to: ");
            println(argv[redir + 1]);
        } else {
            echo((uint8_t *)argv[1], NULL);
        }
    }

    else {
        write((uint8_t *)"  ");
        write((uint8_t *)cmd);
        println(": command not found  (type 'help' for list)");
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// PROMPT
// ─────────────────────────────────────────────────────────────────────────────

static void print_prompt(void) {
    uint8_t *cwd = pwd();
    write((uint8_t *)"[ksh ");
    if (cwd) write(cwd);
    write((uint8_t *)"]> ");
}

// ─────────────────────────────────────────────────────────────────────────────
// MAIN SHELL LOOP
// ─────────────────────────────────────────────────────────────────────────────

void shell_main(void) {
    static char  input[MAX_INPUT];
    static char *argv[MAX_ARGS];

    print_banner();

    while (1) {
        print_prompt();

        read((uint8_t *)input, MAX_INPUT);

        int len = sh_strlen(input);
        if (len > 0 && input[len - 1] == '\n') input[len - 1] = '\0';
        if (input[0] == '\0') continue;

        int argc = parse(input, argv, MAX_ARGS);
        dispatch(argv, argc);
    }
}