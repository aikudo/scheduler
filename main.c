#include <stdio.h>
#include <stdlib.h>
#include "scheduler.h"
#include "shell.h"

void item (uint8_t argc, char **argv){

    for(int i = 0; i < argc; i++){
        printf("argv[%d] %s\n", i, argv[i]);
    }
    putchar('\n');
}


int main(void){
    printf("hello %d\n", stuff(3));
    Shell_ptr s = &shell_inst;

    shell_init(s);
    while(1){
        shell_loop(s);
    }
    return 0;
}
