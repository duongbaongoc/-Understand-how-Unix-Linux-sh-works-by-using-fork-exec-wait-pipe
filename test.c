#include <unistd.h> //chdir(), fork()

int main(int argc, char *argv[], char *env[])
{
	char *arg[] = {"/bin/ls",NULL};
	execve(arg[0], arg, env );
}
