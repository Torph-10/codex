/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ase <ase@student.42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/28 12:58:12 by ase               #+#    #+#             */
/*   Updated: 2026/08/30 12:54:46 by ase              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	smart_sleep(long duration_ms, t_simulation *sim)
{
	long	end_time;

	end_time = get_current_time(sim) + duration_ms;
	while (get_current_time(sim) < end_time && !simulation_stopped(sim))
	{
		usleep(500);
	}
}

void	print_status(t_simulation *sim, int coder_id, char *msg)
{
	long	time;

	pthread_mutex_lock(&sim->log_mutex);
	if (!simulation_stopped(sim))
	{
		time = get_current_time(sim);
		printf("%ld %d %s\n", time, coder_id, msg);
	}
	pthread_mutex_unlock(&sim->log_mutex);
}

long	get_time_ms(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return (tv.tv_sec * 1000L + tv.tv_usec / 1000);
}

long	get_current_time(t_simulation *sim)
{
	return (get_time_ms() - sim->start_time);
}
