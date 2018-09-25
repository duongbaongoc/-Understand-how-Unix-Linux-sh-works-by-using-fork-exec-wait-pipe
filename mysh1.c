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
void tokenize(char * s, char * symbol, char *result[], int *len);
char * prompt_command();

int LEN_MY_ARGS;
char *MY_ARGS[20]; //store all command args including
                    //redirection and args go after
//main funtion
int main(int argc, char *argv[], char *env[])
{
  
  char *line = prompt_command(); //var MY_ARGS, LEN_MY_ARGS 
  tokenize(line, " ", MY_ARGS, &LEN_MY_ARGS);

  for (int i = 0; i < LEN_MY_ARGS; i++)
	printf("%s\n", MY_ARGS[i]);
 
  return 0;
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
//prompt an input line and return the input
//as a list of arguments
char * prompt_command()
{
  char *line = (char *) malloc(128*sizeof(char));
  printf("Command: ");
  fgets(line, 128, stdin);
  return line;
}


