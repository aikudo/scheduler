#include "shell.h"

#define false 0
#define true 1
#define PROMPT "> "
#define LINE_SIZE 80
#define MAX_COMMAND 10
#define MAX_ARGS 5

struct Command_tag{
    char *cmdstr;
    char *description;
    ShellCallBack exec;
};

struct Shell_tag{
    char line [LINE_SIZE];
    uint8_t line_length;
    uint8_t argc;
    char *argv[MAX_ARGS];
    uint8_t line_complete;
    uint8_t echo;
    Command command_list[MAX_COMMAND];
};

Shell shell_inst; // only one shell

static void read_line(Shell_ptr s){
    unsigned char c = getchar();

    switch (c) {
    case 0x04 : // ctrl-D
        printf("^D\r\n");
        s->line[s->line_length] = 0;
        s->line_complete = true;
        return;

    case 0x7f : // delete
    case 0x08 : // ctrl-H
        if (s->line_length > 0) {
            putchar(0x08);
            printf(" ");
            putchar(0x08);
            --s->line_length;
        }
        return;

    case 0x15 : // ctrl-U
        printf("^U\r\n%s", PROMPT);
        s->line[s->line_length = 0] = 0;
        return;

    case '\r' :
        printf("\r\n");
        s->line[s->line_length] = 0;
        s->line_complete = true;
        return;

    default :
        if (c < ' ' || c >= 0x80) {
            printf("<0x%02x>", c);
            return;
        }
    }

    if (s->line_length > LINE_SIZE) return;
    if (s->echo) putchar(c);
    s->line[s->line_length++] = c;
}

static void parse_line(Shell_ptr s) {
    s->argc = 0;
    uint8_t i = 0;
    while (i < s->line_length && s->argc < MAX_ARGS) {
        // start argument word
        s->argv[s->argc++] = &s->line[i];
        while (i < s->line_length && s->line[i] != ' ') i++;
        s->line[i++] = 0;

        // skip whitespace
        while (i < s->line_length && s->line[i] == ' ') i++;
    }
}


static void shell_help(uint8_t argc, char** argv){
}

void shell_init(Shell_ptr s){
    s->command_list[0].cmdstr = "help";
    s->command_list[0].description = "Simple menu";
    s->command_list[0].exec = shell_help;
    printf("Welcome shell\n");
}

void shell_echo(void);
void shell_color(void);


static Command_ptr find_command(Shell_ptr s, char *cmdstr){
    printf("here\n");
    if(cmdstr[0] == '?') cmdstr = "help";   //does this work?
    for(uint8_t i = 0; i < MAX_COMMAND; i++){
        if(strcmp(s->command_list[i].cmdstr, cmdstr) == 0 ){
            return (&s->command_list[i]);
        }
    }
    return 0;
}

void shell_loop(Shell_ptr s){
    read_line(s);
    if (!s->line_complete) return;
    parse_line(s);
    if (s->argc > 0) {
        Command_ptr cmd = find_command(s, s->argv[0]);

        if (cmd) {
            cmd->exec(s->argc, s->argv);
            printf("\r\n");
        } else {
            printf("%s ?\r\n\n", s->argv[0]);
        }
    }
    s->line_complete = false;
    s->line_length = 0;
    printf(PROMPT);
}

uint8_t shell_insert(Shell_ptr s, char *cmd, char *description, ShellCallBack fn){
    for(uint8_t i = 1; i < MAX_COMMAND; i++){
        if(s->command_list[i].cmdstr == NULL){
            s->command_list[i].cmdstr = cmd;
            s->command_list[i].description = description;
            s->command_list[i].exec = fn;
            return i;
        }
    }
    return 0;
}

