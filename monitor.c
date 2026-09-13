/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ase <ase@student.42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/28 13:05:32 by ase               #+#    #+#             */
/*   Updated: 2026/09/13 00:24:41 by ase              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	wake_up_everyone(t_simulation *sim)
{
	int	i;

	i = 0;
	while (i < sim->config.number_of_coders)
	{
		pthread_mutex_lock(&sim->dongles[i].mutex);
		pthread_cond_broadcast(&sim->dongles[i].cond);
		pthread_mutex_unlock(&sim->dongles[i].mutex);
		i++;
	}
}

static int	all_coders_finished(t_simulation *sim)
{
	int	i;
	int	count;

	i = 0;
	while (i < sim->config.number_of_coders)
	{
		pthread_mutex_lock(&sim->state_mutex);
		count = sim->coders[i].compile_count;
		pthread_mutex_unlock(&sim->state_mutex);
		if (count < sim->config.number_of_compiles_required)
			return (0);
		i++;
	}
	return (1);
}

static int	check_burnout(t_simulation *sim, int i, long now)
{
	long	last_compile_start;
	int		count;

	pthread_mutex_lock(&sim->state_mutex);
	last_compile_start = sim->coders[i].last_compile_start;
	count = sim->coders[i].compile_count;
	pthread_mutex_unlock(&sim->state_mutex);
	if (count >= sim->config.number_of_compiles_required)
		return (0);
	if (now - last_compile_start >= sim->config.time_to_burnout)
	{
		pthread_mutex_lock(&sim->log_mutex);
		pthread_mutex_lock(&sim->state_mutex);
		if (sim->stopped == 0)
		{
			sim->stopped = 1;
			printf("%ld Coder %d burned out\n", now, sim->coders[i].id);
		}
		pthread_mutex_unlock(&sim->state_mutex);
		pthread_mutex_unlock(&sim->log_mutex);
		wake_up_everyone(sim);
		return (1);
	}
	return (0);
}

static int	check_all_burnouts(t_simulation *sim, long now)
{
	int	i;

	i = 0;
	while (i < sim->config.number_of_coders)
	{
		if (check_burnout(sim, i, now))
			return (1);
		i++;
	}
	return (0);
}

void	*monitor_routine(void *arg)
{
	t_simulation	*sim;
	long			now;

	sim = (t_simulation *)arg;
	while (!simulation_stopped(sim))
	{
		now = get_current_time(sim);
		if (check_all_burnouts(sim, now))
			return (NULL);
		if (all_coders_finished(sim))
		{
			pthread_mutex_lock(&sim->state_mutex);
			sim->stopped = 1;
			pthread_mutex_unlock(&sim->state_mutex);
			return (NULL);
		}
		usleep(1000);
	}
	return (NULL);
}
