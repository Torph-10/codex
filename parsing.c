/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelgarh <abelgarh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/28 12:40:28 by ase               #+#    #+#             */
/*   Updated: 2026/09/11 00:32:29 by abelgarh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	parse_number(char **av, t_config *config)
{
	config->number_of_coders = atoi(av[1]);
	config->time_to_burnout = atoi(av[2]);
	config->time_to_compile = atoi(av[3]);
	config->time_to_debug = atoi(av[4]);
	config->time_to_refactor = atoi(av[5]);
	config->number_of_compiles_required = atoi(av[6]);
	config->dongle_cooldown = atoi(av[7]);
	if (config->number_of_coders < 1 || config->time_to_burnout < 1
		|| config->time_to_compile < 1
		|| config->number_of_compiles_required < 1)
	{
		printf("Error: argument must be at least 1\n");
		return (0);
	}
	if (strcmp(av[8], "fifo") == 0)
		config->scheduler = SCHED_FIFO;
	else if (strcmp(av[8], "edf") == 0)
		config->scheduler = SCHED_EDF;
	else
	{
		printf("Error: Scheduler must be fifo or edf\n");
		return (0);
	}
	return (1);
}

static int	check_number(char *str)
{
	int	i;

	if ((!str) || str[0] == '\0')
		return (0);
	i = 0;
	while (str[i])
	{
		if (str[i] < '0' || str[i] > '9')
			return (0);
		i++;
	}
	return (1);
}

int	parse(int ac, char **av, t_config *config)
{
	int	i;

	if (ac != 9)
	{
		printf("Invalid number of arguments!\n");
		return (0);
	}
	i = 1;
	while (i <= 7)
	{
		if (!check_number(av[i]))
		{
			printf("Invalid argument\n");
			return (0);
		}
		i++;
	}
	if (!parse_number(av, config))
		return (0);
	return (1);
}
