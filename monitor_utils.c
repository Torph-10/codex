/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor_utils.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelgarh <abelgarh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/08 15:53:02 by ase               #+#    #+#             */
/*   Updated: 2026/09/11 00:32:19 by abelgarh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	simulation_stopped(t_simulation *sim)
{
	int	stopped;

	pthread_mutex_lock(&sim->state_mutex);
	stopped = sim->stopped;
	pthread_mutex_unlock(&sim->state_mutex);
	return (stopped);
}

int	join_monitor(t_simulation *sim)
{
	if (pthread_join(sim->monitor, NULL) != 0)
		return (0);
	return (1);
}
