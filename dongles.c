/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongles.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelgarh <abelgarh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/29 09:11:22 by ase               #+#    #+#             */
/*   Updated: 2026/09/11 00:31:56 by abelgarh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	wait_and_take(t_coder *coder, t_dongle *dongle)
{
	int	status;

	pthread_mutex_lock(&dongle->mutex);
	while (!simulation_stopped(coder->simulation))
	{
		status = try_acquire(coder, dongle);
		if (status == 1)
		{
			pthread_mutex_unlock(&dongle->mutex);
			print_status(coder->simulation, coder->id, "has taken a dongle");
			return (1);
		}
		if (status == 0)
		{
			pthread_mutex_unlock(&dongle->mutex);
			usleep(500);
			pthread_mutex_lock(&dongle->mutex);
			continue ;
		}
		pthread_cond_wait(&dongle->cond, &dongle->mutex);
	}
	heap_remove(&dongle->waiters, coder->id);
	pthread_mutex_unlock(&dongle->mutex);
	return (0);
}

static void	release_one(t_coder *coder, t_dongle *dongle)
{
	pthread_mutex_lock(&dongle->mutex);
	dongle->held_by = -1;
	dongle->available_at = get_current_time(coder->simulation)
		+ coder->simulation->config.dongle_cooldown;
	pthread_cond_broadcast(&dongle->cond);
	pthread_mutex_unlock(&dongle->mutex);
}

static void	order_dongles(t_coder *coder, t_dongle **first, t_dongle **second)
{
	if (coder->left->id < coder->right->id)
	{
		*first = coder->left;
		*second = coder->right;
	}
	else
	{
		*first = coder->right;
		*second = coder->left;
	}
	queue_request(coder, *first);
}

int	take_dongles(t_coder *coder)
{
	t_dongle	*first;
	t_dongle	*second;

	order_dongles(coder, &first, &second);
	if (first != second)
		queue_request(coder, second);
	if (!wait_and_take(coder, first))
	{
		if (first != second)
			cancel_request(coder, second);
		return (0);
	}
	if (first == second)
	{
		smart_sleep(coder->simulation->config.time_to_burnout + 10,
			coder->simulation);
		release_one(coder, first);
		return (0);
	}
	if (!wait_and_take(coder, second))
	{
		release_one(coder, first);
		return (0);
	}
	return (1);
}

void	release_dongles(t_coder *coder)
{
	release_one(coder, coder->left);
	release_one(coder, coder->right);
}
