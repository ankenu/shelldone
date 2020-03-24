#ifndef __SHELL_BODY_H__
#define __SHELL_BODY_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <pwd.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#define TRUE    1
#define FALSE   0

enum    command_error
{
    OK
};

enum    command_name
{
    NONE,
    EXIT,
    CD,
    PWD,
    HISTORY
};

enum    output_types
{
    NO,
    REWRITE,
    APPEND
};

typedef struct
{
    enum command_name   name;
    int                 count_arg;
    char**              arg;
    char*               input_file;
    char*               output_file; /* NULL - no redirection */
    enum output_types   output_type; /* 1 - rewrite, 2 - append */
} program;

typedef struct
{
    int         foreground;
    pid_t       group_id;
    program**   prog; //programs
    int         count_prog;
} job;

// typedef struct
// {
//     char**      str;
//     int         count_str;
//     int         max_num;
//     int         difference;
// } hist;

typedef struct type_node
{
    struct type_node*   next;
    struct type_node*   prev;
    char*               str;
    int                 num;
}node;

typedef struct 
{
    node*   head;
    node*   tail;
    node*   prevhist;
    int     max_num;
}main_node;

char*       cmd_all[] = {"exit", "cd", "pwd", "history"};
//hist        story = {NULL, 0, 100, 0};
main_node   main_hist = {NULL, NULL, NULL, 100};
main_node*  hist = &main_hist;   
int         argc_main;
char**      argv_main; 
char*       input_str;

void    delete_history();
char*   delete_comments(char* str);

void    shell_exit();

char*   command_input(char* str);
void    command_parser(char* str);
void    job_controller(char* str);
void    command_line();

#endif /* __SHELL_BODY_H__ */