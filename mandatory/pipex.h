/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pipex.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rel-fila <rel-fila@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/01/10 15:10:28 by rel-fila          #+#    #+#             */
/*   Updated: 2023/01/10 15:10:47 by rel-fila         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PIPEX_H
# define PIPEX_H

# define ERR_ENVP "Error envp !!!\n"
# define ERR_INPUT "invalid number of arguments\n"
# define ERR_INFILE "Infile"
# define ERR_OUTFILE "Outfile"
# define ERR_PIPE "Pipe"
# define ERR_CMD "Command not found\n"

# include <stdio.h>
# include <unistd.h>
# include <stdlib.h>
# include <fcntl.h>
# include <errno.h>
# include <string.h>
# include <sys/wait.h>

typedef struct s_pipex
{
	pid_t	pid1;
	pid_t	pid2;
	int		tube[2];
	int		infile;
	int		outfile;
	char	*paths;
	char	**cmd_paths;
	char	**cmd_args;
	char	*cmd;
}	t_pipex;

void	first_child(t_pipex pipex, char *argv[], char *envp[]);
void	second_child(t_pipex pipex, char *argv[], char *envp[]);
void	parent_free(t_pipex *pipex);
void	child_free(t_pipex *pipex);
int		msg(char *err);
void	msg_error(char *err);
int		ft_strncmp(const char *s1, const char *s2, size_t n);
char	*ft_strjoin(char const *s1, char const *s2);
char	**ft_split(char *str, char c);
char	*ft_strdup(const char *src);
size_t	ft_strlen(const char *s);

#endif
