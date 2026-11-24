/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap_utils.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abarrio <abarrio@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/24 00:21:38 by abarrio           #+#    #+#             */
/*   Updated: 2026/09/24 00:21:38 by abarrio          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "coders.h"

// Removes a coder from anywhere in the heap: the last item fills the
// hole and is sifted in whichever direction restores the order.
void	heap_remove(t_heap *h, int coder_idx)
{
	int	i;
	int	pos;

	pos = -1;
	i = 0;
	while (i < h->size)
	{
		if (h->items[i].coder_idx == coder_idx)
			pos = i;
		i++;
	}
	if (pos == -1)
		return ;
	h->items[pos] = h->items[h->size - 1];
	h->size--;
	if (pos < h->size)
	{
		heap_sift_up(h, pos);
		heap_sift_down(h, pos);
	}
}
