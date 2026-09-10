/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ase <ase@student.42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/28 12:57:38 by ase               #+#    #+#             */
/*   Updated: 2026/09/10 15:22:11 by ase              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	init_one_dongle(t_simulation *sim, int i)
{
	if (pthread_mutex_init(&sim->dongles[i].mutex, NULL) != 0)
		return (0);
	if (pthread_cond_init(&sim->dongles[i].cond, NULL) != 0)
	{
		pthread_mutex_destroy(&sim->dongles[i].mutex);
		return (0);
	}
	sim->dongles[i].available_at = 0;
	sim->dongles[i].id = i;
	sim->dongles[i].held_by = -1;
	sim->dongles[i].fifo_ticket = 0;
	sim->dongles[i].waiters.size = 0;
	sim->dongles[i].waiters.capacity = sim->config.number_of_coders;
	sim->dongles[i].waiters.scheduler = sim->config.scheduler;
	sim->dongles[i].waiters.requests = malloc(sizeof(t_request)
			* sim->dongles[i].waiters.capacity);
	if (!sim->dongles[i].waiters.requests)
	{
		pthread_mutex_destroy(&sim->dongles[i].mutex);
		pthread_cond_destroy(&sim->dongles[i].cond);
		return (0);
	}
	return (1);
}

static int	init_dongles(t_simulation *sim)
{
	int	i;

	i = 0;
	while (i < sim->config.number_of_coders)
	{
		if (!init_one_dongle(sim, i))
			return (cleanup_dongles(sim, i), 0);
		i++;
	}
	return (1);
}

static void	init_coders(t_simulation *sim)
{
	int	i;

	i = 0;
	while (i < sim->config.number_of_coders)
	{
		sim->coders[i].id = i + 1;
		sim->coders[i].compile_count = 0;
		sim->coders[i].last_compile_start = 0;
		sim->coders[i].simulation = sim;
		sim->coders[i].left = &sim->dongles[i];
		sim->coders[i].right = &sim->dongles[(i + 1)
			% sim->config.number_of_coders];
		i++;
	}
}

static int	alloc_simulation(t_simulation *sim)
{
	sim->coders = malloc(sizeof(t_coder) * sim->config.number_of_coders);
	sim->dongles = malloc(sizeof(t_dongle) * sim->config.number_of_coders);
	if (!sim->coders || !sim->dongles)
	{
		free(sim->coders);
		free(sim->dongles);
		return (0);
	}
	return (1);
}

int	init_simulation(t_simulation *sim, t_config *config)
{
	sim->config = *config;
	sim->stopped = 0;
	sim->start_time = get_time_ms();
	if (!alloc_simulation(sim))
		return (0);
	if (!init_dongles(sim))
		return (0);
	init_coders(sim);
	if (pthread_mutex_init(&sim->log_mutex, NULL) != 0)
	{
		cleanup_dongles(sim, sim->config.number_of_coders);
		return (0);
	}
	if (pthread_mutex_init(&sim->state_mutex, NULL) != 0)
	{
		pthread_mutex_destroy(&sim->log_mutex);
		cleanup_dongles(sim, sim->config.number_of_coders);
		return (0);
	}
	return (1);
}
