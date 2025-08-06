#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <stdio.h>

int err(char *str)
{
	while(*str)
		write(2, str++, 1);
	return (1);
}

int chd(char **av, int i)
{
	if(i != 2)
		return (err("error: cd: bad arguments\n"));
	else if(chdir(av[1]) == -1)
		return (err("error: cd: cannot change directory to "), err(av[1]), err("\n"));
	return 0;
}

int exe(char **av, int i, char **env)
{
	int fd[2]; 
	int status;
	int apipe = av[i] && !strcmp(av[i], "|");

	if(!apipe && !strcmp(*av, "cd"))
		return chd(av, i);
	if(apipe && pipe(fd) == -1)
		return err("error: fatal\n");
	int pid = fork();
	if(!pid)
	{
		av[i] = 0;
		if(apipe && (dup2(fd[1], 1) == -1 || close(fd[0]) == -1 || close(fd[1]) == -1))
			return err("error: fatal\n");
		if(!strcmp(*av, "cd"))
			return chd(av, i);
		execve(*av, av, env);
		return(err("error: cannot execute "), err(*av), err("\n"));
	}
	waitpid(pid, &status, 0);
	if(apipe && (dup2(fd[0], 0) == -1 || close(fd[0]) == -1 || close(fd[1]) == -1))
		return(err("error: fatal\n"));
	return WIFEXITED(status) && WEXITSTATUS(status);
}

int main(int ac, char **av, char **env)
{
	int i = 0;
	int status = 0;
	(void) ac;

	while(av[i] && av[++i])
	{
		av += i;
		i = 0;
		while(av[i] && strcmp(av[i], "|") && strcmp(av[i], ";"))
			i++;
		if(i)
			status = exe(av, i, env);
	}
	return status;
}

