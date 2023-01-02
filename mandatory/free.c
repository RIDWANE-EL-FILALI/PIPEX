#include "pipex.h"

void    parent_free(t_pipex *pipex)
{
    int i;

    i = 0;
    close(pipex->infile);
    close(pipex->outfile);
    while (pipex->cmd_paths[i])
    {
        free(pipex->cmd_paths[i]);
        i++;
    }
    free(pipex->cmd_paths);
}

void    child_free(t_pipex *pipex)
{
    int i;

    i = 0;
    while (pipex->cmd_args[i])
    {
        free(pipex->cmd_args[i]);
        i++;
    }
    free(pipex->cmd_args);
    free(pipex->cmd);
}