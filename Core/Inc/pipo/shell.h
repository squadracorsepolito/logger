#pragma once

typedef int (*shell_command_func_t)(int argc, char **argv);

typedef struct {
  char *name;
  shell_command_func_t func;
} shell_command_t;

#define SHELL_BUFFER_SIZE 256
#define SHELL_ARGV_SIZE 16
#define SHELL_ESCAPE_BUFFER_SIZE 16
#define SHELL_HISTORY_SIZE 16

typedef enum {
  SHELL_ESCAPE_CLEAR,
} shell_escape_e;

typedef enum {
  SHELL_MODE_NORMAL,
  SHELL_MODE_ESCAPE,
} shell_mode_e;

typedef struct {
  shell_command_t *commands;
  int commands_size;
  char buffer[SHELL_BUFFER_SIZE];
  int buffer_cursor;
  int buffer_size;
  shell_mode_e mode;
  char escape_buffer[SHELL_ESCAPE_BUFFER_SIZE];
  int escape_buffer_cursor;
  char history[SHELL_HISTORY_SIZE][SHELL_BUFFER_SIZE];
  int history_head;
  int history_cursor;
} shell_t;

void shell_init(shell_t *shell);
void shell_print_prompt();
void shell_print_escape(shell_escape_e escape);
void shell_input(shell_t *shell, char c);