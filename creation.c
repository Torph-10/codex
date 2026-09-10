/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   creation.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelgarh <abelgarh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/28 12:56:39 by ase               #+#    #+#             */
/*   Updated: 2026/09/11 00:31:40 by abelgarh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	create_monitor(t_simulation *sim)
{
	if (pthread_create(&sim->monitor, NULL, monitor_routine, sim) != 0)
		return (0);
	return (1);
}

int	create_coders(t_simulation *sim)
{
	int	i;

	i = 0;
	while (i < sim->config.number_of_coders)
	{
		if (pthread_create(&sim->coders[i].thread, NULL,
				coder_routine, &sim->coders[i]) != 0)
			return (0);
		i++;
	}
	return (1);
}
