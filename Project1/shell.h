#ifndef HELPERS_H
#define HELPERS_H

#define MAX_INPUT_SIZE 256
#define MAX_DAEMON 10
#define _GNU_SOURCE

typedef void (*sighandler_t)(int);

char* concat(char* c1, char* c2);

char* updir(char* dir, char* buff);

char** tokenize(char* in, const char* delim);

char** io_redirect(char** tokens, int size, int rw);

int is_background(char** tokens, int size);

int has_io(char** tokens, int size);

int s_array_len(char** in);

int change_dir(char** tokens, char* current_dir, int size);

int is_piped(char* in);

int pipe_commands(char** tokens);

#endif