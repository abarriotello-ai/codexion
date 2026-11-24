/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abarrio <abarrio@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/24 00:21:38 by abarrio           #+#    #+#             */
/*   Updated: 2026/09/24 00:21:38 by abarrio          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "coders.h"

// A dongle is shared by exactly two coders and each is queued at most once,
// so WAITQ_CAPACITY (2) always suffices; the check only guards against
// writing past the array if that invariant were ever broken.
void	heap_push(t_heap *h, int coder_idx, long long key, long long seq)
{
	if (h->size >= h->capacity)
		return ;
	h->items[h->size].coder_idx = coder_idx;
	h->items[h->size].key = key;
	h->items[h->size].arrival_seq = seq;
	h->size++;
	heap_sift_up(h, h->size - 1);
}
