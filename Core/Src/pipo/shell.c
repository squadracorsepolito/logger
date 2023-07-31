#include "pipo/shell.h"

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

typedef void (*shell_escape_handler_func_t)(shell_t *shell);

typedef struct
{
  char *name;
  shell_escape_handler_func_t func;
} shell_escape_handlers_t;

void shell_init(shell_t *shell)
{
  shell->commands = NULL;
  shell->commands_size = 0;
  memset(shell->buffer, 0, SHELL_BUFFER_SIZE);
  shell->buffer_cursor = 0;
  shell->buffer_size = 0;
  shell->mode = SHELL_MODE_NORMAL;
  memset(shell->escape_buffer, 0, 16);
  shell->escape_buffer_cursor = 0;
  memset(shell->history, 0, SHELL_HISTORY_SIZE * SHELL_BUFFER_SIZE);
  shell->history_head = 0;
  shell->history_cursor = 0;
}

void shell_print_escape(shell_escape_e escape)
{
  switch (escape)
  {
  case SHELL_ESCAPE_CLEAR:
    write(STDOUT_FILENO, "\e[2J", 4);
    write(STDOUT_FILENO, "\e[H", 3);
    break;

  default:
    break;
  }
}

void shell_print_prompt()
{
  write(STDOUT_FILENO, "> ", 2);
}

void _shell_input_up(shell_t *shell)
{
  int previous = shell->history_cursor > 0 ? shell->history_cursor - 1 : SHELL_HISTORY_SIZE;
  if (!isprint((int)shell->history[previous][0]) || previous == shell->history_head)
    return;

  memcpy(shell->buffer, shell->history[previous], SHELL_BUFFER_SIZE);
  shell->buffer_cursor = shell->buffer_size = strlen(shell->buffer);
  shell->history_cursor = previous;
  write(STDOUT_FILENO, "\e[2K", 4); // clear line
  write(STDOUT_FILENO, "\e[1G", 4); // move cursor to beginning of line
  shell_print_prompt();
  write(STDOUT_FILENO, shell->buffer, shell->buffer_size);
}

void _shell_input_down(shell_t *shell)
{
  int next = (shell->history_cursor + 1) % SHELL_HISTORY_SIZE;

  if (next == shell->history_head)
  {
    shell->buffer[0] = '\0';
    shell->buffer_cursor = shell->buffer_size = 0;
  }
  else
  {
    if (!isprint((int)shell->history[next][0]))
      return;

    memcpy(shell->buffer, shell->history[next], SHELL_BUFFER_SIZE);
    shell->buffer_cursor = shell->buffer_size = strlen(shell->buffer);
  }

  shell->history_cursor = next;
  write(STDOUT_FILENO, "\e[2K", 4); // clear line
  write(STDOUT_FILENO, "\e[1G", 4); // move cursor to beginning of line
  shell_print_prompt();
  write(STDOUT_FILENO, shell->buffer, shell->buffer_size);
}

void _shell_input_right(shell_t *shell)
{
  if (shell->buffer_cursor < shell->buffer_size)
  {
    shell->buffer_cursor++;
    write(STDOUT_FILENO, "\e[C", 3);
  }
}

void _shell_input_left(shell_t *shell)
{
  if (shell->buffer_cursor > 0)
  {
    shell->buffer_cursor--;
    write(STDOUT_FILENO, "\e[D", 3);
  }
}

void _shell_input_delete(shell_t *shell)
{
  if (shell->buffer_size > shell->buffer_cursor)
  {
    int delta = shell->buffer_size - shell->buffer_cursor;
    shell->buffer_size--;
    memmove(&shell->buffer[shell->buffer_cursor], &shell->buffer[shell->buffer_cursor + 1], delta);
    write(STDOUT_FILENO, " \b", 3);
    write(STDOUT_FILENO, shell->buffer + shell->buffer_cursor, delta - 1);
    write(STDOUT_FILENO, " ", 1);
    for (int i = 0; i < delta; i++)
    {
      write(STDOUT_FILENO, "\b", 1);
    }
  }
}

shell_escape_handlers_t _escape_handlers[] = {
    {"[A", _shell_input_up},
    {"[B", _shell_input_down},
    {"[C", _shell_input_right},
    {"[D", _shell_input_left},
    {"[3~", _shell_input_delete},
};

