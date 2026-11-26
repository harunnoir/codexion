/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   request_compare.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abait-el <abait-el@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/10 02:27:45 by abait-el          #+#    #+#             */
/*   Updated: 2026/09/10 02:27:45 by abait-el         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	compare_fifo(const t_request *left, const t_request *right)
{
	if (left->arrival_order < right->arrival_order)
		return (-1);
	if (left->arrival_order > right->arrival_order)
		return (1);
	return (0);
}

static int	compare_edf(const t_request *left, const t_request *right)
{
	if (left->burnout_deadline_ms < right->burnout_deadline_ms)
		return (-1);
	if (left->burnout_deadline_ms > right->burnout_deadline_ms)
		return (1);
	return (compare_fifo(left, right));
}

t_request_compare	scheduler_comparator(t_scheduler_policy policy)
{
	if (policy == POLICY_EDF)
		return (compare_edf);
	return (compare_fifo);
}
