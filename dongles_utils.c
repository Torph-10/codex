/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongles_utils.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ase <ase@student.42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/08 13:48:09 by ase               #+#    #+#             */
/*   Updated: 2026/09/12 19:08:49 by ase              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	queue_request(t_coder *coder, t_dongle *dongle)
{
	pthread_mutex_lock(&dongle->mutex);
	heap_push(&dongle->waiters, coder, dongle);
	pthread_mutex_unlock(&dongle->mutex);
}

void	cancel_request(t_coder *coder, t_dongle *dongle)
{
	pthread_mutex_lock(&dongle->mutex);
	heap_remove(&dongle->waiters, coder->id);
	pthread_mutex_unlock(&dongle->mutex);
}

int	try_acquire(t_coder *coder, t_dongle *dongle)
{
	if (heap_top(&dongle->waiters) != coder->id || dongle->held_by != -1)
		return (-1);
	if (get_current_time(coder->simulation) < dongle->available_at)
		return (0);
	heap_remove(&dongle->waiters, coder->id);
	dongle->held_by = coder->id;
	return (1);
}
