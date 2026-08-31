/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ase <ase@student.42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/28 12:38:44 by ase               #+#    #+#             */
/*   Updated: 2026/08/29 10:57:20 by ase              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	cleanup_simulation(t_simulation *sim)
{
	int	i;

	i = 0;
	while (i < sim->config.number_of_coders)
	{
		pthread_mutex_destroy(&sim->dongles[i].mutex);
		pthread_cond_destroy(&sim->dongles[i].cond);
		free(sim->dongles[i].waiters.requests);
		i++;
	}
	pthread_mutex_destroy(&sim->state_mutex);
	pthread_mutex_destroy(&sim->log_mutex);
	free(sim->dongles);
	free(sim->coders);
}

int	main(int ac, char **av)
{
	t_config		config;
	t_simulation	sim;

	if (!parse(ac, av, &config))
		return (1);
	if (!init_simulation(&sim, &config))
		return (1);
	if (!create_monitor(&sim))
		return (1);
	if (!create_coders(&sim))
		return (1);
	if (!join_coders(&sim))
		return (1);
	if (!join_monitor(&sim))
		return (1);
	cleanup_simulation(&sim);
	return (0);
}
