//Author: Ngoc Duong
//Date: 09/19/2018
//This program simulates the Unix sh for command processing

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> //chdir(), fork()
#include <sys/wait.h> //wait()
#include <sys/stat.h> //open()
#include <fcntl.h> //O_RDONLY, O_APPEND, O_WRONLY, O_CREAT

//helper functions' declarations
void set_env(char *env[]);
void tokenize(char * s, char * symbol, char *result[], int *len);
void find_home();
void find_path();
char * prompt_command();
char * check_redirection(int *index);

//part1 funtion declarations
void handle_commands();
void handle_cd();
void handle_exit();
void handle_other_command();
void execute_command(char *my_args[]);
void handle_redirection(char *file_name, char *direction);

//part2 funtion declarations
int check_pipe();
void break_pipe_command(char *head_ARGS[], 
	char *tail_ARGS[], int pipe_index);
void handle_pipe(char *head_ARGS[], char *tail_ARGS[]);
void child_execution();

//global variables
char HOME[50]; //store the HOME path retrieved from env
char *PATH[20]; //store paths from PATH retrieved from env
int LEN_PATH;
char *MY_ARGS[20]; //store all command args including
                    //redirection and args go after
int LEN_MY_ARGS;
char *MY_ENVP[100];


//main funtion
int main(int argc, char *argv[], char *env[])
{	
  //set up variables
  find_home(); //var HOME
  find_path(); //var PATH, LEN_PATH

  char *input_line = prompt_command();
  tokenize(input_line, " ", MY_ARGS, &LEN_MY_ARGS); //var MY_ARGS, LEN_MY_ARGS
  set_env(env); //var MY_ENVP

  //execute the program
  handle_commands();

  return 0;
}

//Helper functions
/******************************************************/

void set_env(char *env[])
{
  int i = 0;
  while (env[i] != NULL)
  {
    MY_ENVP[i] = env[i];
    i++;
  }
}

//tokenize a string by a symbol
//return result and len of the result
void tokenize(char * s, char * symbol, char *result[], int *len)
{
  int i = 0;

  if (s[strlen(s) - 1] == '\n')
  	s[strlen(s)-1] = '\0';	//delete the last \n

  char *tok = strtok(s, symbol);
  while (tok != NULL)
    {
      result[i] = tok;
      tok = strtok(NULL, symbol);
      i++;
    }
  
  *len = i;
}

//find HOME in the env
void find_home()
{
    strcpy(HOME, getenv("HOME"));
}

//find PATH in the env
void find_path()
{
  int i = 0;
  char *path = getenv("PATH");  

  if (path != NULL) //break path down to make PATH
  {
	tokenize(path, ":", PATH, &LEN_PATH);
  }
}

//prompt an input line and return the input
char * prompt_command()
{
  char *line = (char *) malloc(128*sizeof(char));
  printf("Command: ");
  fgets(line, 128, stdin);

  return line;
}

//check for redirection
//return a string of either ">","<", or ">>"
//return NULL if there is no redirection
//return index of the derirction symbol
char *check_redirection(int *index)
{ 
  char *result = NULL;
  for (int i = 0; i < LEN_MY_ARGS; i++)
    {
      if (strcmp(MY_ARGS[i], "<") == 0 ||
	  strcmp(MY_ARGS[i], ">") == 0 ||
	  strcmp(MY_ARGS[i], ">>") == 0)
	{
	  result = MY_ARGS[i];
	  *index = i;
	  break;
	}
    }
  return result;
}

//Part 1: Single Command with I/O Redirection
/******************************************************/

//handle all commands
void handle_commands()
{
  char *command = MY_ARGS[0];
  if (strcmp(command,"cd")==0)
    handle_cd();
  else if (strcmp(command, "exit") == 0)
    handle_exit();
  else
    handle_other_command();
}

//Handle command = "cd"
void handle_cd()
{ 
  int ret = 0;
  if (LEN_MY_ARGS == 1)
    ret = chdir(HOME);
  else
    ret = chdir(MY_ARGS[1]);
  if (ret==-1)
    printf("Invalid path\n");
}

void handle_exit()
{
  exit(1);
}


