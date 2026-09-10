/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cleanup.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelgarh <abelgarh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/10 23:30:37 by abelgarh          #+#    #+#             */
/*   Updated: 2026/09/11 00:31:20 by abelgarh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	cleanup_dongles(t_simulation *sim, int count)
{
	int	i;

	i = 0;
	while (i < count)
	{
		pthread_mutex_destroy(&sim->dongles[i].mutex);
		pthread_cond_destroy(&sim->dongles[i].cond);
		free(sim->dongles[i].waiters.requests);
		i++;
	}
	free(sim->dongles);
	free(sim->coders);
}

void	cleanup_simulation(t_simulation *sim)
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
