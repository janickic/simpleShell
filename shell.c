// Camille Janicki
// 301 208 901

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <errno.h>
#include <signal.h>
#include "list.h"
#include "common.h"

#define NUM_TOKENS (COMMAND_LENGTH / 2 + 1)
int global_count=1;
struct nodeStruct *head = NULL;


/**
 * Command Input and Processing
 */

/*
 * Tokenize the string in 'buff' into 'tokens'.
 * buff: Character array containing string to tokenize.
 *       Will be modified: all whitespace replaced with '\0'
 * tokens: array of pointers of size at least COMMAND_LENGTH/2 + 1.
 *       Will be modified so tokens[i] points to the i'th token
 *       in the string buff. All returned tokens will be non-empty.
 *       NOTE: pointers in tokens[] will all point into buff!
 *       Ends with a null pointer.
 * returns: number of tokens.
 */
int tokenize_command(char *buff, char *tokens[], _Bool *in_background)
{

	int token_count = 0;
	_Bool in_token = false;
	int num_chars = strnlen(buff, COMMAND_LENGTH);
	for (int i = 0; i < num_chars; i++) {
		switch (buff[i]) {
		// Handle token delimiters (ends):
		case ' ':
		case '\t':
		case '\n':
			buff[i] = '\0';
			in_token = false;
			break;

		// Handle other characters (may be start)
		default:
			if (!in_token) {
				tokens[token_count] = &buff[i];
				token_count++;
				in_token = true;
			}
		}
	}
	tokens[token_count] = NULL;

	if (token_count > 0 && strcmp(tokens[token_count - 1], "&") == 0) {
		*in_background = true;
		tokens[token_count - 1] = 0;
	}

	return token_count;
}

/**
 * Read a command from the keyboard into the buffer 'buff' and tokenize it
 * such that 'tokens[i]' points into 'buff' to the i'th token in the command.
 * buff: Buffer allocated by the calling code. Must be at least
 *       COMMAND_LENGTH bytes long.
 * tokens[]: Array of character pointers which point into 'buff'. Must be at
 *       least NUM_TOKENS long. Will strip out up to one final '&' token.
 *       tokens will be NULL terminated (a NULL pointer indicates end of tokens).
 * in_background: pointer to a boolean variable. Set to true if user entered
 *       an & as their last token; otherwise set to false.
 */
void read_command(char *buff, char *tokens[], _Bool *in_background)
{
	*in_background = false;

	// Read input
	int length = read(STDIN_FILENO, buff, COMMAND_LENGTH-1);

	if (length < 0  && (errno !=EINTR) ) {
		perror("Unable to read command from keyboard. Terminating.\n");
		exit(-1);
	}

	// Null terminate and strip \n.
	buff[length] = '\0';
	if (buff[strlen(buff) - 1] == '\n') {
		buff[strlen(buff) - 1] = '\0';
	}

	// Tokenize (saving original command string)
	int token_count = tokenize_command(buff, tokens, in_background);

	if (token_count == 0) {
		return;
	}

	// // Extract if running in background:
	// if (token_count > 0 && strcmp(tokens[token_count - 1], "&") == 0) {
	// 	*in_background = true;
	// 	tokens[token_count - 1] = 0;
	// }
}


/* Signal handler function */
void handle_SIGINT()
{
	write(STDOUT_FILENO, "\n", strlen("\n"));
	List_print(&head);
}

/**
 * Main and Execute Commands
*/
/**
 * Steps For Basic Shell:
 * 1. Fork a child process
 * 2. Child process invokes execvp() using results in token array.
 * 3. If in_backgr0ound is false, parent waits for
 *    child to finish. Otherwise, parent loops back to
 *    read_command() again immediately.
 */
