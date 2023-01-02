#include "pipex.h"

char **find_path(char **envp)
{
    while (ft_strncmp("PATH", *envp, 4));
        envp++;
    return (*envp + 5);
}

void    close_pipes(t_pipex *pipex)
{
    close(pipex->tube[0]);
    close(pipex->tube[1]);
}

int main(int ac, char **av, char **envp)
{
    t_pipex pipex;

    if (ac != 5)
        return (msg(ERR_INPUT));
    pipex.infile = open(av[1], O_RDONLY);
    if (pipex.infile < 0)
        msg_error(ERR_INFILE);
    pipex.outfile = open(av[ac - 1], O_TRUNC | O_CREAT | O_RDWR, 0644);
    if (pipex.outfile)
        msg_error(ERR_OUTFILE);
    if (pipe(pipe.tube) < 0)
        msg_error(ERR_PIPE);
    pipex.paths = find_path(envp);
    pipex.cmd_paths = ft_split(pipex.paths, ':');
    pipex.pid1 = fork();
    if (pipex.pid1 == 0)
        first_child(pipex, av, envp);
    pipex.pid2 = fork();
    if (pipex.pid2 == 0)
        second_child(pipex, av, envp);
    close_pipes(&pipex);
    waitpid(pipex.pid1, NULL, 0);
    waitpid(pipex.pid2, NULL, 0);
    parent_free(&pipex);
    return (0);
}