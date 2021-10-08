/*
 * This code implements a simple shell program
 * It supports the internal shell command "exit", 
 * backgrounding processes with "&", input redirection
 * with "<" and output redirection with ">".
 * However, this is not complete.
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>


extern char **parseline();

/*
 * Handle exit signals from child processes
 */
void sig_handler(int signal) {
  int status;
  int result = wait(&status);

  printf("Wait returned %d\n", result);
}

/*
 * The main shell function
 */ 
main() {
  int i;
  char **args; 
  int result;
  int block;
  int output;
  int input;
  //vvvvvvvvvvvvvvvvvvv
  int containsPipe;  
  char *nextCommand;
  //^^^^^^^^^^^^^^^^^^^
  char *output_filename;
  char *input_filename;

  // Set up the signal handler
  sigset(SIGCHLD, sig_handler);

  // Loop forever
  while(1) {      

    // Print out the prompt and get the input
    printf("->");
    args = parseline();

    // No input, continue
    if(args[0] == NULL)
      continue;

    // Check for internal shell commands, such as exit
    if(internal_command(args))//internal_command returns true if there is more to do
      continue;

    // Check for an ampersand
    block = (ampersand(args) == 0);//ampersand() returns zero if there is an &
//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
    // Check for pipes
    containsPipe = pipeCheck(args, &nextCommand);
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    
    // Check for redirected input
    input = redirect_input(args, &input_filename);

    switch(input) {
    case -1:
      printf("Syntax error!\n");
      continue;
     
      break;
    case 0:
      break;
    case 1:
      printf("Redirecting input from: %s\n", input_filename);
      break;
    }

    // Check for redirected output
    output = redirect_output(args, &output_filename);

    switch(output) {
    case -1:
      printf("Syntax error!\n");
      continue;
      break;
    case 0:
      break;
    case 1:
      printf("Redirecting output to: %s\n", output_filename);
      break;
    }

    // Do the command
    do_command(args, block, 
	       input, input_filename, 
	       output, output_filename, containsPipe, nextCommand);
  }
}
//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
/*
 * check for pipe in command
 */

   int pipeCheck(char **args, char **nextCommand){
        int cp = 0;
        int b = 0;
        int c = 0;
        memset(nextCommand, '\0', sizeof(nextCommand));
        for (int i = 0; args[i] != NULL; i++){
                if(args[i][0] == '|'){
                        cp = 1;
                        free(args[i]);
                        args[i] = NULL;
                }else if(cp == 1){
                        char **temp = strdup(args[i]);
                        nextCommand[c] = temp;
                        c+=1;
                        free(args[i]);
                        args[i] = NULL;
                }


        }
        return cp;
}





//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
/*
 * Check for ampersand as the last argument
 */
int ampersand(char **args) {
  int i;

  for(i = 1; args[i] != NULL; i++) ;

  if(args[i-1][0] == '&') {
    free(args[i-1]);
    args[i-1] = NULL;
    return 1;
  } else {
    return 0;
  }
  
  return 0;
}

/* 
 * Check for internal commands
 * Returns true if there is more to do, false otherwise 
 */
int internal_command(char **args) {
  if(strcmp(args[0], "exit") == 0) {
    exit(0);
  }
//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
/*
  if(strcmp(args[0], "cd")==0){
	if(args[1]!= NULL){
	//has pathname ideally. 
	//incorperate error checking
	int cdr = chdir(args[1]);
		if(cdr<0){
			printf("chdir change of directory failed\n");
			return 0;
		}else{
			printf("chdir change of directory successful\n");
			return 0;
		}
	return 0;
	}


	if(args[1] == NULL){
		char s[100];
		chdir("..");
		printf("%s\n", getcwd(s,100));
		return 0;
	}
  }

  */
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  return 0;
}

/* 
 * Do the command
 */
