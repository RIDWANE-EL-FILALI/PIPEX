#ifndef PIPEX_H
# define PIPEX_H

# define ERR_INPUT "invalid number of arguments\n"
# define ERR_INFILE "Infile"
# define ERR_OUTFILE "Outfile"
# define ERR_PIPE "Pipe"
# define ERR_CMD "Command not found\n"

//perror
# include <stdio.h>
//write read close acces pipe dup dup2 execve fork
# include <unistd.h>
//malloc exit free
# include <stdlib.h>
//open unlink
# include <fcntl.h>

# include <errno.h>
//strerror
# include <string.h>


#include <sys/wait.h>

//my structure

typedef struct s_pipex
{
    pid_t pid1;
    pid_t pid2;
    int tube[2];
    int infile;
    int outfile;
    char *paths;
    char **cmd_paths;
    char **cmd_args;
    char *cmd;
}   t_pipex

//children file funtions
void	first_child(t_pipex pipex, char *argv[], char *envp[]);
void	second_child(t_pipex pipex, char *argv[], char *envp[]);
//free.c
void    parent_free(t_pipex *pipex);
void    child_free(t_pipex *pipex);
//error.c
int msg(char *err);
void    msg_error(char *err);
//funtions
int		ft_strncmp(const char *s1, const char *s2, size_t n);
char	*ft_strjoin(char const *s1, char const *s2);
char	**ft_split(char const *s, char c);
char	*ft_strdup(const char *src);

#endif