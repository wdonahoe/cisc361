#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "shell.h"

void interrupt_handler(){
	signal(SIGINT, SIG_DFL);
    raise(SIGINT);
}

void sus_handler(){
	signal(SIGTSTP, SIG_DFL);
	raise(SIGTSTP);
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

void fork_error(){
	printf("%s","\n Fork failed, quitting. \n");
	exit(EXIT_FAILURE);
}

int make_background(char** tokens, int len){
	pid_t pid1, pid2;
	int status, succ;

	pid1 = fork();
	if (pid1 == 0){
		setsid();
		pid2 = fork();
		if (pid2 == 0){
			chdir("/");
			umask(0);
			
			setpgid(0, 0);

			close(STDOUT_FILENO);
			close(STDIN_FILENO);
			close(STDERR_FILENO);

			tokens[len - 1] = 0;
			char* const* t = (char* const*)tokens;

			succ = execvp(*t, t);

			if (succ == -1){
				printf("shell: %s: %s\n", *t, strerror(errno));
				exit(EXIT_FAILURE);
			}
		}
		else if (pid2 > 0){
			exit(EXIT_SUCCESS);
		}
		else {
			fork_error();
		}
	}
	else if (pid1 > 0){
		waitpid(pid1, &status, WUNTRACED | WCONTINUED);
	}
	else {
		fork_error();
	}
}

int change_dir(char** tokens, char* current_dir, int size){
	int succ;
	const char* up = "..";
	char buff[MAX_INPUT_SIZE];
	strcpy(buff, current_dir);
	char* error = "shell: cd: %s: %s\n";

	if (size == 1){
		if ((succ = chdir("/home")) == -1){
			printf(error, tokens[1], strerror(errno));
		}
	}
	else if (size == 2){
		if ((strcmp(tokens[1], up) == 0)){
			char* new_dir = updir(current_dir, buff);
			if ((succ = chdir(new_dir)) == -1){
				printf(error, tokens[1], strerror(errno));
			}
		}
		else {
			if ((succ = chdir(tokens[1])) == -1){
				printf(error, tokens[1], strerror(errno));
			}
		}			
	}
	return succ;
}

int createProcess(char** tokens, char* in, int is_p){
	pid_t pID, pID1, w;
	int status;
	int code;
	const char* pipe_delim = "|";
	int len = s_array_len(tokens);

	if (is_p == 0){
		char buff[MAX_INPUT_SIZE * 4];
		strcpy(buff,in);
		char** new_tokens = tokenize(buff, pipe_delim);
		code = pipe_commands(new_tokens);
		free(new_tokens);
	}
	else if ((pID = fork()) >= 0){
		if (pID == 0){
			code = execute(tokens);	
		}		
		else {
			if ((w = waitpid(pID, &status, WUNTRACED | WCONTINUED)) == -1){
				perror("waitpid");
				exit(EXIT_FAILURE);
			}
		}
	}
	else {
		printf("%s","\n Fork failed, quitting. \n");
		exit(EXIT_FAILURE);
	}
	return code;
}

int pipe_exec(char** tokens, int* pipefd, int pipe_to_close, int in_out){
	pid_t pID, w;
	int status, succ;

	pID = fork();
	if (pID < 0){
		printf("%s\n","Fork Failed!");
	}
	else if (pID == 0){
		close(pipefd[pipe_to_close]);
		dup2(pipefd[in_out], in_out);
		char** new_tokens = tokenize(tokens[pipe_to_close]," ");
		succ = execute(new_tokens);
	}
	else{
		if ((w = waitpid(pID, &status, WUNTRACED | WCONTINUED)) == -1){
			perror("waitpid");
			exit(EXIT_FAILURE);
		}
	}
	close(pipefd[in_out]);

	return succ;
}		

int pipe_commands(char** tokens){
	int pipefd[2];
	int succ1, succ2;

	pipe(pipefd);
	succ1 = pipe_exec(tokens, pipefd, 0, STDOUT_FILENO);
	succ2 = pipe_exec(tokens, pipefd, 1, STDIN_FILENO);

	if (succ1 != succ2)
		return 0;
	else
		return 1;
}

char* readLine(char *input){
	char cwd[1024];
	getcwd(cwd, sizeof(cwd));
	char* prompt = concat(cwd, "> ");

	printf("%s",prompt);
    fgets(input, MAX_INPUT_SIZE, stdin);

	if ((strlen(input) > 0) && (input[strlen(input) - 1] == '\n'))
		input[strlen(input) - 1] = '\0';

	free(prompt);

	return input;
}

void handle_signal(int signo, sighandler_t handler){
	struct sigaction new_action, old_action;

	new_action.sa_handler = handler;
	sigemptyset(&new_action.sa_mask);
	new_action.sa_flags = 0;

	sigaction(signo, NULL, &old_action);
	if (old_action.sa_handler != SIG_IGN)
		sigaction(signo, &new_action, NULL);

}

char** io_redirect(char** tokens, int size, int rw){
	int fdesc;
	if (rw == 0){ //out
		fdesc = open(tokens[size - 1],O_WRONLY | O_CREAT, 0666);
		dup2(fdesc, STDOUT_FILENO);
	}
	else if (rw == 1){ //in
		fdesc = open(tokens[size - 1], O_RDONLY);
		dup2(fdesc, STDIN_FILENO);
	}
	close(fdesc);
	tokens[size - 2] = 0;

	return tokens;
}

int main(){
	char* input = malloc(MAX_INPUT_SIZE);
	const char* delim = " ";
	char** tokens;
	char* line;
	int is_p, last_prog;

	handle_signal(SIGINT,interrupt_handler);
	handle_signal(SIGTSTP, sus_handler);

	while(true){
		line = readLine(input);

		if (strcmp(line, "exit") != 0){
			char buff[MAX_INPUT_SIZE];
			strcpy(buff, line);

			is_p = is_piped(buff);
			tokens = tokenize(buff, delim);

			if (strcmp(tokens[0],"cd") == 0){
				char cwd[MAX_INPUT_SIZE * 4];
				getcwd(cwd, sizeof(cwd));
				change_dir(tokens, cwd, s_array_len(tokens));
			}
			else if (is_background(tokens, s_array_len(tokens)) == 0){
				make_background(tokens, s_array_len(tokens));
			}
			else if ((createProcess(tokens, line, is_p)) == 0){
				memset(buff, 0, strlen(buff));
				free(tokens);
				free(input);
				exit(EXIT_FAILURE);
			}
			else {
				free(tokens);
			}
		}
		else {
			free(input);
			exit(EXIT_SUCCESS);
		}
	}

	return 0;
}