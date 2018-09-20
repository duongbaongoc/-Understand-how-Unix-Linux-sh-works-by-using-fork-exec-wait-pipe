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

//function declarations
int prompt_command(char args[][100]);
void handle_cd(char *path, char *home);
char * find_home(char *env[]);
void handle_other_command();
char *check_redirection(char args[][100], int argc);
void handle_redirection(char *file_name, char *direction);

//main funtion
int main(int argc, char *argv[], char *env[])
{
  /*char args[10][100];
  int len = prompt_command(args);
  printf("%d\n",len);
  char *redirect = check_redirection(args, len);
  if (redirect != NULL)
    puts(redirect);*/
  handle_redirection("file_name", ">");
  return 0;
}

//Part 1: Single Command with I/O Redirection
/******************************************************/

//prompt an input line and return the input
//as a list of arguments
//return the number of arguments
int prompt_command(char args[][100])
{
  int i = 0;
  char line[128];
  printf("Command: ");
  fgets(line, 128, stdin);

  char *tok = strtok(line, " ");
  while (tok != NULL)
    {
      strcpy(args[i], tok);
      tok = strtok(NULL, " ");
      i++;
    }
  
  return i;
}

//Handle command = "cd"
void handle_cd(char *path, char *home)
{
  int ret = 0;
  if (strcmp(path, "") == 0)
    ret = chdir(home);
  else
      ret = chdir(path);
  if (ret==-1)
    printf("Invalid path\n");
}

//find $HOME in the env
char * find_home(char *env[])
{
  int i = 0;
  char *home;
  while (env[i] != NULL)
    {
      if (strncmp(env[i], "HOME=", 5) == 0) //found the HOME
	{
	  home = env[i]+5;
	  break;
	}
      i++;
      }

  if (env[i] == NULL)
    home = "Could not find HOME";
  return home;
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
      printf("Process the child\n");//child_process(); //implement later
    }
}

//check for redirection
//return a string of either ">","<", or ">>"
//return NULL if there is no redirection
char *check_redirection(char args[][100], int argc)
{
  char * result = NULL;
  for (int i = 0; i < argc; i++)
    {
      if (strcmp(args[i], "<") == 0 ||
	  strcmp(args[i], ">") == 0 ||
	  strcmp(args[i], ">>") == 0)
	{
	  result = args[i];
	  break;
	}
    }
  return result;
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