int do_command(char **args, int block,
	       int input, char *input_filename,
	       int output, char *output_filename,  int containsPipe, char *nextCommand) {//edited
  
  int result;
  pid_t child_id;//pid_t is a datatype to store process ids
  int status;

  // Fork the child process
  child_id = fork();

  // Check for errors in fork()
  switch(child_id) {
  case EAGAIN:
    perror("Error EAGAIN: ");
    return;
  case ENOMEM:
    perror("Error ENOMEM: ");
    
  }

  if(child_id == 0) {// look up when fork() == zero

    // Set up redirection in the child process
    if(input)
      freopen(input_filename, "r", stdin);

    if(output)
      freopen(output_filename, "w+", stdout);

    // Execute the command
    result = execvp(args[0], args);

    exit(-1);
  }
//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
if(containsPipe){
    pid_t pid1;                                // [STDIN -> terminal_input, STDOUT -> terminal_output]                       (of the parent process)
    pid_t pid2;                                  //
    int fd[2];                                   //
                                                 //
    pipe(fd);                                    // [STDIN -> terminal_input, STDOUT -> terminal_output, fd[0] -> pipe_input, fd[1] -> pipe_output]
    pid1 = fork();                               //
                                                 //
    if(pid1==0)                                  //
    {                                            // I am going to be the wc process (i.e. taking input from the pipe)
        close(fd[INPUT_END]);                    // [STDIN -> terminal_input, STDOUT -> terminal_output, fd[1] -> pipe_output] (of the WC process)
        dup2(fd[OUTPUT_END], STDIN_FILENO);      // [STDIN -> pipe_output, STDOUT -> terminal_output, fd[1] -> pipe_output]    (of the WC process)
        close(fd[OUTPUT_END]);                   // [STDIN -> pipe_output, STDOUT -> terminal_output]                          (of the WC process)
        execvp(args[0], args);                   //
    }                                            //
    else                                         //
    {                                            //
        pid2=fork();                             //
                                                 //
        if(pid2==0)                              //
        {                                        // I am going to be the ls process (i.e. producing output to the pipe)
            close(fd[OUTPUT_END]);               // [STDIN -> terminal_input, STDOUT -> terminal_output, fd[0] -> pipe_input] (of the ls process)
            dup2(fd[INPUT_END], STDOUT_FILENO);  // [STDIN -> terminal_input, STDOUT -> pipe_input, fd[0] -> pipe_input]      (of the ls process)
            close(fd[INPUT_END]);                // [STDIN -> terminal_input, STDOUT -> pipe_input]                           (of the ls process)
            execvp(nextCommand[0], nextCommand); //
        }                                        //
                                                 //
        close(fd[OUTPUT_END]);                   // [STDIN -> terminal_input, STDOUT -> terminal_output, fd[0] -> pipe_input] (of the parent process)
        close(fd[INPUT_END]);                    // [STDIN -> terminal_input, STDOUT -> terminal_output]                      (of the parent process)
        waitpid(-1, NULL, 0);                    // As the parent process - we wait for a process to die (-1) means I don't care which one - it could be either ls or wc
        waitpid(-1, NULL, 0);                    // As the parent process - we wait for the another process to die.
                                                 // At this point we can safely assume both process are completed
    }   
}


//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  // Wait for the child process to complete, if necessary
  if(block) {
    printf("Waiting for child, pid = %d\n", child_id);
    result = waitpid(child_id, &status, 0);
  }
}

/*
 * Check for input redirection
 */
int redirect_input(char **args, char **input_filename) {
  int i;
  int j;

  for(i = 0; args[i] != NULL; i++) {

    // Look for the <
    if(args[i][0] == '<') {
      free(args[i]);

      // Read the filename
      if(args[i+1] != NULL) {
	*input_filename = args[i+1];
      } else {
	return -1;
      }

      // Adjust the rest of the arguments in the array
      for(j = i; args[j-1] != NULL; j++) {
	args[j] = args[j+2];
      }

      return 1;
    }
  }

  return 0;
}

/*
 * Check for output redirection
 */
int redirect_output(char **args, char **output_filename) {
  int i;
  int j;

  for(i = 0; args[i] != NULL; i++) {

    // Look for the >
    if(args[i][0] == '>') {
      free(args[i]);

      // Get the filename 
      if(args[i+1] != NULL) {
	*output_filename = args[i+1];
      } else {
	return -1;
      }

      // Adjust the rest of the arguments in the array
      for(j = i; args[j-1] != NULL; j++) {
	args[j] = args[j+2];
      }

      return 1;
    }
  }

  return 0;
}



