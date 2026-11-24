/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap_core.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abarrio <abarrio@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/24 00:21:38 by abarrio           #+#    #+#             */
/*   Updated: 2026/09/24 00:21:38 by abarrio          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "coders.h"

void	heap_swap(t_heap_entry *a, t_heap_entry *b)
{
	t_heap_entry	tmp;

	tmp = *a;
	*a = *b;
	*b = tmp;
}

// Heap order: smaller key first, earliest arrival on a tie.
int	heap_less(t_heap_entry *a, t_heap_entry *b)
{
	if (a->key != b->key)
		return (a->key < b->key);
	return (a->arrival_seq < b->arrival_seq);
}

// Moves items[idx] up while it goes before its parent.
void	heap_sift_up(t_heap *h, int idx)
{
	int	parent;

	while (idx > 0)
	{
		parent = (idx - 1) / 2;
		if (!heap_less(&h->items[idx], &h->items[parent]))
			break ;
		heap_swap(&h->items[idx], &h->items[parent]);
		idx = parent;
	}
}

// Moves items[idx] down while one of its children goes before it.
void	heap_sift_down(t_heap *h, int idx)
{
	int	smallest;
	int	left;
	int	right;

	while (1)
	{
		smallest = idx;
		left = 2 * idx + 1;
		right = 2 * idx + 2;
		if (left < h->size && heap_less(&h->items[left], &h->items[smallest]))
			smallest = left;
		if (right < h->size
			&& heap_less(&h->items[right], &h->items[smallest]))
			smallest = right;
		if (smallest == idx)
			break ;
		heap_swap(&h->items[idx], &h->items[smallest]);
		idx = smallest;
	}
}