//handle commands different than "cd" and "exist"
void handle_other_command()
{
  int status = 0;
  int pid = fork();

  if (pid < 0)
  {
    printf("Fork failed\n");
    exit(1);
  }
  else if (pid) //parent's execution
    {
      pid = wait(&status); //wait for a dead child
      printf("Child's exit status code: %d\n", status);
    }
  else //child's execution
    {
	child_execution();
    }
}

//execute command by execve()
void execute_command(char *my_args[])
{
  int index_redi = 0;
  char *redi = NULL;
  int i = 0;

  redi = check_redirection(&index_redi);
  //Handle redirection
  if (redi != NULL)
  {
    handle_redirection(my_args[index_redi + 1], redi);
    my_args[index_redi] = NULL;
  }

  //executing the command
  for (int i = 0; i < LEN_PATH; i++)
  {
    char file_path[50];
    strcpy(file_path, PATH[i]);
    strcat(file_path, "/");
    strcat(file_path, my_args[0]);
    execve(file_path, my_args, MY_ENVP);
  }
  printf("Invalid command\n");
}

//Handle I/O redirection
void handle_redirection(char *file_name, char *direction)
{
  if (strcmp(direction, ">") == 0) //replace stdout with the file
    {
      close(1);
      open(file_name, O_WRONLY|O_CREAT, 0644);
    }
  else if (strcmp(direction, "<") == 0) //replace stdin with the file
    {
      close(0);
      open(file_name, O_RDONLY);
    }
  else if (strcmp(direction, ">>") == 0) //replace stdout with file to append
    {
      close(1);
      open(file_name, O_APPEND|O_WRONLY|O_CREAT, 0644);
    }
  else
    {
      puts("Invalid redirection");
    }
}

//Part 2: Commands with Pipes
/******************************************************/

//return 0 if there is no pipe or return the pipe index
int check_pipe()
{
  int result = 0;
  for (int i = 0; i < LEN_MY_ARGS; i++)
    {
      if (strcmp(MY_ARGS[i], "|") == 0)
	{
	  result = i;
	  break;
	}
    }
  return result;
}

//break down a command that has a pipe to
//head_ARGS and tail_ARGS
//pre-condition: the command has at least a pipe
void break_pipe_command(char *head_ARGS[], 
	char *tail_ARGS[], int pipe_index)
{
  //make a copy of MY_ARGS
  char args[50][20];
  for (int i = 0; i < LEN_MY_ARGS; i++)
  {
    strcpy(args[i], MY_ARGS[i]);
  }  
  //make head and tail
  for (int i = 0; i < pipe_index; i++)
  {
    head_ARGS[i] = args[i];
  }
  for (int i = pipe_index + 1; i < LEN_MY_ARGS; i++)
  {
    tail_ARGS[i-pipe_index-1] = args[i];
  }
  //add NULL to the end of head and tail
  head_ARGS[pipe_index] = NULL;
  tail_ARGS[LEN_MY_ARGS - 1 - pipe_index] = NULL;
}

//child sh handle pipe, redirection, and execution
void child_execution()
{
  int pipe_index = check_pipe();
  char *head_ARGS[20];
  char *tail_ARGS[20];

  if (pipe_index == 0) //no pipe
  {
    execute_command(MY_ARGS);
  }
  else //there is a pipe
  {
    break_pipe_command(head_ARGS, tail_ARGS, pipe_index);
    handle_pipe(head_ARGS, tail_ARGS);
  }
}

//handle commands with pipe
void handle_pipe(char *head_ARGS[], char *tail_ARGS[])
{
  int pd[2];
  pipe(pd); //create PIPE, pd[0] for read from the pipe
	    //, pd[1] for write to the pipe
  int pid = fork();

  if (pid < 0)
  {
    printf("Pipe fork failed\n");
    exit(1);
  }
  else if (pid) //parent, to write
  {
    close(pd[0]);
    close(1);
    dup(pd[1]);
    close(pd[1]);
    printf("head[0]=%s\n", head_ARGS[0]);//test
    execute_command(head_ARGS);
  }
  else //child, to read
  {
    close(pd[1]);
    close(0);
    dup(pd[0]);
    close(pd[0]);
    printf("tail[0]=%s\n", tail_ARGS[0]);//test
    execute_command(tail_ARGS);
  }
}
