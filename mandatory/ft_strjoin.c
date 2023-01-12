/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strjoin.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rel-fila <rel-fila@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/01/10 15:03:09 by rel-fila          #+#    #+#             */
/*   Updated: 2023/01/10 15:03:11 by rel-fila         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "pipex.h"

char	*ft_strjoin(char const *s1, char const *s2)
{
	int		len1;
	int		len2;
	char	*tab;
	int		i;

	if (s1 == NULL)
		return (NULL);
	len1 = ft_strlen(s1);
	len2 = ft_strlen(s2);
	i = 0;
	tab = (char *)malloc(sizeof(char) * (len1 + len2 +1));
	if (!tab)
		return (NULL);
	while (*s1 != '\0')
	{
		tab[i++] = *s1;
		s1++;
	}
	while (*s2 != '\0')
	{
		tab[i++] = *s2;
		s2++;
	}
	tab[i] = '\0';
	return (tab);
}
