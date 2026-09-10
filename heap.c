/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelgarh <abelgarh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/07 12:16:09 by ase               #+#    #+#             */
/*   Updated: 2026/09/11 00:32:00 by abelgarh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	heap_priority(t_heap *heap, t_request *a, t_request *b)
{
	if (heap->scheduler == SCHED_EDF)
	{
		if (a->deadline != b->deadline)
			return (a->deadline < b->deadline);
	}
	if (a->arrival_order != b->arrival_order)
		return (a->arrival_order < b->arrival_order);
	return (a->coder_id < b->coder_id);
}

void	heap_swap(t_request *a, t_request *b)
{
	t_request	tmp;

	tmp = *a;
	*a = *b;
	*b = tmp;
}

int	heap_top(t_heap *heap)
{
	if (heap->size == 0)
		return (-1);
	return (heap->requests[0].coder_id);
}

void	heap_push(t_heap *heap, t_coder *coder, t_dongle *dongle)
{
	int		i;
	int		parent;

	if (heap->size >= heap->capacity)
		return ;
	dongle->fifo_ticket++;
	heap->requests[heap->size].coder_id = coder->id;
	heap->requests[heap->size].arrival_order = dongle->fifo_ticket;
	heap->requests[heap->size].deadline = coder->last_compile_start
		+ coder->simulation->config.time_to_burnout;
	i = heap->size;
	heap->size++;
	while (i > 0)
	{
		parent = (i - 1) / 2;
		if (!heap_priority(heap, &heap->requests[i], &heap->requests[parent]))
			break ;
		heap_swap(&heap->requests[i], &heap->requests[parent]);
		i = parent;
	}
}

void	heap_remove(t_heap *heap, int coder_id)
{
	int	i;

	i = 0;
	while (i < heap->size && heap->requests[i].coder_id != coder_id)
		i++;
	if (i >= heap->size)
		return ;
	heap->size--;
	heap->requests[i] = heap->requests[heap->size];
	while (i > 0 && heap_priority(heap, &heap->requests[i],
			&heap->requests[(i - 1) / 2]))
	{
		heap_swap(&heap->requests[i], &heap->requests[(i - 1) / 2]);
		i = (i - 1) / 2;
	}
	sift_down(heap, i);
}