int main(int argc, char* argv[])
{
	char input_buffer[COMMAND_LENGTH];
	char *tokens[NUM_TOKENS];

	/* set up the signal handler */
	struct sigaction handler;
	handler.sa_handler = handle_SIGINT;
	handler.sa_flags = 0;
	sigemptyset(&handler.sa_mask);
	sigaction(SIGINT, &handler, NULL);

	// Infinite loop for shell
	while (true) {

		char ter_prompt[COMMAND_LENGTH];
		write(STDOUT_FILENO,getcwd(ter_prompt, sizeof(ter_prompt)), strlen(getcwd(ter_prompt, sizeof(ter_prompt))));
		write(STDOUT_FILENO, "> ", strlen("> "));
		_Bool in_background = false;		
		read_command(input_buffer, tokens, &in_background);
		int status;

		// ENTER
		if(tokens[0]==NULL){
			continue;//do nothing
		}

		// !N or !!
		if(tokens[0][0]=='!')
		{
			// Want to fetch command from history (linked list)
			struct nodeStruct *node;
			if(strcmp(&tokens[0][1],"!")==0)
			{
				node = List_findNode(head, global_count-1);
			}else
			{
				int n = atoi(&tokens[0][1]);
				node = List_findNode(head,n);
			}

			if(node!=NULL)
			{
				strcpy(input_buffer, node->command);
				int token_count = tokenize_command(input_buffer, tokens, &in_background);

				if (token_count == 0) {
					write(STDOUT_FILENO, "Issue with tokenize_command", strlen("Issue with tokenize_command"));
					write(STDOUT_FILENO, "\n", strlen("\n"));
				}

				write(STDOUT_FILENO, node->command, strlen(node->command));
				write(STDOUT_FILENO, "\n", strlen("\n"));

			}else
			{
				write(STDOUT_FILENO, "SHELL: Unknown history command.", strlen("SHELL: Unknown history command."));
				write(STDOUT_FILENO, "\n", strlen("\n"));	
				goto reset;
			}
		}

		// check command
		if(tokens[0]==NULL){
			continue;//do nothing
		}
		// EXIT
		else if(strcmp(tokens[0],"exit")==0)
		{
			// Freeing memory before exiting
			List_free(head);
			exit(0);
		// CD
		}else if(strcmp(tokens[0],"cd")==0)
		{
			if(chdir(tokens[1])==-1)
			{
				write(STDOUT_FILENO, strerror(errno), strlen(strerror(errno)));
				write(STDOUT_FILENO, "\n", strlen("\n"));
			}
		// PWD
		}else if(strcmp(tokens[0],"pwd")==0)
		{
			char pwd_buffer[COMMAND_LENGTH];
			if(getcwd(pwd_buffer, sizeof(pwd_buffer)) !=NULL)
			{
				write(STDOUT_FILENO, pwd_buffer, strlen(pwd_buffer));
				write(STDOUT_FILENO, "\n", strlen("\n"));
			} else
			{
				write(STDOUT_FILENO, "getcwd() error", strlen("getcwd() error"));
				write(STDOUT_FILENO, "\n", strlen("\n"));
			}
		// HISTORY
		}else if(strcmp(tokens[0],"history")==0)
		{
			struct nodeStruct* node = List_createNode(global_count,"history");
			global_count++;
			List_append(&head, node);

			List_print(&head);
			goto reset;

		}else{
		
			pid_t id = fork();
			if(id==0)
			{
				//CHILD
				int check = execvp(tokens[0], tokens);
				if(check == -1)
				{
					write(STDOUT_FILENO, tokens[0], strlen(tokens[0]));
					write(STDOUT_FILENO, ": Unknown command.", strlen(": Unknown command."));
					write(STDOUT_FILENO, "\n", strlen("\n"));
					// goto reset;
					exit(0);
				}
				exit(errno);

			}else
			{
				//PARENT
				if(in_background==false)
				{
					waitpid(id,&status, 0);
				}
			}
		}

		// DEBUG: Dump out arguments:
		// for (int i = 0; tokens[i] != NULL; i++) {
		// 	write(STDOUT_FILENO, "   Token: ", strlen("   Token: "));
		// 	write(STDOUT_FILENO, tokens[i], strlen(tokens[i]));
		// 	write(STDOUT_FILENO, "\n", strlen("\n"));
		// }
		// if (in_background) {
		// 	write(STDOUT_FILENO, "Run in background.\n", strlen("Run in background.\n"));
		// }

		// Rebuilding input_buffer for history
		char *tmp = (char *)malloc(COMMAND_LENGTH);
		strcpy(tmp,tokens[0]);
		for (int i = 1; tokens[i] != NULL; i++) {
			tmp = strcat(tmp," ");
			tmp = strcat(tmp,tokens[i]);
		}
		if(in_background)
		{
			tmp = strcat(tmp," &");
		}
		// printf("\"%s\"\n", tmp);
		// Storing last command in linked list
		struct nodeStruct* node = List_createNode(global_count,tmp);
		global_count++;
		List_append(&head, node);
		free(tmp);

		reset:
		// Cleanup any previously exited background child processes
		// (The zombies)
		while (waitpid(-1, NULL, WNOHANG) > 0)
			; // do nothing.

		//Zero input_buffer and tokens
		memset(input_buffer,0,strlen(input_buffer));
		for (int i = 1; tokens[i] != NULL; i++) {
			memset(tokens[i],0,strlen(tokens[i]));
		}

	}


	return 0;
}