void _shell_input_escape(shell_t *shell, char c)
{
  shell->escape_buffer[shell->escape_buffer_cursor++] = c;

  // are we still interested in this escape sequence?
  for (int i = 0; i < sizeof(_escape_handlers) / sizeof(shell_escape_handlers_t); i++)
  {
    if (strncmp(_escape_handlers[i].name, shell->escape_buffer, shell->escape_buffer_cursor) == 0)
    {
      if (strlen(_escape_handlers[i].name) == shell->escape_buffer_cursor)
      {
        _escape_handlers[i].func(shell);
        shell->escape_buffer_cursor = 0;
        shell->mode = SHELL_MODE_NORMAL;
      }
      return;
    }
  }

  // not interested in this escape sequence
  shell->escape_buffer_cursor = 0;
  shell->mode = SHELL_MODE_NORMAL;
}

void _shell_input_backspace(shell_t *shell)
{
  if (shell->buffer_cursor > 0)
  {
    int delta = shell->buffer_size - shell->buffer_cursor;
    shell->buffer_size--;
    shell->buffer_cursor--;
    memmove(&shell->buffer[shell->buffer_cursor], &shell->buffer[shell->buffer_cursor + 1], delta);
    write(STDOUT_FILENO, "\b", 1);
    write(STDOUT_FILENO, &shell->buffer[shell->buffer_cursor], delta);
    write(STDOUT_FILENO, " ", 1);
    for (int i = 0; i < delta + 1; i++)
    {
      write(STDOUT_FILENO, "\b", 1);
    }
  }
}

void _shell_input_default(shell_t *shell, char c)
{
  if (shell->buffer_cursor < SHELL_BUFFER_SIZE)
  {
    if (shell->buffer_cursor == shell->buffer_size)
    {
      shell->buffer[shell->buffer_cursor] = c;
      shell->buffer_cursor++;
      shell->buffer_size++;
      write(STDOUT_FILENO, &c, 1);
    }
    else
    {
      int delta = shell->buffer_size - shell->buffer_cursor;
      shell->buffer_size++;
      memmove(&shell->buffer[shell->buffer_cursor + 1], &shell->buffer[shell->buffer_cursor], delta);
      shell->buffer[shell->buffer_cursor] = c;
      shell->buffer_cursor++;
      write(STDOUT_FILENO, &c, 1);
      write(STDOUT_FILENO, shell->buffer + shell->buffer_cursor, delta);
      for (int i = 0; i < delta; i++)
      {
        write(STDOUT_FILENO, "\b", 1);
      }
    }
  }
}

void _shell_input_cr(shell_t *shell)
{
  shell->buffer[shell->buffer_size] = '\0';

  write(STDOUT_FILENO, "\r\n", 2);

  if (shell->buffer_size > 0)
  {
    memcpy(shell->history[shell->history_head], shell->buffer, SHELL_BUFFER_SIZE);
    shell->history_head = (shell->history_head + 1) % SHELL_HISTORY_SIZE;
    shell->history_cursor = shell->history_head;

    char *argv[SHELL_ARGV_SIZE];
    int argc = 0;
    char *token = strtok(shell->buffer, " ");
    
    while (token != NULL && argc < SHELL_ARGV_SIZE)
    {
      argv[argc++] = token;
      token = strtok(NULL, " ");
    }

    shell_command_func_t func = NULL;
    for (int i = 0; i < shell->commands_size; i++)
    {
      if (strcmp(argv[0], shell->commands[i].name) == 0)
      {
        func = shell->commands[i].func;
      }
    }

    if (func)
    {
      func(argc, argv);
    }
    else
    {
      printf("ERR: unknown command %s\r\n", argv[0]);
    }
  }

  shell->buffer_size = 0;
  shell->buffer_cursor = 0;
  shell_print_prompt(shell);
}

void _shell_input_normal(shell_t *shell, char c)
{
  switch (c)
  {
  case '\e':
    shell->mode = SHELL_MODE_ESCAPE;
    break;
  case '\r':
    return _shell_input_cr(shell);
  case '\b':
  case 0x7F:
    return _shell_input_backspace(shell);
  default:
    return _shell_input_default(shell, c);
  }
}

void shell_input(shell_t *shell, char c)
{
  switch (shell->mode)
  {
  case SHELL_MODE_NORMAL:
    return _shell_input_normal(shell, c);
  case SHELL_MODE_ESCAPE:
    return _shell_input_escape(shell, c);
  }
}