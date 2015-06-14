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


static char line [LINE_SIZE];
static uint8_t line_length;
static uint8_t argc;
static char *argv[MAX_ARGS];
static uint8_t line_complete;
static uint8_t echo = true;
static Command command_list[MAX_COMMAND];

static void read_line(void){
    unsigned char c = getchar();

    switch (c) {
    case 0x04 : // ctrl-D
        printf("^D\r\n");
        line[line_length] = 0;
        line_complete = true;
        return;

    case 0x7f : // delete
    case 0x08 : // ctrl-H
        if (line_length > 0) {
            putchar(0x08);
            printf(" ");
            putchar(0x08);
            --line_length;
        }
        return;

    case 0x15 : // ctrl-U
        printf("^U\r\n%s", PROMPT);
        line[line_length = 0] = 0;
        return;

    case '\r' :
        printf("\r\n");
        line[line_length] = 0;
        line_complete = true;
        return;

    default :
        if (c < ' ' || c >= 0x80) {
            //printf("<0x%02x>", c);
            return;
        }
    }

    if (line_length > LINE_SIZE) return;
    if (echo) putchar(c);
    line[line_length++] = c;
}

static void parse_line(void) {
    argc = 0;
    uint8_t i = 0;
    while (i < line_length && argc < MAX_ARGS) {
        // start argument word
        argv[argc++] = &line[i];
        while (i < line_length && line[i] != ' ') i++;
        line[i++] = 0;

        // skip whitespace
        while (i < line_length && line[i] == ' ') i++;
    }
}


static void shell_help(int argc, char** argv){

}
void shell_init(void){
    command_list[0].cmdstr = "help";
    command_list[0].description = "Simple menu";
    command_list[0].exec = shell_help;
}

void shell_echo(void);
void shell_color(void);


static Command_ptr find_command(char *cmdstr){
    if(cmdstr[0] == '?') cmdstr = "help";   //does this work?
    for(uint8_t i = 0; i < MAX_COMMAND; i++){
        if(strcmp(command_list[i].cmdstr, cmdstr) == 0 ){
            return (&command_list[i]);
        }
    }
    return 0;
}

void shell_loop(void){
    read_line();
    if (!line_complete) return;
    parse_line();
    if (argc > 0) {
        Command_ptr cmd = find_command(argv[0]);

        if (cmd) {
            cmd->exec(argc, argv);
            printf("\r\n");
        } else {
            printf("%s ?\r\n\n", argv[0]);
        }
    }
    line_complete = false;
    line_length = 0;
    printf(PROMPT);
}

uint8_t shell_insert(char *cmd, char *description, ShellCallBack fn){
    for(uint8_t i = 1; i < MAX_COMMAND; i++){
        if(command_list[i].cmdstr == NULL){
            command_list[i].cmdstr = cmd;
            command_list[i].description = description;
            command_list[i].exec = fn;
            return i;
        }
    }
    return 0;
}

