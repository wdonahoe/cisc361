#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "shell.h"

char* updir(char* dir, char* buff){
	int i = 0;
	int j = strlen(dir);
	while (dir[j] != '/'){
		i++;
		j--;
	}
	int len = strlen(dir) - i;
	strcpy(buff, dir);
	buff[len] = '\0';

	return buff;
}

char* concat(char* c1,char* c2){
	int len1 = strlen(c1);
	int len2 = strlen(c2);

	char* result = malloc(len1 + len2 + 1);

	memcpy(result, c1, len1);
	memcpy(result + len1, c2, len2 + 1);

	return result;
}

int is_background(char** tokens, int size) {
	int val = 1;

	if (strcmp(tokens[size - 1], "&") == 0) val = 0;

	return val;
}

int has_io(char** tokens, int size){
	int val = -1;

	if (strcmp(tokens[size - 2], ">") == 0)
		val = 0;
	else if (strcmp(tokens[size - 2], "<") == 0)
		val = 1;

	return val;
}

char** tokenize(char* in, const char* delim){
	char* p = strtok(in, delim);
	int n_spaces = 0;
	char** res = NULL;

	while (p) {
  		res = realloc(res, sizeof(char*) * ++n_spaces);

  		if (res == NULL)
    		exit(-1);

  		res[n_spaces - 1] = p;

 		p = strtok(NULL, delim);
	}

	res = realloc(res, sizeof(char*) * (n_spaces + 1));
	res[n_spaces] = 0;

	return res;
}

int is_piped(char* in){
	int i = 0;
	int len = strlen(in);
	while (in[i] != '|' && i < len){
		i++;
	}
	return (i == len);
}

int s_array_len(char** in){
	int count = 0;
	while (in[count] != NULL) count++;

	return count;
}

/*
int basic_execute(char** tokens, int piped, int* pipefd, int pipe_to_close, int in_out){
	int succ, is_p;
	char** new_tokens;

	if (piped){
		close(pipefd[pipe_to_close]);
		dup2(pipefd[in_out], in_out);
		new_tokens = tokenize(tokens[pipe_to_close]," ");
		succ = execute(new_tokens);
	}
	else {
		succ = execute(tokens);
	}

	return succ;
}


int execute(char** tokens){
	char* buff[MAX_INPUT_SIZE];
	char* const* t;

	int len = s_array_len(tokens);
	memcpy(buff, tokens, MAX_INPUT_SIZE);

	int io = has_io(buff, len);
	if (io >= 0)
		t = (char* const*)io_redirect(tokens, len, io);	
	else 
		t = (char* const*)tokens;

	if ((execvp(*t, t)) == -1){
		printf("shell: %s: %s\n", *t, strerror(errno));
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

*/

/*

int piping(){
	int pipefd[2];
	int succ1, succ2;

	pipe(pipefd);
	succ1 = basic_fork(tokens, 0, pipefd, 0, STDOUT_FILENO);
	succ2 = pipe_exec(tokens, 0, pipefd, 1, STDIN_FILENO);

	return (succ1 != succ2);
}
int basic_fork(char** tokens, int piped, int* pipefd, int pipe_to_close, int in_out){
	int pID, w, succ;

	if ((pID = fork())){
		succ = basic_execute(tokens, piped, pipefd, pipe_to_close, in_out);
	}
	else if (pID > 0){
		if ((w = waitpid(pID, &status, WUNTRACED | WCONTINUED)) == -1){
			perror("waitpid");
			exit(EXIT_FAILURE);
		}
	}
	else{
		printf("%s\n","Fork failed!");
		exit(EXIT_FAILURE);
	}
	return succ;
}
*/
