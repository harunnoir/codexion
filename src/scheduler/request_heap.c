/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   request_heap.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abait-el <abait-el@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/10 02:27:47 by abait-el          #+#    #+#             */
/*   Updated: 2026/09/10 02:27:48 by abait-el         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	swap_requests(t_request *first, t_request *second)
{
	t_request	temporary;

	temporary = *first;
	*first = *second;
	*second = temporary;
}

void	request_heap_init(t_request_heap *heap, t_request_compare compare)
{
	heap->count = 0;
	heap->compare = compare;
}

bool	request_heap_push(t_request_heap *heap, t_request request)
{
	if (heap->count >= MAX_DONGLE_WAITERS)
		return (false);
	heap->items[heap->count++] = request;
	if (heap->count == 2 && heap->compare(&heap->items[1], &heap->items[0]) < 0)
		swap_requests(&heap->items[0], &heap->items[1]);
	return (true);
}

bool	request_heap_peek(t_request_heap *heap, t_request *request)
{
	if (heap->count == 0)
		return (false);
	*request = heap->items[0];
	return (true);
}

void	request_heap_remove(t_request_heap *heap, t_coder *coder)
{
	if (heap->count == 0)
		return ;
	if (heap->items[0].owner == coder)
	{
		heap->count--;
		if (heap->count == 1)
			heap->items[0] = heap->items[1];
	}
	else if (heap->count == 2 && heap->items[1].owner == coder)
		heap->count--;
}
