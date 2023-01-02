#include "pipex.h"

int msg(char *err)
{
    write(2, err, ft_strlen(err));
    return (1);
}

void    msg_error(char *err)
{
    perror(err);
    exit(1);
}