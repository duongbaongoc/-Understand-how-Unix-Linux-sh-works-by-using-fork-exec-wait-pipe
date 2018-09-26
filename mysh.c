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
void find_home(char *env[]);
void find_path(char *env[]);
char * prompt_command();
char * check_redirection(int *index);

//part1 funtion declarations
void handle_commands();
void handle_cd();
void handle_exit();
void handle_other_command();
void execute_command();
void handle_redirection(char *file_name, char *direction);

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
  find_home(env); //var HOME
  find_path(env); //var PATH, LEN_PATH

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
void find_home(char *env[])
{
  int i = 0;
  while (env[i] != NULL)
    {
      if (strncmp(env[i], "HOME=", 5) == 0) //found the HOME
	{
	  strcpy(HOME, env[i]+5);
	  break;
	}
      i++;
      }
}

//find PATH in the env
void find_path(char *env[])
{
  int i = 0;

  char *path;
  while (env[i] != NULL)
    {
      if (strncmp(env[i], "PATH=", 5) == 0) //found the PATH
	{
	  path = env[i]+5;
	  break;
	}
      i++;
    }

  if (env[i] != NULL) //break path down to make PATH
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
    printf("Fold failed\n");
    exit(1);
  }
  else if (pid) //parent's execution
    {
      pid = wait(&status); //wait for a dead child
      printf("Child's exit status code: %d\n", status);
    }
  else //child's execution
    {
      execute_command();
    }
}


//execute command by execve()
void execute_command()
{
  int index_redi = 0;
  char *redi = NULL;
  char *args[20];
  int i = 0;
  while (MY_ARGS[i] != NULL)
  {
    args[i] = MY_ARGS[i];
    i++;
  } //copy MY_ARGS over to args

  redi = check_redirection(&index_redi);

  //Handle redirection
  if (redi != NULL)
  {
    handle_redirection(MY_ARGS[index_redi + 1], redi);
    args[index_redi] = NULL;
  }

  //executing the command
  for (int i = 0; i < LEN_PATH; i++)
  {
    char file_path[50];
    strcpy(file_path, PATH[i]);
    strcat(file_path, "/");
    strcat(file_path, args[0]);
    execve(file_path, args, MY_ENVP);
  }
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
      open(file_name, O_APPEND|O_CREAT, 0644);
    }
  else
    {
      puts("Invalid redirection");
    }
}
/****************************************************************/






