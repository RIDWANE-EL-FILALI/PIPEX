/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pipex.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rel-fila <rel-fila@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/01/10 15:04:28 by rel-fila          #+#    #+#             */
/*   Updated: 2023/01/10 15:06:47 by rel-fila         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "pipex.h"

void	close_pipes(t_pipex *pipex)
{
	close(pipex->tube[0]);
	close(pipex->tube[1]);
}

char	*find_path(char **envp, t_pipex *pipex)
{
	int		cond;
	int		i;

	i = 0;
	cond = 0;
	while (envp[i])
	{
		if (ft_strncmp(envp[i], "PATH", 4) == 0)
			cond = 1;
		i++;
	}
	if (cond == 0)
	{
		msg(ERR_ENVP);
		close_pipes(pipex);
		close(pipex->infile);
		close(pipex->outfile);
		exit(1);
	}
	while (ft_strncmp("PATH", *envp, 4))
		envp++;
	return (*envp + 5);
}

int	main(int ac, char **av, char **envp)
{
	t_pipex	pipex;

	if (ac != 5)
		return (msg(ERR_INPUT));
	pipex.infile = open(av[1], O_RDONLY);
	if (pipex.infile < 0)
		msg_error(ERR_INFILE);
	pipex.outfile = open(av[ac - 1], O_TRUNC | O_CREAT | O_RDWR, 0644);
	if (pipex.outfile < 0)
		msg_error(ERR_OUTFILE);
	if (pipe(pipex.tube) < 0)
		msg_error(ERR_PIPE);
	pipex.paths = find_path(envp, &pipex);
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
