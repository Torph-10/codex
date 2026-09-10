/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongles_utils.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelgarh <abelgarh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/08 13:48:09 by ase               #+#    #+#             */
/*   Updated: 2026/09/11 00:31:44 by abelgarh         ###   ########.fr       */
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

void	sift_down(t_heap *heap, int i)
{
	int	small;
	int	child;

	while (i < heap->size)
	{
		small = i;
		child = 2 * i + 1;
		if (child < heap->size
			&& heap_priority(heap, &heap->requests[child],
				&heap->requests[small]))
			small = child;
		if (child + 1 < heap->size
			&& heap_priority(heap, &heap->requests[child + 1],
				&heap->requests[small]))
			small = child + 1;
		if (small == i)
			break ;
		heap_swap(&heap->requests[i], &heap->requests[small]);
		i = small;
	}
}
