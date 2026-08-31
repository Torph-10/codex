/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder_routine.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ase <ase@student.42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/28 13:02:31 by ase               #+#    #+#             */
/*   Updated: 2026/08/29 10:59:03 by ase              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	join_coders(t_simulation *sim)
{
	int	i;

	i = 0;
	while (i < sim->config.number_of_coders)
	{
		if (pthread_join(sim->coders[i].thread, NULL) != 0)
			return (0);
		i++;
	}
	return (1);
}

static void	perform_compile(t_coder *coder)
{
	take_dongles(coder);
	pthread_mutex_lock(&coder->simulation->state_mutex);
	coder->last_compile_start = get_current_time(coder->simulation);
	pthread_mutex_unlock(&coder->simulation->state_mutex);
	print_status(coder->simulation, coder->id, "is compiling");
	smart_sleep(coder->simulation->config.time_to_compile, coder->simulation);
	release_dongles(coder);
}

static void	perform_debug(t_coder *coder)
{
	print_status(coder->simulation, coder->id, "is debugging");
	smart_sleep(coder->simulation->config.time_to_debug, coder->simulation);
}

static void	perform_refactor(t_coder *coder)
{
	print_status(coder->simulation, coder->id, "is refactoring");
	smart_sleep(coder->simulation->config.time_to_refactor, coder->simulation);
}

void	*coder_routine(void *arg)
{
	t_coder	*coder;

	coder = (t_coder *)arg;
	if (coder->id % 2 == 0)
		smart_sleep(coder->simulation->config.time_to_compile / 2, coder->simulation);
	while (coder->compile_count
		< coder->simulation->config.number_of_compiles_required)
	{
		perform_compile(coder);
		if (simulation_stopped(coder->simulation))
			break ;
		perform_debug(coder);
		if (simulation_stopped(coder->simulation))
			break ;
		perform_refactor(coder);
		pthread_mutex_lock(&coder->simulation->state_mutex);
		coder->compile_count++;
		pthread_mutex_unlock(&coder->simulation->state_mutex);
	}
	return (NULL);
}
