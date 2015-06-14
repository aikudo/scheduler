#ifndef __SHELL_H__
#define __SHELL_H__
#include <stdio.h>
#include <string.h>
#include <stdint.h>
typedef void (*ShellCallBack)(uint8_t, char**);
typedef struct Command_tag Command, *Command_ptr;

/**
 * shell_loop
 *
 * This is a the main body of the shell that can be 
 * inserted in the big main spin loop.
 *
 * Returns
 *  None
 */
void shell_loop(void);

/**
 * shell_insert
 *
 * Insert a shell command and its description to a command list.
 * It's assummed that the user does not use overlapsed keywords.
 *
 * Returns
 *  An index of the command list or large number like 100 if it fails
 */
uint8_t shell_insert(char *cmd, char *description, ShellCallBack fn);


/*
void shell_welcome(void);
void shell_echo(void);
void shell_color(void);
void shell_remove(uint16_t cmdid);
*/


#endif //__SHELL_H__
