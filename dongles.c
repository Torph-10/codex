/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongles.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ase <ase@student.42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/29 09:11:22 by ase               #+#    #+#             */
/*   Updated: 2026/08/30 12:49:04 by ase              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

#include "codexion.h"

static void queue_request(t_coder *coder, t_dongle *dongle)
{
	pthread_mutex_lock(&dongle->mutex);
	heap_push(&dongle->waiters, coder, dongle);
	pthread_mutex_unlock(&dongle->mutex);
}

static void cancel_request(t_coder *coder, t_dongle *dongle)
{
	pthread_mutex_lock(&dongle->mutex);
	heap_remove(&dongle->waiters, coder->id);
	pthread_mutex_unlock(&dongle->mutex);
}

static int wait_and_take(t_coder *coder, t_dongle *dongle)
{
	pthread_mutex_lock(&dongle->mutex);
	while (!simulation_stopped(coder->simulation))
	{
		if (heap_top(&dongle->waiters) == coder->id && dongle->held_by == -1)
		{
			if (get_current_time(coder->simulation) >= dongle->available_at)
			{
				heap_remove(&dongle->waiters, coder->id);
				dongle->held_by = coder->id;
				pthread_mutex_unlock(&dongle->mutex);
				print_status(coder->simulation, coder->id, "has taken a dongle");
				return (1);
			}
			else
			{
				pthread_mutex_unlock(&dongle->mutex);
				usleep(500);
				pthread_mutex_lock(&dongle->mutex);
				continue ;
			}
		}
		
		pthread_cond_wait(&dongle->cond, &dongle->mutex);
	}

	heap_remove(&dongle->waiters, coder->id);
	pthread_mutex_unlock(&dongle->mutex);
	return (0);
}
static void release_one(t_coder *coder, t_dongle *dongle)
{
	pthread_mutex_lock(&dongle->mutex);
	dongle->held_by = -1;
	dongle->available_at = get_current_time(coder->simulation) + coder->simulation->config.dongle_cooldown;	
	pthread_cond_broadcast(&dongle->cond);
	pthread_mutex_unlock(&dongle->mutex);
}

void take_dongles(t_coder *coder)
{
	t_dongle *first;
	t_dongle *second;
	
	if (coder->left->id < coder->right->id)
	{
		first = coder->left;
		second = coder->right;
	}
	else
	{
		first = coder->right;
		second = coder->left;
	}
	queue_request(coder, first);
	if (first != second)
		queue_request(coder, second);
	if (!wait_and_take(coder, first))
	{
		if (first != second)
			cancel_request(coder, second);
		return ;
	}
	if (first == second)
	{
		smart_sleep(coder->simulation->config.time_to_burnout + 10, coder->simulation);
		return ;
	}
	if (!wait_and_take(coder, second))
	{
		release_one(coder, first);
		return ;
	}
}

void release_dongles(t_coder *coder)
{
	release_one(coder, coder->left);
	if (coder->left != coder->right)
		release_one(coder, coder->right);
}
